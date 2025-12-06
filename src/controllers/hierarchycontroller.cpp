#include "../../headers/controllers/hierarchycontroller.h"
#include "../../headers/views/hierarchytreeview.h"
#include "../../headers/views/blockview.h"
#include "../../headers/views/dropgraphicsview.h"
#include "../../headers/models/blockmodel.h"
#include "../../headers/models/connectionmodel.h"
#include <QGraphicsScene>
#include <QDebug>

HierarchyController::HierarchyController(
    HierarchyTreeView *treeView,
    QGraphicsScene *scene,
    DropGraphicsView *graphicsView,
    QObject *parent)
    : QObject(parent),
      m_treeView(treeView),
      m_scene(scene),
      m_graphicsView(graphicsView)
{
    // Connect tree view signals
    connect(m_treeView, &HierarchyTreeView::blockItemClicked,
            this, &HierarchyController::onTreeItemClicked);
    connect(m_treeView, &HierarchyTreeView::blockItemDoubleClicked,
            this, &HierarchyController::onTreeItemDoubleClicked);
}

void HierarchyController::registerBlock(BlockModel *block, BlockView *view)
{
    if (!block || !view)
        return;

    QString blockId = block->id();

    // Store block and view
    m_blocks[blockId] = block;
    m_blockViews[blockId] = view;

    // Add to tree view
    m_treeView->addBlock(block);

    // Initially add as root (will be reorganized when connections are made)
    m_treeView->setBlockAsRoot(block);

    qDebug() << "Registered block:" << block->label() << "with ID:" << blockId;
}

void HierarchyController::unregisterBlock(BlockModel *block)
{
    if (!block)
        return;

    QString blockId = block->id();

    // Remove from tree view
    m_treeView->removeBlock(block);

    // Remove from maps
    m_blocks.remove(blockId);
    m_blockViews.remove(blockId);

    // Remove from hierarchy maps
    m_childrenMap.remove(blockId);
    m_parentsMap.remove(blockId);

    // Remove this block from other blocks' children/parents
    for (auto it = m_childrenMap.begin(); it != m_childrenMap.end(); ++it)
    {
        it.value().remove(blockId);
    }
    for (auto it = m_parentsMap.begin(); it != m_parentsMap.end(); ++it)
    {
        it.value().remove(blockId);
    }

    qDebug() << "Unregistered block:" << blockId;
}

void HierarchyController::addConnection(ConnectionModel *connection)
{
    if (!connection || m_connections.contains(connection))
        return;

    m_connections.append(connection);
    rebuildHierarchy();
}

void HierarchyController::removeConnection(ConnectionModel *connection)
{
    if (!connection)
        return;

    m_connections.removeOne(connection);
    rebuildHierarchy();
}

void HierarchyController::rebuildHierarchy()
{
    qDebug() << "Rebuilding hierarchy...";

    // Clear existing tree structure
    m_treeView->clearHierarchy();

    // Rebuild parent-child maps from connections
    buildHierarchyMaps();

    // Get root blocks (blocks with no incoming connections)
    QList<BlockModel *> rootBlocks = getRootBlocks();

    qDebug() << "Found" << rootBlocks.size() << "root blocks";

    // Add root blocks to tree
    for (BlockModel *block : rootBlocks)
    {
        addBlockToTree(block, nullptr);
    }
}

void HierarchyController::buildHierarchyMaps()
{
    m_childrenMap.clear();
    m_parentsMap.clear();

    for (ConnectionModel *conn : m_connections)
    {
        // Safety check - connection may have been deleted
        if (!conn || !conn->isValid())
            continue;

        QString startId = conn->startBlockId();
        QString endId = conn->endBlockId();

        // Add to children map (parent -> children)
        m_childrenMap[startId].insert(endId);

        // Add to parents map (child -> parents)
        m_parentsMap[endId].insert(startId);
    }

    qDebug() << "Built hierarchy maps - Parents:" << m_parentsMap.size()
             << "Children:" << m_childrenMap.size();
}

QList<BlockModel *> HierarchyController::getRootBlocks()
{
    QList<BlockModel *> roots;

    for (BlockModel *block : m_blocks.values())
    {
        // Safety check - block may have been deleted during batch operations
        if (!block)
            continue;
            
        if (!hasIncomingConnections(block))
        {
            roots.append(block);
        }
    }

    return roots;
}

QList<BlockModel *> HierarchyController::getChildBlocks(BlockModel *parent)
{
    QList<BlockModel *> children;

    if (!parent)
        return children;

    QSet<QString> childIds = m_childrenMap.value(parent->id());
    for (const QString &childId : childIds)
    {
        BlockModel *child = m_blocks.value(childId);
        if (child)
        {
            children.append(child);
        }
    }

    return children;
}

bool HierarchyController::hasIncomingConnections(BlockModel *block)
{
    if (!block)
        return false;

    return m_parentsMap.contains(block->id()) &&
           !m_parentsMap[block->id()].isEmpty();
}

void HierarchyController::addBlockToTree(BlockModel *block, BlockModel *parentBlock)
{
    if (!block)
        return;

    if (parentBlock)
    {
        // Add as child
        m_treeView->setBlockAsChild(parentBlock, block);
        qDebug() << "Added" << block->label() << "as child of" << parentBlock->label();
    }
    else
    {
        // Add as root
        m_treeView->setBlockAsRoot(block);
        qDebug() << "Added" << block->label() << "as root";
    }

    // Recursively add children
    QList<BlockModel *> children = getChildBlocks(block);
    for (BlockModel *child : children)
    {
        addBlockToTree(child, block);
    }
}

void HierarchyController::highlightBlockInScene(BlockModel *block)
{
    if (!block)
        return;

    BlockView *view = m_blockViews.value(block->id());
    if (!view)
        return;

    // Clear previous selection
    m_scene->clearSelection();

    // Select the block
    view->setSelected(true);

    qDebug() << "Highlighted block:" << block->label();
}

void HierarchyController::centerViewOnBlock(BlockModel *block)
{
    if (!block || !m_graphicsView)
        return;

    BlockView *view = m_blockViews.value(block->id());
    if (!view)
        return;

    // Center the view on the block
    m_graphicsView->centerOn(view);

    qDebug() << "Centered view on block:" << block->label();
}

void HierarchyController::onBlockCreated(BlockModel *block, BlockView *view)
{
    qDebug() << "HierarchyController: Block created -" << block->label();
    registerBlock(block, view);
}

void HierarchyController::onBlockDeleted(BlockModel *block)
{
    qDebug() << "HierarchyController: Block deleted";
    unregisterBlock(block);
}

void HierarchyController::onConnectionCreated(ConnectionModel *connection)
{
    qDebug() << "HierarchyController: Connection created";
    addConnection(connection);
}

void HierarchyController::onConnectionDeleted(ConnectionModel *connection)
{
    qDebug() << "HierarchyController: Connection deleted";
    removeConnection(connection);
}

void HierarchyController::onTreeItemClicked(BlockModel *block)
{
    qDebug() << "HierarchyController: Tree item clicked -" << block->label();
    highlightBlockInScene(block);
    centerViewOnBlock(block);
}

void HierarchyController::onTreeItemDoubleClicked(BlockModel *block)
{
    qDebug() << "HierarchyController: Tree item double-clicked -" << block->label();
    highlightBlockInScene(block);
    centerViewOnBlock(block);
}