#ifndef HIERARCHYCONTROLLER_H
#define HIERARCHYCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QList>

class HierarchyTreeView;
class QGraphicsScene;
class BlockDefinition;
class BlockDefinitionView;
class PartProperty;
class DropGraphicsView;

class HierarchyController : public QObject
{
    Q_OBJECT

public:
    explicit HierarchyController(
        HierarchyTreeView *treeView,
        QGraphicsScene *scene,
        DropGraphicsView *graphicsView,
        QObject *parent = nullptr);

    // Block registration
    void registerBlock(BlockDefinition *definition, BlockDefinitionView *view);
    void unregisterBlock(BlockDefinition *definition);

    // Connection tracking (PartProperties)
    void addConnection(PartProperty *partProperty);
    void removeConnection(PartProperty *partProperty);

    // Hierarchy management
    void rebuildHierarchy();

public slots:
    void onBlockCreated(BlockDefinition *definition, BlockDefinitionView *view);
    void onBlockDeleted(BlockDefinition *definition);
    void onConnectionCreated(PartProperty *partProperty);
    void onConnectionDeleted(PartProperty *partProperty);
    void onTreeItemClicked(BlockDefinition *definition);
    void onTreeItemDoubleClicked(BlockDefinition *definition);

private:
    HierarchyTreeView *m_treeView;
    QGraphicsScene *m_scene;
    DropGraphicsView *m_graphicsView;

    // Data structures
    // definitions are typically unique by pointer, but we might want ID lookups
    QMap<QString, BlockDefinition *> m_definitions;    // id -> BlockDefinition
    QMap<QString, BlockDefinitionView *> m_definitionViews; // id -> BlockDefinitionView
    QList<PartProperty *> m_connections;

    // Helper methods
    void buildHierarchy(); // Simplified from buildHierarchyMaps
    QList<BlockDefinition *> getRootDefinitions();
    void addDefinitionToTree(BlockDefinition *definition);
    void highlightDefinitionInScene(BlockDefinition *definition);
    void centerViewOnDefinition(BlockDefinition *definition);
};

#endif // HIERARCHYCONTROLLER_H