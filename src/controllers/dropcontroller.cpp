#include "../headers/controllers/dropcontroller.h"
#include <QGraphicsScene>
#include "views/blockview.h"
#include "controllers/connectioncontroller.h"

DropController::DropController(QGraphicsScene *scene, DropGraphicsView *view, ConnectionController *connectionController, QObject *parent)
    : QObject(parent), m_scene(scene), m_view(view), m_connectionController(connectionController)
{
}

void DropController::handleDrop(const QString &blockType, const QPointF &position)
{
    BlockModel *model = createBlockModel(blockType);
    if (model)
    {
        model->setPosition(position);
        addBlock(model);
    }
}

void DropController::addBlock(BlockModel *model)
{
    if (!model)
        return;

    // Track the block
    m_blocks.append(model);

    // Create view for the block
    BlockView *view = new BlockView(model, nullptr);
    m_scene->addItem(view);
    
    // Track the view-model association
    m_blockViews[model] = view;

    // Register the new block with the connection controller
    if (m_connectionController)
    {
        m_connectionController->registerBlockView(view);
    }

    // Emit signal for hierarchy controller
    emit blockCreated(model, view);
}

void DropController::deleteBlock(BlockModel *block)
{
    if (!block)
        return;

    // Step 1: Remove from our tracking FIRST (before any signals or deletions)
    m_blocks.removeOne(block);
    BlockView *view = m_blockViews.take(block);
    
    // Step 2: Delete all connections involving this block (cascading delete)
    if (m_connectionController)
    {
        m_connectionController->deleteConnectionsForBlock(block);
        
        // Unregister view from connection controller
        if (view)
        {
            m_connectionController->unregisterBlockView(view);
        }
    }
    
    // Step 3: Remove view from scene and delete it
    if (view)
    {
        m_scene->removeItem(view);
        delete view;
        view = nullptr;
    }
    
    // Step 4: Emit signal for hierarchy controller (block is still valid here)
    emit blockDeleted(block);
    
    // Step 5: Finally delete the model (LAST)
    delete block;
}

BlockView* DropController::getViewForBlock(BlockModel *block) const
{
    return m_blockViews.value(block, nullptr);
}

void DropController::clear()
{
    // Clear tracked blocks (scene->clear() will delete the views)
    m_blocks.clear();
}

BlockModel *DropController::createBlockModel(const QString &blockType)
{
    QColor color = getColorForType(blockType);
    if (!color.isValid())
    {
        return nullptr;
    }

    return new BlockModel(color, blockType, QPointF(0, 0));
}

QColor DropController::getColorForType(const QString &blockType)
{
    if (blockType == "Red Block")
    {
        return Qt::red;
    }
    else if (blockType == "Blue Block")
    {
        return Qt::blue;
    }
    else if (blockType == "Green Block")
    {
        return Qt::green;
    }
    return QColor();
}