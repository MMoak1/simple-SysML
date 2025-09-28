#include "../headers/controllers/connectioncontroller.h"
#include "../headers/views/blockview.h"
#include "../headers/models/connectionmodel.h"
#include "../headers/views/connectionview.h"
#include <QGraphicsScene>
#include <QDebug>

ConnectionController::ConnectionController(QGraphicsScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene), m_connectionMode(false)
{
}

ConnectionController::~ConnectionController()
{
    clearAllConnections();
}

void ConnectionController::setConnectionMode(bool enabled)
{
    if (m_connectionMode != enabled)
    {
        m_connectionMode = enabled;
        emit connectionModeChanged(enabled);
    }
}

void ConnectionController::registerBlockView(BlockView *view)
{
    if (!m_blockViews.contains(view))
    {
        m_blockViews.append(view);
        connect(view, &BlockView::connectionStarted, this, &ConnectionController::onConnectionStarted);
        connect(view, &BlockView::connectionCompleted, this, &ConnectionController::onConnectionCompleted);
        connect(view, &QObject::destroyed, this, &ConnectionController::onBlockViewDestroyed);
    }
}

void ConnectionController::unregisterBlockView(BlockView *view)
{
    m_blockViews.removeOne(view);
    disconnect(view, &BlockView::connectionStarted, this, &ConnectionController::onConnectionStarted);
    disconnect(view, &BlockView::connectionCompleted, this, &ConnectionController::onConnectionCompleted);
    disconnect(view, &QObject::destroyed, this, &ConnectionController::onBlockViewDestroyed);
}

void ConnectionController::clearAllConnections()
{
    // Remove all connection views from the scene
    for (auto it = m_connectionViews.begin(); it != m_connectionViews.end(); ++it)
    {
        m_scene->removeItem(it.value());
        delete it.value();
    }
    m_connectionViews.clear();

    // Delete all connection models
    qDeleteAll(m_connections);
    m_connections.clear();
}

void ConnectionController::onConnectionStarted(BlockView *startBlock)
{
    if (!m_connectionMode)
    {
        qDebug() << "Connection mode not enabled";
        return;
    }

    qDebug() << "Connection started from block:" << startBlock->model()->label();
}

void ConnectionController::onConnectionCompleted(BlockView *startBlock, BlockView *endBlock)
{
    if (!m_connectionMode || !startBlock)
    {
        qDebug() << "Connection mode not enabled or no start block";
        return;
    }

    if (!endBlock)
    {
        qDebug() << "No end block found";
        return;
    }

    qDebug() << "Connection completed from" << startBlock->model()->label() << "to" << endBlock->model()->label();

    // Check if connection already exists
    if (connectionExists(startBlock, endBlock))
    {
        qDebug() << "Connection already exists";
        return;
    }

    // Create new connection
    BlockModel *startModel = startBlock->model();
    BlockModel *endModel = endBlock->model();

    if (startModel && endModel)
    {
        ConnectionModel *connection = new ConnectionModel(startModel, endModel, this);
        ConnectionView *connectionView = new ConnectionView(connection, nullptr);

        m_connections.append(connection);
        m_connectionViews[connection] = connectionView;
        m_scene->addItem(connectionView);

        qDebug() << "Connection created successfully";
    }
}

void ConnectionController::onBlockViewDestroyed(QObject *obj)
{
    BlockView *view = static_cast<BlockView *>(obj);
    unregisterBlockView(view);

    // Remove any connections involving this block
    QList<ConnectionModel *> connectionsToRemove;

    for (ConnectionModel *connection : m_connections)
    {
        if (connection->startBlock() == view->model() ||
            connection->endBlock() == view->model())
        {
            connectionsToRemove.append(connection);
        }
    }

    for (ConnectionModel *connection : connectionsToRemove)
    {
        ConnectionView *view = m_connectionViews.value(connection);
        if (view)
        {
            m_scene->removeItem(view);
            delete view;
            m_connectionViews.remove(connection);
        }

        m_connections.removeOne(connection);
        delete connection;
    }
}

bool ConnectionController::connectionExists(BlockView *start, BlockView *end)
{
    for (ConnectionModel *connection : m_connections)
    {
        if ((connection->startBlock() == start->model() && connection->endBlock() == end->model()) ||
            (connection->startBlock() == end->model() && connection->endBlock() == start->model()))
        {
            return true;
        }
    }
    return false;
}