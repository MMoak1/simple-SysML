# Hierarchical Tree View - Implementation Plan

## Implementation Order

This document provides the step-by-step implementation plan for adding the hierarchical tree view feature to the modeling tool.

## Phase 1: Core Components Creation

### Step 1: Create HierarchyTreeView Class

**Files to Create**:
- `headers/views/hierarchytreeview.h`
- `src/views/hierarchytreeview.cpp`

**Key Implementation Details**:

```cpp
// hierarchytreeview.h
class HierarchyTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit HierarchyTreeView(QWidget *parent = nullptr);
    
    // Block management
    void addBlock(BlockModel *block);
    void removeBlock(BlockModel *block);
    QTreeWidgetItem* findBlockItem(BlockModel *block);
    
    // Hierarchy management
    void clearHierarchy();
    void setBlockAsRoot(BlockModel *block);
    void setBlockAsChild(BlockModel *parent, BlockModel *child);
    
    // Selection and highlighting
    void selectBlock(BlockModel *block);
    void clearSelection();

signals:
    void blockItemClicked(BlockModel *block);
    void blockItemDoubleClicked(BlockModel *block);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QMap<QString, QTreeWidgetItem*> m_blockItems;  // blockId -> tree item
    QMap<QTreeWidgetItem*, BlockModel*> m_itemToBlock;  // tree item -> block
    
    void setupTreeWidget();
    QIcon createColorIcon(const QColor &color);
};
```

**Implementation Notes**:
- Use `QTreeWidget` as base class for built-in tree functionality
- Store bidirectional mapping between blocks and tree items
- Create colored square icons to match block colors
- Handle single-click for selection, double-click for focus/zoom

### Step 2: Create HierarchyController Class

**Files to Create**:
- `headers/controllers/hierarchycontroller.h`
- `src/controllers/hierarchycontroller.cpp`

**Key Implementation Details**:

```cpp
// hierarchycontroller.h
class HierarchyController : public QObject
{
    Q_OBJECT

public:
    explicit HierarchyController(
        HierarchyTreeView *treeView,
        QGraphicsScene *scene,
        QObject *parent = nullptr
    );
    
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

private:
    HierarchyTreeView *m_treeView;
    QGraphicsScene *m_scene;
    
    // Data structures
    QMap<QString, BlockModel*> m_blocks;           // blockId -> BlockModel
    QMap<QString, BlockView*> m_blockViews;        // blockId -> BlockView
    QList<ConnectionModel*> m_connections;
    
    // Hierarchy tracking
    QMap<QString, QSet<QString>> m_childrenMap;    // parentId -> set of childIds
    QMap<QString, QSet<QString>> m_parentsMap;     // childId -> set of parentIds
    
    // Helper methods
    void buildHierarchyMaps();
    QList<BlockModel*> getRootBlocks();
    QList<BlockModel*> getChildBlocks(BlockModel *parent);
    bool hasIncomingConnections(BlockModel *block);
    void addBlockToTree(BlockModel *block, QTreeWidgetItem *parentItem = nullptr);
    void highlightBlockInScene(BlockModel *block);
};
```

**Implementation Notes**:
- Maintain separate maps for parent-child relationships
- Support multiple parents per block (store in sets)
- Rebuild hierarchy when connections change
- Coordinate selection between tree and scene

## Phase 2: Integration with Existing Components

### Step 3: Modify ConnectionController

**File**: `headers/controllers/connectioncontroller.h`

**Changes**:
```cpp
// Add signals
signals:
    void connectionCreated(ConnectionModel *connection);
    void connectionDeleted(ConnectionModel *connection);
```

**File**: `src/controllers/connectioncontroller.cpp`

