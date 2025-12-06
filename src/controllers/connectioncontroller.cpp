#include "../headers/controllers/connectioncontroller.h"
#include "../headers/views/blockview.h"
#include "../headers/models/connectionmodel.h"
#include "../headers/views/connectionview.h"
#include <QGraphicsScene>
#include <QDebug>
#include <QSet>
#include <QMessageBox>

ConnectionController::ConnectionController(QGraphicsScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene)
{
}

ConnectionController::~ConnectionController()
{
    clearAllConnections();
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

void ConnectionController::addConnection(ConnectionModel *connection)
{
    if (!connection)
        return;
    
    // Create view for the connection
    ConnectionView *connectionView = new ConnectionView(connection, nullptr);
    
    m_connections.append(connection);
    m_connectionViews[connection] = connectionView;
    m_scene->addItem(connectionView);
    
    // Emit signal for hierarchy controller
    emit connectionCreated(connection);
}

void ConnectionController::deleteConnection(ConnectionModel *connection)
{
    if (!connection)
        return;
    
    // Step 1: Remove from our tracking lists FIRST (before any signals or deletions)
    m_connections.removeOne(connection);
    ConnectionView *view = m_connectionViews.take(connection);
    
    // Step 2: Remove view from scene and delete it
    if (view)
    {
        m_scene->removeItem(view);
        delete view;
        view = nullptr;
    }
    
    // Step 3: Emit signal for hierarchy controller (connection is still valid here)
    emit connectionDeleted(connection);
    
    // Step 4: Finally delete model (LAST)
    delete connection;
}

void ConnectionController::deleteConnectionsForBlock(BlockModel *block)
{
    if (!block)
        return;
    
    // Find all connections involving this block
    QList<ConnectionModel *> connectionsToRemove;
    for (ConnectionModel *connection : m_connections)
    {
        // Safety check
        if (!connection)
            continue;
            
        if (connection->startBlock() == block || connection->endBlock() == block)
        {
            connectionsToRemove.append(connection);
        }
    }
    
    // Delete each connection (check null in case of concurrent modification)
    for (ConnectionModel *connection : connectionsToRemove)
    {
        if (connection)
            deleteConnection(connection);
    }
}

void ConnectionController::onConnectionStarted(BlockView *startBlock)
{
    qDebug() << "Connection started from block:" << startBlock->model()->label();
}

void ConnectionController::onConnectionCompleted(BlockView *startBlock, BlockView *endBlock)
{
    if (!startBlock || !endBlock)
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
        // Check if this connection would create a cycle
        if (wouldCreateCycle(startModel, endModel))
        {
            qDebug() << "Connection rejected: would create a cycle in the hierarchy";
            QMessageBox::warning(nullptr, "Cyclic Dependency",
                "Cannot create this connection.\n\n"
                "It would create a cyclic dependency in the hierarchy.");
            return;
        }
        
        ConnectionModel *connection = new ConnectionModel(startModel, endModel, this);
        ConnectionView *connectionView = new ConnectionView(connection, nullptr);

        m_connections.append(connection);
        m_connectionViews[connection] = connectionView;
        m_scene->addItem(connectionView);

        qDebug() << "Connection created successfully";

        // Emit signal for hierarchy controller
        emit connectionCreated(connection);
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

        // Emit signal before deleting
        emit connectionDeleted(connection);

        delete connection;
    }
}

bool ConnectionController::connectionExists(BlockView *start, BlockView *end)
{
    // Only check directional connection (start->end), not bidirectional
    for (ConnectionModel *connection : m_connections)
    {
        if (connection->startBlock() == start->model() &&
            connection->endBlock() == end->model())
        {
            return true;
        }
    }
    return false;
}

bool ConnectionController::wouldCreateCycle(BlockModel *fromBlock, BlockModel *toBlock)
{
    // Adding a connection from fromBlock to toBlock creates a cycle if
    // toBlock can already reach fromBlock through existing connections.
    // In other words: if there's a path from toBlock to fromBlock,
    // then adding fromBlock -> toBlock would complete a cycle.
    return canReach(toBlock, fromBlock);
}

bool ConnectionController::canReach(BlockModel *fromBlock, BlockModel *targetBlock)
{
    if (!fromBlock || !targetBlock)
        return false;
    
    if (fromBlock == targetBlock)
        return true;
    
    // DFS to check if we can reach targetBlock from fromBlock
    QSet<BlockModel*> visited;
    QList<BlockModel*> stack;
    stack.append(fromBlock);
    
    while (!stack.isEmpty())
    {
        BlockModel *current = stack.takeLast();
        
        if (current == targetBlock)
            return true;
        
        if (visited.contains(current))
            continue;
        
        visited.insert(current);
        
        // Find all blocks that can be reached from current (its children via connections)
        for (ConnectionModel *conn : m_connections)
        {
            if (conn->startBlock() == current && !visited.contains(conn->endBlock()))
            {
                stack.append(conn->endBlock());
            }
        }
    }
    
    return false;
}