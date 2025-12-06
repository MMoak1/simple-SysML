#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QList>
#include <QHash>

class QGraphicsScene;
class BlockView;
class BlockModel;
class ConnectionModel;
class ConnectionView;

class ConnectionController : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionController(QGraphicsScene *scene, QObject *parent = nullptr);
    ~ConnectionController();

    void registerBlockView(BlockView *view);
    void unregisterBlockView(BlockView *view);

    void addConnection(ConnectionModel *connection);
    void deleteConnection(ConnectionModel *connection);
    void deleteConnectionsForBlock(BlockModel *block);
    void clearAllConnections();
    
    QList<ConnectionModel*> connections() const { return m_connections; }

signals:
    void connectionCreated(ConnectionModel *connection);
    void connectionDeleted(ConnectionModel *connection);

private slots:
    void onConnectionStarted(BlockView *startBlock);
    void onConnectionCompleted(BlockView *startBlock, BlockView *endBlock);
    void onBlockViewDestroyed(QObject *obj);

private:
    QGraphicsScene *m_scene;
    QList<BlockView *> m_blockViews;
    QList<ConnectionModel *> m_connections;
    QHash<ConnectionModel *, ConnectionView *> m_connectionViews;

    bool connectionExists(BlockView *start, BlockView *end);
    bool wouldCreateCycle(BlockModel *fromBlock, BlockModel *toBlock);
    bool canReach(BlockModel *fromBlock, BlockModel *targetBlock);
};

#endif // CONNECTIONCONTROLLER_H