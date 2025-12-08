#include "../../headers/controllers/connectioncontroller.h"
#include "../../headers/views/blockdefinitionview.h"
#include "../../headers/models/blockdefinition.h"
#include "../../headers/models/partproperty.h"
#include "../../headers/views/connectionview.h"
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

void ConnectionController::registerBlockView(BlockDefinitionView *view)
{
    if (!m_blockViews.contains(view))
    {
        m_blockViews.append(view);
        connect(view, &BlockDefinitionView::connectionStarted, this, &ConnectionController::onConnectionStarted);
        connect(view, &BlockDefinitionView::connectionCompleted, this, &ConnectionController::onConnectionCompleted);
        connect(view, &QObject::destroyed, this, &ConnectionController::onBlockViewDestroyed);
    }
}

void ConnectionController::unregisterBlockView(BlockDefinitionView *view)
{
    m_blockViews.removeOne(view);
    disconnect(view, &BlockDefinitionView::connectionStarted, this, &ConnectionController::onConnectionStarted);
    disconnect(view, &BlockDefinitionView::connectionCompleted, this, &ConnectionController::onConnectionCompleted);
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

    // Delete all part properties? 
    // Wait, PartProperties are owned by BlockDefinition. 
    // If we just clear the diagram, do we delete the underlying semantic model?
    // In "clear()" context (e.g. new file), yes.
    // But ConnectionController logic usually assumes ownership or management.
    // DropController clears Definitions. If Definitions are deleted, they delete their parts.
    // So Connections might be double deleted if we delete them here AND DropController deletes blocks.
    // However, DropController calls clear() which clears definitions.
    // ConnectionController should mostly clear its views.
    // But if we are just clearing connections, we should remove them from the blocks.
    
    // For now, let's assume this is called during full clear. 
    // We'll clear our tracking. Actual deletion might happen via BlockDefinition.
    // BUT, if we are deleting a connection explicitly, we must remove it from owner.
    
    m_connections.clear();
}

void ConnectionController::addConnection(PartProperty *partProperty)
{
    if (!partProperty)
        return;
    
    // Check if we already track this
    if (m_connections.contains(partProperty))
        return;

    // Create view for the connection
    ConnectionView *connectionView = new ConnectionView(partProperty, nullptr);
    
    m_connections.append(partProperty);
    m_connectionViews[partProperty] = connectionView;
    m_scene->addItem(connectionView);
    
    // Emit signal for hierarchy controller
    emit connectionCreated(partProperty);
}

void ConnectionController::deleteConnection(PartProperty *partProperty)
{
    if (!partProperty)
        return;
    
    // Step 1: Remove from our tracking lists FIRST
    m_connections.removeOne(partProperty);
    ConnectionView *view = m_connectionViews.take(partProperty);
    
    // Step 2: Remove view from scene and delete it
    if (view)
    {
        m_scene->removeItem(view);
        delete view;
        view = nullptr;
    }
    
    // Step 3: Emit signal (while partProperty is still valid)
    emit connectionDeleted(partProperty);
    
    // Step 4: Remove from owner BlockDefinition (this will delete the PartProperty)
    if (partProperty->owner())
    {
        partProperty->owner()->removePartProperty(partProperty);
    }
    // Note: removePartProperty calls deleteLater() on the part, so it's scheduled for deletion.
}

void ConnectionController::removeConnectionView(PartProperty *partProperty)
{
    // This method ONLY removes the view, it does NOT delete the PartProperty.
    // Use this when PartProperty will be deleted by BlockDefinition (cascade delete).
    if (!partProperty)
        return;
    
    m_connections.removeOne(partProperty);
    ConnectionView *view = m_connectionViews.take(partProperty);
    
    if (view)
    {
        m_scene->removeItem(view);
        delete view;
    }
    
    // Emit signal - the PartProperty object is still valid but being deleted
    emit connectionDeleted(partProperty);
}

