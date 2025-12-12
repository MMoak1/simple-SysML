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

void DropController::handleDrop(const QString &blockType, const QPointF &position, const QString &customName)
{
    BlockDefinition *definition = createBlockDefinition(blockType);
    if (definition)
    {
        definition->setTypeName(customName);
        definition->setPosition(position);
        addBlock(definition);
    }
}

void DropController::handleDefinitionDrop(const QString &definitionId, const QPointF &position)
{
    // Find the source definition by ID
    BlockDefinition *sourceDef = nullptr;
    for (BlockDefinition *def : m_definitions)
    {
        if (def->id() == definitionId)
        {
            sourceDef = def;
            break;
        }
    }

    if (!sourceDef)
    {
        qDebug() << "DropController: Could not find source definition with ID:" << definitionId;
        return;
    }

    // Clone it
    BlockDefinition *newDef = sourceDef->clone();
    
    // Set new position
    newDef->setPosition(position);
    
    // Add to project
    addBlock(newDef);
    
    qDebug() << "DropController: Created instance copy of" << sourceDef->typeName();
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

    // Step 1: Remove all connections involving this block FIRST (before any deletions)
    // This properly handles cascade deletes for:
    //   - Parts owned by this block (removes views only, BlockDefinition destructor deletes PartProperties)
    //   - Parts where this block is the type (fully removes those connections from their owners)
    if (m_connectionController)
    {
        m_connectionController->deleteConnectionsForBlock(definition);
    }
    
    // Step 2: Remove from our tracking
    m_definitions.removeOne(definition);
    BlockDefinitionView *view = m_definitionViews.take(definition);
    
    // Step 3: Remove view from scene and defer deletion
    if (view)
    {
        m_scene->removeItem(view);
        view->deleteLater();  // SAFE: defers to event loop
    }
    
    // Step 4: Emit signal for hierarchy controller (block is still valid here)
    emit blockDeleted(definition);
    
    // Step 5: Finally delete the definition using deleteLater
    // This ensures all signal handlers complete before destruction
    definition->deleteLater();
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