**Changes**:
```cpp
void ConnectionController::onConnectionCompleted(BlockView *startBlock, BlockView *endBlock)
{
    // ... existing code ...
    
    // After creating connection
    emit connectionCreated(connection);
}

// Add method for connection deletion
void ConnectionController::deleteConnection(ConnectionModel *connection)
{
    // Remove from scene
    ConnectionView *view = m_connectionViews.value(connection);
    if (view) {
        m_scene->removeItem(view);
        delete view;
        m_connectionViews.remove(connection);
    }
    
    // Remove from list
    m_connections.removeOne(connection);
    
    // Emit signal before deleting
    emit connectionDeleted(connection);
    
    delete connection;
}
```

### Step 4: Modify DropController

**File**: `headers/controllers/dropcontroller.h`

**Changes**:
```cpp
// Add signal
signals:
    void blockCreated(BlockModel *model, BlockView *view);
```

**File**: `src/controllers/dropcontroller.cpp`

**Changes**:
```cpp
void DropController::handleDrop(const QPointF &position, const QString &blockType, const QColor &color)
{
    // ... existing code to create block and view ...
    
    // After creating block and view
    emit blockCreated(blockModel, blockView);
}
```

### Step 5: Modify MainWindow

**File**: `headers/mainwindow.h`

**Changes**:
```cpp
#include "views/hierarchytreeview.h"
#include "controllers/hierarchycontroller.h"

class MainWindow : public QMainWindow
{
    // ... existing code ...
    
private:
    // ... existing members ...
    HierarchyTreeView *hierarchyTreeView;
    HierarchyController *hierarchyController;
};
```

**File**: `src/mainwindow.cpp`

**Changes**:
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ... existing initialization ...
    hierarchyTreeView = nullptr;
    hierarchyController = nullptr;
}

void MainWindow::setupToolInterface()
{
    // ... existing code for scene, dropGraphicsView, blockMenuView ...
    
    // Create hierarchy tree view
    hierarchyTreeView = new HierarchyTreeView(this);
    
    // Create controllers
    connectionController = new ConnectionController(scene, this);
    dropController = new DropController(scene, dropGraphicsView, connectionController, this);
    menuController = new MenuController(blockMenuView, this);
    
    // Create hierarchy controller AFTER other controllers
    hierarchyController = new HierarchyController(hierarchyTreeView, scene, this);
    
    // Connect signals
    connect(dropGraphicsView, &DropGraphicsView::dropPerformed, 
            dropController, &DropController::handleDrop);
    
    // Connect hierarchy controller to drop controller
    connect(dropController, &DropController::blockCreated,
            hierarchyController, &HierarchyController::onBlockCreated);
    
    // Connect hierarchy controller to connection controller
    connect(connectionController, &ConnectionController::connectionCreated,
            hierarchyController, &HierarchyController::onConnectionCreated);
    connect(connectionController, &ConnectionController::connectionDeleted,
            hierarchyController, &HierarchyController::onConnectionDeleted);
    
    // Create horizontal splitter with THREE widgets
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(blockMenuView);
    splitter->addWidget(dropGraphicsView);
    splitter->addWidget(hierarchyTreeView);
    
    // Set stretch factors (proportions)
    splitter->setStretchFactor(0, 1);  // Block menu: 20%
    splitter->setStretchFactor(1, 3);  // Graphics view: 60%
    splitter->setStretchFactor(2, 1);  // Hierarchy tree: 20%
    
    setCentralWidget(splitter);
}

void MainWindow::newFile()
{
    if (startMenu)
    {
        delete startMenu;
        startMenu = nullptr;
        setupToolInterface();
    }
    else
    {
        scene->clear();
        // Clear hierarchy tree
        if (hierarchyController) {
            hierarchyController->rebuildHierarchy();
        }
    }
}
```

## Phase 3: Detailed Implementation

### HierarchyTreeView Implementation Details

**Constructor Setup**:
```cpp
HierarchyTreeView::HierarchyTreeView(QWidget *parent)
    : QTreeWidget(parent)
{
    setupTreeWidget();
}

