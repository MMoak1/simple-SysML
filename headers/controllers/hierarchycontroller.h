#ifndef HIERARCHYCONTROLLER_H
#define HIERARCHYCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QList>

class HierarchyTreeView;
class QGraphicsScene;
class BlockModel;
class BlockView;
class ConnectionModel;
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
    void registerBlock(BlockModel *block, BlockView *view);
    void unregisterBlock(BlockModel *block);

    // Connection tracking
    void addConnection(ConnectionModel *connection);
    void removeConnection(ConnectionModel *connection);

    // Hierarchy management
    void rebuildHierarchy();

public slots:
    void onBlockCreated(BlockModel *block, BlockView *view);
    void onBlockDeleted(BlockModel *block);
    void onConnectionCreated(ConnectionModel *connection);
    void onConnectionDeleted(ConnectionModel *connection);
    void onTreeItemClicked(BlockModel *block);
    void onTreeItemDoubleClicked(BlockModel *block);

private:
    HierarchyTreeView *m_treeView;
    QGraphicsScene *m_scene;
    DropGraphicsView *m_graphicsView;

    // Data structures
    QMap<QString, BlockModel *> m_blocks;    // blockId -> BlockModel
    QMap<QString, BlockView *> m_blockViews; // blockId -> BlockView
    QList<ConnectionModel *> m_connections;

    // Hierarchy tracking
    QMap<QString, QSet<QString>> m_childrenMap; // parentId -> set of childIds
    QMap<QString, QSet<QString>> m_parentsMap;  // childId -> set of parentIds

    // Helper methods
    void buildHierarchyMaps();
    QList<BlockModel *> getRootBlocks();
    QList<BlockModel *> getChildBlocks(BlockModel *parent);
    bool hasIncomingConnections(BlockModel *block);
    void addBlockToTree(BlockModel *block, BlockModel *parentBlock = nullptr);
    void highlightBlockInScene(BlockModel *block);
    void centerViewOnBlock(BlockModel *block);
};

#endif // HIERARCHYCONTROLLER_H