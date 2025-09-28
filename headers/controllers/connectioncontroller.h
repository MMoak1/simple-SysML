#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QList>
#include <QHash>

class QGraphicsScene;
class BlockView;
class ConnectionModel;
class ConnectionView;

class ConnectionController : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionController(QGraphicsScene *scene, QObject *parent = nullptr);
    ~ConnectionController();

    void setConnectionMode(bool enabled);
    bool isConnectionModeEnabled() const { return m_connectionMode; }

    void registerBlockView(BlockView *view);
    void unregisterBlockView(BlockView *view);

    void clearAllConnections();

signals:
    void connectionModeChanged(bool enabled);

private slots:
    void onConnectionStarted(BlockView *startBlock);
    void onConnectionCompleted(BlockView *startBlock, BlockView *endBlock);
    void onBlockViewDestroyed(QObject *obj);

private:
    QGraphicsScene *m_scene;
    bool m_connectionMode;
    QList<BlockView *> m_blockViews;
    QList<ConnectionModel *> m_connections;
    QHash<ConnectionModel *, ConnectionView *> m_connectionViews;

    bool connectionExists(BlockView *start, BlockView *end);
};

#endif // CONNECTIONCONTROLLER_H