void HierarchyTreeView::setupTreeWidget()
{
    // Column configuration
    setColumnCount(1);
    setHeaderLabel("Block Hierarchy");
    
    // Visual styling
    setAlternatingRowColors(true);
    setAnimated(true);
    setExpandsOnDoubleClick(true);
    setIndentation(20);
    
    // Selection behavior
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Enable drag-drop (for future enhancement)
    setDragEnabled(false);
    setAcceptDrops(false);
    
    // Connect signals
    connect(this, &QTreeWidget::itemClicked, this, &HierarchyTreeView::onItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &HierarchyTreeView::onItemDoubleClicked);
}
```

**Block Management**:
```cpp
void HierarchyTreeView::addBlock(BlockModel *block)
{
    if (!block || m_blockItems.contains(block->id())) {
        return;
    }
    
    // Create tree item
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, block->label());
    item->setIcon(0, createColorIcon(block->color()));
    
    // Store mappings
    m_blockItems[block->id()] = item;
    m_itemToBlock[item] = block;
    
    // Connect to model updates
    connect(block, &BlockModel::labelChanged, this, [this, block, item](const QString &label) {
        item->setText(0, label);
    });
    
    connect(block, &BlockModel::colorChanged, this, [this, block, item](const QColor &color) {
        item->setIcon(0, createColorIcon(color));
    });
}

void HierarchyTreeView::removeBlock(BlockModel *block)
{
    if (!block) return;
    
    QTreeWidgetItem *item = m_blockItems.value(block->id());
    if (item) {
        // Remove from parent or root
        QTreeWidgetItem *parent = item->parent();
        if (parent) {
            parent->removeChild(item);
        } else {
            int index = indexOfTopLevelItem(item);
            if (index >= 0) {
                takeTopLevelItem(index);
            }
        }
        
        // Clean up mappings
        m_blockItems.remove(block->id());
        m_itemToBlock.remove(item);
        
        delete item;
    }
}

QIcon HierarchyTreeView::createColorIcon(const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    
    // Add border
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 15, 15);
    
    return QIcon(pixmap);
}
```

### HierarchyController Implementation Details

**Hierarchy Building**:
```cpp
void HierarchyController::rebuildHierarchy()
{
    // Clear existing tree structure
    m_treeView->clearHierarchy();
    
    // Rebuild parent-child maps from connections
    buildHierarchyMaps();
    
    // Get root blocks (blocks with no incoming connections)
    QList<BlockModel*> rootBlocks = getRootBlocks();
    
    // Add root blocks to tree
    for (BlockModel *block : rootBlocks) {
        addBlockToTree(block, nullptr);
    }
}

void HierarchyController::buildHierarchyMaps()
{
    m_childrenMap.clear();
    m_parentsMap.clear();
    
    for (ConnectionModel *conn : m_connections) {
        QString startId = conn->startBlockId();
        QString endId = conn->endBlockId();
        
        // Add to children map
        m_childrenMap[startId].insert(endId);
        
        // Add to parents map
        m_parentsMap[endId].insert(startId);
    }
}

QList<BlockModel*> HierarchyController::getRootBlocks()
{
    QList<BlockModel*> roots;
    
    for (BlockModel *block : m_blocks.values()) {
        if (!hasIncomingConnections(block)) {
            roots.append(block);
        }
    }
    
    return roots;
}

bool HierarchyController::hasIncomingConnections(BlockModel *block)
{
    return m_parentsMap.contains(block->id()) && 
           !m_parentsMap[block->id()].isEmpty();
}

void HierarchyController::addBlockToTree(BlockModel *block, QTreeWidgetItem *parentItem)
{
    if (!block) return;
    
    // Get or create tree item for this block
    QTreeWidgetItem *item = m_treeView->findBlockItem(block);
    
    if (parentItem) {
        // Add as child
        m_treeView->setBlockAsChild(block, parentItem);
    } else {
        // Add as root
        m_treeView->setBlockAsRoot(block);
    }
    
    // Recursively add children
    QList<BlockModel*> children = getChildBlocks(block);
    for (BlockModel *child : children) {
        addBlockToTree(child, item);
    }
}