void ConnectionController::deleteConnectionsForBlock(BlockDefinition *definition)
{
    if (!definition)
        return;
    
    qDebug() << "ConnectionController: Deleting all connections for block:" << definition->typeName();
    
    // Collect connections to remove (connections where this block is owner or type)
    QList<PartProperty *> connectionsToRemove;
    
    for (PartProperty *conn : m_connections)
    {
        if (conn->owner() == definition || conn->type() == definition)
        {
            connectionsToRemove.append(conn);
        }
    }
    
    // For connections WHERE this block is the OWNER:
    // The PartProperty will be deleted when BlockDefinition is deleted (qDeleteAll in destructor)
    // So we just remove the views.
    
    // For connections WHERE this block is the TYPE (target):
    // We need to fully delete those connections (remove from their owner too)
    
    for (PartProperty *conn : connectionsToRemove)
    {
        if (conn->owner() == definition)
        {
            // Owner is being deleted, PartProperty will be deleted by ~BlockDefinition
            // Just clean up our view
            removeConnectionView(conn);
        }
        else if (conn->type() == definition)
        {
            // This block is the target type, remove the connection from its owner
            deleteConnection(conn);
        }
    }
}

void ConnectionController::onConnectionStarted(BlockDefinitionView *startBlock)
{
    qDebug() << "Connection started from block:" << startBlock->definition()->typeName();
}

void ConnectionController::onConnectionCompleted(BlockDefinitionView *startBlock, BlockDefinitionView *endBlock)
{
    if (!startBlock || !endBlock)
    {
        qDebug() << "No end block found";
        return;
    }

    qDebug() << "Connection completed from" << startBlock->definition()->typeName() << "to" << endBlock->definition()->typeName();

    // Check if connection already exists?
    // In SysML, you can have multiple parts of the same type.
    // But maybe we want to prevent redundant identical connections for MVP?
    // Let's allow it, but maybe check for cycle.

    // Create new connection (Part Property)
    BlockDefinition *startDef = startBlock->definition();
    BlockDefinition *endDef = endBlock->definition();

    if (startDef && endDef)
    {
        // Check if this connection would create a cycle (Composition Hierarchy)
        if (wouldCreateCycle(startDef, endDef))
        {
            qDebug() << "Connection rejected: would create a cycle in the hierarchy";
            QMessageBox::warning(nullptr, "Recursive Composition",
                "Cannot add this part property.\n\n"
                "It would create a recursive composition cycle (a block cannot contain itself).");
            return;
        }
        
        // Generate unique name
        QString partName = startDef->generateUniquePartName(endDef);
        
        // Add PartProperty to startDef
        PartProperty *part = startDef->addPartProperty(partName, endDef);
        
        // Add to our controller (create view)
        addConnection(part);

        qDebug() << "Connection (PartProperty) created successfully";
    }
}

void ConnectionController::onBlockViewDestroyed(QObject *obj)
{
    // Note: When this is called, the BlockDefinitionView is being destroyed.
    // The BlockDefinition might already be deleted or in the process of deletion.
    // We should NOT try to access the definition's data or delete PartProperties here.
    // Just clean up our tracking of the view.
    
    BlockDefinitionView *view = static_cast<BlockDefinitionView *>(obj);
    
    // Just unregister the view - don't try to delete connections
    // Connection cleanup should be handled by deleteConnectionsForBlock() which is called
    // BEFORE the block is deleted (by DropController::deleteBlock)
    m_blockViews.removeOne(view);
    
    // Don't disconnect signals - the object is being destroyed anyway
}

bool ConnectionController::connectionExists(BlockDefinitionView *start, BlockDefinitionView *end)
{
    // Not strictly enforced for PartProperties
    return false;
}

bool ConnectionController::wouldCreateCycle(BlockDefinition *fromBlock, BlockDefinition *toBlock)
{
    // Adding a part of type 'toBlock' to 'fromBlock' creates a cycle if
    // 'toBlock' already contains 'fromBlock' (directly or indirectly).
    // So we check if 'fromBlock' is reachable from 'toBlock'.
    return canReach(toBlock, fromBlock);
}

bool ConnectionController::canReach(BlockDefinition *fromBlock, BlockDefinition *targetBlock)
{
    if (!fromBlock || !targetBlock)
        return false;
    
    if (fromBlock == targetBlock)
        return true;
    
    // DFS to check reachability via composition (PartProperties)
    QSet<BlockDefinition*> visited;
    QList<BlockDefinition*> stack;
    stack.append(fromBlock);
    
    while (!stack.isEmpty())
    {
        BlockDefinition *current = stack.takeLast();
        
        if (current == targetBlock)
            return true;
        
        if (visited.contains(current))
            continue;
        
        visited.insert(current);
        
        // Check all parts of current block
        for (PartProperty *part : current->partProperties())
        {
            if (part->type() && !visited.contains(part->type()))
            {
                stack.append(part->type());
            }
        }
    }
    
    return false;
}