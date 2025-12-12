#include "../../headers/controllers/hierarchycontroller.h"
#include "../../headers/views/hierarchytreeview.h"
#include "../../headers/views/blockdefinitionview.h"
#include "../../headers/views/dropgraphicsview.h"
#include "../../headers/models/blockdefinition.h"
#include "../../headers/models/partproperty.h"
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
    connect(m_treeView, &HierarchyTreeView::definitionItemClicked,
            this, &HierarchyController::onTreeItemClicked);
    connect(m_treeView, &HierarchyTreeView::definitionItemDoubleClicked,
            this, &HierarchyController::onTreeItemDoubleClicked);
}

void HierarchyController::registerBlock(BlockDefinition *definition, BlockDefinitionView *view)
{
    if (!definition || !view)
        return;

    QString id = definition->id();

    // Store definition and view
    m_definitions[id] = definition;
    m_definitionViews[id] = view;

    // Add to tree view
    m_treeView->addDefinition(definition);

    // Also verify if there are existing parts to show?
    // When a block is dropped, it has no parts initially.
    // But if loaded from file, it might.
    for (PartProperty *part : definition->partProperties())
    {
        m_treeView->updatePart(part);
        
        // Also track parts as connections if not already?
        if (!m_connections.contains(part)) {
             m_connections.append(part);
        }
    }

    qDebug() << "Registered block definition:" << definition->typeName() << "with ID:" << id;
}

void HierarchyController::unregisterBlock(BlockDefinition *definition)
{
    if (!definition)
        return;

    QString id = definition->id();

    // Remove from tree view
    m_treeView->removeDefinition(definition);

    // Remove from maps
    m_definitions.remove(id);
    m_definitionViews.remove(id);

    qDebug() << "Unregistered block definition:" << id;
}

void HierarchyController::addConnection(PartProperty *partProperty)
{
    if (!partProperty || m_connections.contains(partProperty))
        return;

    m_connections.append(partProperty);
    
    // Update tree view to show this part under its owner
    if (partProperty->owner()) {
        m_treeView->updatePart(partProperty);
    }
    
    // Connect to PartProperty signals for live updates
    connect(partProperty, &PartProperty::nameChanged, this, [this, partProperty]() {
        m_treeView->updatePart(partProperty);
    });
    connect(partProperty, &PartProperty::multiplicityChanged, this, [this, partProperty]() {
        m_treeView->updatePart(partProperty);
    });
}

void HierarchyController::removeConnection(PartProperty *partProperty)
{
    if (!partProperty)
        return;

    m_connections.removeOne(partProperty);
    
    // To remove from tree, we might need to refresh the owner node
    // HierarchyTreeView::updatePart adds or updates. It doesn't explicit remove.
    // If we want to remove, we might need a removePart method on view, or just clear and rebuild owner.
    // For now, let's trigger a full rebuild or add removePart to View API.
    // Or we can rebuild hierarchy for simplicity.
    rebuildHierarchy();
}

void HierarchyController::rebuildHierarchy()
{
    qDebug() << "Rebuilding hierarchy...";

    // Clear existing tree structure
    m_treeView->clearHierarchy();

    // Add all definitions
    for (BlockDefinition *def : m_definitions.values())
    {
        m_treeView->addDefinition(def);
        
        // Add all parts for this definition
        for (PartProperty *part : def->partProperties())
        {
            m_treeView->updatePart(part);
        }
    }
}

void HierarchyController::buildHierarchy()
{
    rebuildHierarchy();
}

QList<BlockDefinition *> HierarchyController::getRootDefinitions()
{
    return m_definitions.values(); // All are roots
}

void HierarchyController::addDefinitionToTree(BlockDefinition *definition)
{
    m_treeView->addDefinition(definition);
    for (PartProperty *part : definition->partProperties())
    {
        m_treeView->updatePart(part);
    }
}

void HierarchyController::highlightDefinitionInScene(BlockDefinition *definition)
{
    if (!definition)
        return;

    BlockDefinitionView *view = m_definitionViews.value(definition->id());
    if (!view)
        return;

    // Clear previous selection
    m_scene->clearSelection();

    // Select the block
    view->setSelected(true);

    qDebug() << "Highlighted block:" << definition->typeName();
}

void HierarchyController::centerViewOnDefinition(BlockDefinition *definition)
{
    if (!definition || !m_graphicsView)
        return;

    BlockDefinitionView *view = m_definitionViews.value(definition->id());
    if (!view)
        return;

    // Center the view on the block
    m_graphicsView->centerOn(view);

    qDebug() << "Centered view on block:" << definition->typeName();
}

void HierarchyController::onBlockCreated(BlockDefinition *definition, BlockDefinitionView *view)
{
    qDebug() << "HierarchyController: Block Created -" << definition->typeName();
    registerBlock(definition, view);
}

void HierarchyController::onBlockDeleted(BlockDefinition *definition)
{
    qDebug() << "HierarchyController: Block Deleted";
    unregisterBlock(definition);
}

void HierarchyController::onConnectionCreated(PartProperty *partProperty)
{
    qDebug() << "HierarchyController: Part Property Created";
    addConnection(partProperty);
}

void HierarchyController::onConnectionDeleted(PartProperty *partProperty)
{
    qDebug() << "HierarchyController: Part Property Deleted";
    removeConnection(partProperty);
}

void HierarchyController::onTreeItemClicked(BlockDefinition *definition)
{
    qDebug() << "HierarchyController: Tree item clicked -" << definition->typeName();
    highlightDefinitionInScene(definition);
    centerViewOnDefinition(definition);
}

void HierarchyController::onTreeItemDoubleClicked(BlockDefinition *definition)
{
    qDebug() << "HierarchyController: Tree item double-clicked -" << definition->typeName();
    highlightDefinitionInScene(definition);
    centerViewOnDefinition(definition);
}