QList<BlockModel*> HierarchyController::getChildBlocks(BlockModel *parent)
{
    QList<BlockModel*> children;
    
    if (!parent) return children;
    
    QSet<QString> childIds = m_childrenMap.value(parent->id());
    for (const QString &childId : childIds) {
        BlockModel *child = m_blocks.value(childId);
        if (child) {
            children.append(child);
        }
    }
    
    return children;
}
```

**Selection Handling**:
```cpp
void HierarchyController::onTreeItemClicked(BlockModel *block)
{
    if (!block) return;
    
    highlightBlockInScene(block);
}

void HierarchyController::highlightBlockInScene(BlockModel *block)
{
    if (!block) return;
    
    BlockView *view = m_blockViews.value(block->id());
    if (!view) return;
    
    // Clear previous selection
    m_scene->clearSelection();
    
    // Select the block
    view->setSelected(true);
    
    // Center view on the block (need to get the graphics view)
    // This requires access to the DropGraphicsView
    // Can be done by storing a reference or emitting a signal
}
```

## Phase 4: CMakeLists.txt Updates

**File**: `CMakeLists.txt`

**Changes**:
```cmake
# Add new source files
set(SOURCES
    # ... existing sources ...
    src/views/hierarchytreeview.cpp
    src/controllers/hierarchycontroller.cpp
)

# Add new header files
set(HEADERS
    # ... existing headers ...
    headers/views/hierarchytreeview.h
    headers/controllers/hierarchycontroller.h
)
```

## Phase 5: Testing Plan

### Test Case 1: Basic Hierarchy
1. Create Block A
2. Create Block B
3. Connect A → B
4. Verify: B appears under A in tree

### Test Case 2: Multi-Level Hierarchy
1. Create blocks A, B, C, D
2. Connect A → B, B → C, C → D
3. Verify: Tree shows A > B > C > D

### Test Case 3: Multiple Children
1. Create blocks A, B, C, D
2. Connect A → B, A → C, A → D
3. Verify: B, C, D all appear under A

### Test Case 4: Tree Selection
1. Create and connect blocks
2. Click on tree item
3. Verify: Block highlights in scene
4. Verify: View centers on block

### Test Case 5: Scene Selection
1. Create and connect blocks
2. Click on block in scene
3. Verify: Tree item highlights
4. Verify: Tree scrolls to show item

### Test Case 6: Connection Deletion
1. Create A → B
2. Delete connection
3. Verify: B moves to root level

### Test Case 7: Block Deletion
1. Create A → B → C
2. Delete B
3. Verify: C moves to root level
4. Verify: Connection A → B removed

### Test Case 8: Multiple Parents
1. Create A, B, C
2. Connect A → C, B → C
3. Verify: C appears under A (first parent)
4. Verify: Tooltip shows both parents

## Implementation Checklist

- [ ] Create `hierarchytreeview.h` and `.cpp`
- [ ] Create `hierarchycontroller.h` and `.cpp`
- [ ] Modify `connectioncontroller.h` to add signals
- [ ] Modify `connectioncontroller.cpp` to emit signals
- [ ] Modify `dropcontroller.h` to add signals
- [ ] Modify `dropcontroller.cpp` to emit signals
- [ ] Modify `mainwindow.h` to add members
- [ ] Modify `mainwindow.cpp` to integrate components
- [ ] Update `CMakeLists.txt` with new files
- [ ] Build and test basic functionality
- [ ] Test all test cases
- [ ] Handle edge cases
- [ ] Add error handling
- [ ] Document the feature

## Notes for Implementation

1. **Order Matters**: Create HierarchyController AFTER ConnectionController and DropController
2. **Signal Connections**: Ensure all signals are connected before any blocks are created
3. **Memory Management**: Use Qt parent-child ownership for automatic cleanup
4. **Thread Safety**: All operations should be on the main GUI thread
5. **Performance**: Rebuild hierarchy only when connections change, not on every block move