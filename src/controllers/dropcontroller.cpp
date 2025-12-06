#include "../../headers/controllers/dropcontroller.h"
#include <QGraphicsScene>
#include "../../headers/views/blockdefinitionview.h"
#include "../../headers/controllers/connectioncontroller.h"

DropController::DropController(QGraphicsScene *scene, DropGraphicsView *view, ConnectionController *connectionController, QObject *parent)
    : QObject(parent), m_scene(scene), m_view(view), m_connectionController(connectionController)
{
}

void DropController::handleDrop(const QString &blockType, const QPointF &position)
{
    BlockDefinition *definition = createBlockDefinition(blockType);
    if (definition)
    {
        definition->setPosition(position);
        addBlock(definition);
    }
}

void DropController::addBlock(BlockDefinition *definition)
{
    if (!definition)
        return;

    // Track the definition
    m_definitions.append(definition);

    // Create view for the definition
    BlockDefinitionView *view = new BlockDefinitionView(definition, nullptr);
    m_scene->addItem(view);
    
    // Track the view-definition association
    m_definitionViews[definition] = view;

    // Register the new block with the connection controller
    if (m_connectionController)
    {
        m_connectionController->registerBlockView(view);
    }

    // Emit signal for hierarchy controller
    emit blockCreated(definition, view);
}

void DropController::deleteBlock(BlockDefinition *definition)
{
    if (!definition)
        return;

    // Step 1: Remove from our tracking FIRST (before any signals or deletions)
    m_definitions.removeOne(definition);
    BlockDefinitionView *view = m_definitionViews.take(definition);
    
    // Step 2: Delete all connections involving this block (cascading delete)
    if (m_connectionController)
    {
        // m_connectionController->deleteConnectionsForBlock(definition);
    }
    
    // Step 3: Remove view from scene and delete it
    if (view)
    {
        m_scene->removeItem(view);
        delete view;
        view = nullptr;
    }
    
    // Step 4: Emit signal for hierarchy controller (block is still valid here)
    emit blockDeleted(definition);
    
    // Step 5: Finally delete the definition (LAST)
    delete definition;
}

BlockDefinitionView* DropController::getViewForDefinition(BlockDefinition *definition) const
{
    return m_definitionViews.value(definition, nullptr);
}

void DropController::clear()
{
    // Clear tracked definitions (scene->clear() will delete the views)
    m_definitions.clear();
}

BlockDefinition *DropController::createBlockDefinition(const QString &blockType)
{
    QColor color = getColorForType(blockType);
    if (!color.isValid())
    {
        return nullptr;
    }

    return new BlockDefinition(blockType, color, QPointF(0, 0));
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