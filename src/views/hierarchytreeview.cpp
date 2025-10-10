#include "../../headers/views/hierarchytreeview.h"
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>

HierarchyTreeView::HierarchyTreeView(QWidget *parent)
    : QTreeWidget(parent)
{
    setupTreeWidget();
}

void HierarchyTreeView::setupTreeWidget()
{
    // Column configuration
    setColumnCount(1);
    setHeaderLabel("Block Hierarchy");

    // Visual styling
    setAlternatingRowColors(true);
    setAnimated(true);
    setExpandsOnDoubleClick(true);
    setIndentation(20);

    // Selection behavior
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // Disable drag-drop for now
    setDragEnabled(false);
    setAcceptDrops(false);

    // Connect signals
    connect(this, &QTreeWidget::itemClicked, this, &HierarchyTreeView::onItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &HierarchyTreeView::onItemDoubleClicked);
}

void HierarchyTreeView::addBlock(BlockModel *block)
{
    if (!block || m_blockItems.contains(block->id()))
    {
        return;
    }

    // Create tree item
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, block->label());
    item->setIcon(0, createColorIcon(block->color()));

    // Store mappings
    m_blockItems[block->id()] = item;
    m_itemToBlock[item] = block;

    // Connect to model updates
    connect(block, &BlockModel::labelChanged, this, &HierarchyTreeView::onBlockLabelChanged);
    connect(block, &BlockModel::colorChanged, this, &HierarchyTreeView::onBlockColorChanged);
}

void HierarchyTreeView::removeBlock(BlockModel *block)
{
    if (!block)
        return;

    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (item)
    {
        // Disconnect signals
        disconnect(block, &BlockModel::labelChanged, this, &HierarchyTreeView::onBlockLabelChanged);
        disconnect(block, &BlockModel::colorChanged, this, &HierarchyTreeView::onBlockColorChanged);

        // Remove from parent or root
        QTreeWidgetItem *parent = item->parent();
        if (parent)
        {
            parent->removeChild(item);
        }
        else
        {
            int index = indexOfTopLevelItem(item);
            if (index >= 0)
            {
                takeTopLevelItem(index);
            }
        }

        // Clean up mappings
        m_blockItems.remove(block->id());
        m_itemToBlock.remove(item);

        delete item;
    }
}

QTreeWidgetItem *HierarchyTreeView::findBlockItem(BlockModel *block)
{
    if (!block)
        return nullptr;

    return m_blockItems.value(block->id(), nullptr);
}

void HierarchyTreeView::clearHierarchy()
{
    // Remove all items from tree without deleting them
    // We need to keep the QTreeWidgetItem objects alive for re-adding
    while (topLevelItemCount() > 0)
    {
        QTreeWidgetItem *item = takeTopLevelItem(0);
        // Don't delete the item, just remove it from the tree
        // The item is still referenced in m_blockItems
    }

    // Reset item-to-block mapping but keep block-to-item mapping
    m_itemToBlock.clear();
}

void HierarchyTreeView::setBlockAsRoot(BlockModel *block)
{
    if (!block)
        return;

    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (!item)
        return;

    // Remove from current parent if any
    QTreeWidgetItem *parent = item->parent();
    if (parent)
    {
        parent->removeChild(item);
    }
    else
    {
        // Already at root, just ensure it's not in the tree yet
        int index = indexOfTopLevelItem(item);
        if (index >= 0)
        {
            return; // Already at root
        }
    }

    // Add to root level
    addTopLevelItem(item);
    m_itemToBlock[item] = block;
}

void HierarchyTreeView::setBlockAsChild(BlockModel *parent, BlockModel *child)
{
    if (!parent || !child)
        return;

    QTreeWidgetItem *parentItem = m_blockItems.value(parent->id());
    QTreeWidgetItem *childItem = m_blockItems.value(child->id());

    if (!parentItem || !childItem)
        return;

    // Remove child from current parent if any
    QTreeWidgetItem *currentParent = childItem->parent();
    if (currentParent)
    {
        currentParent->removeChild(childItem);
    }
    else
    {
        // Remove from root level
        int index = indexOfTopLevelItem(childItem);
        if (index >= 0)
        {
            takeTopLevelItem(index);
        }
    }

    // Add as child of new parent
    parentItem->addChild(childItem);
    m_itemToBlock[childItem] = child;

    // Expand parent to show the new child
    parentItem->setExpanded(true);
}

void HierarchyTreeView::selectBlock(BlockModel *block)
{
    if (!block)
        return;

    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (item)
    {
        setCurrentItem(item);
        scrollToItem(item);
    }
}

void HierarchyTreeView::clearBlockSelection()
{
    clearSelection();
    setCurrentItem(nullptr);
}

void HierarchyTreeView::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    BlockModel *block = m_itemToBlock.value(item);
    if (block)
    {
        emit blockItemClicked(block);
    }
}

void HierarchyTreeView::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    BlockModel *block = m_itemToBlock.value(item);
    if (block)
    {
        emit blockItemDoubleClicked(block);
    }
}

void HierarchyTreeView::onBlockLabelChanged(const QString &label)
{
    BlockModel *block = qobject_cast<BlockModel *>(sender());
    if (!block)
        return;

    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (item)
    {
        item->setText(0, label);
    }
}

void HierarchyTreeView::onBlockColorChanged(const QColor &color)
{
    BlockModel *block = qobject_cast<BlockModel *>(sender());
    if (!block)
        return;

    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (item)
    {
        item->setIcon(0, createColorIcon(color));
    }
}

QIcon HierarchyTreeView::createColorIcon(const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);

    // Add border
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 15, 15);

    return QIcon(pixmap);
}