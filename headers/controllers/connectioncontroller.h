#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QList>
#include <QHash>

class QGraphicsScene;
class BlockDefinitionView;
class BlockDefinition;
class PartProperty;
class ConnectionView;

class ConnectionController : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionController(QGraphicsScene *scene, QObject *parent = nullptr);
    ~ConnectionController();

    void registerBlockView(BlockDefinitionView *view);
    void unregisterBlockView(BlockDefinitionView *view);

    void addConnection(PartProperty *partProperty);
    void deleteConnection(PartProperty *partProperty);
    
    // Remove all connections involving a block (call BEFORE deleting the block)
    // This handles the cascade delete scenario properly
    void deleteConnectionsForBlock(BlockDefinition *definition);
    
    // Remove view only (for cascade deletes where PartProperty is deleted by BlockDefinition)
    void removeConnectionView(PartProperty *partProperty);
    
    void clearAllConnections();
    
    QList<PartProperty*> connections() const { return m_connections; }

signals:
    void connectionCreated(PartProperty *partProperty);
    void connectionDeleted(PartProperty *partProperty);

private slots:
    void onConnectionStarted(BlockDefinitionView *startBlock);
    void onConnectionCompleted(BlockDefinitionView *startBlock, BlockDefinitionView *endBlock);
    void onBlockViewDestroyed(QObject *obj);

private:
    QGraphicsScene *m_scene;
    QList<BlockDefinitionView *> m_blockViews;
    QList<PartProperty *> m_connections;
    QHash<PartProperty *, ConnectionView *> m_connectionViews;

    bool connectionExists(BlockDefinitionView *start, BlockDefinitionView *end);
    // Cycle detection might be less relevant for composition, but still good to have
    bool wouldCreateCycle(BlockDefinition *fromBlock, BlockDefinition *toBlock);
    bool canReach(BlockDefinition *fromBlock, BlockDefinition *targetBlock);
};

#endif // CONNECTIONCONTROLLER_H