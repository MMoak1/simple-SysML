#include "../headers/mainwindow.h"
#include <QListWidgetItem>
#include <QPixmap>
#include <QIcon>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include "../headers/io/modelserializer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QDebug>
#include <QPointer>
#include "views/startmenu.h"
#include "models/statemachinemodel.h"
#include "../headers/models/statemodel.h"
#include "../headers/views/stateview.h"
#include "../headers/views/connectionview.h"
#include "../headers/views/transitionview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Initialize members
    scene = nullptr;
    splitter = nullptr;
    blockMenuView = nullptr;
    dropGraphicsView = nullptr;
    hierarchyTreeView = nullptr;
    dropController = nullptr;
    menuController = nullptr;
    connectionController = nullptr;
    hierarchyController = nullptr;
    startMenu = nullptr;
    m_menuBar = nullptr;
    m_fileMenu = nullptr;
    m_newAction = nullptr;
    m_stateMachineScene = nullptr;
    m_stateMachineController = nullptr;
    m_diagramViewController = nullptr;
    m_backButton = nullptr;
    m_viewTitleLabel = nullptr;
    m_navigationBar = nullptr;

    // Create start menu
    startMenu = new StartMenu(this);
    setCentralWidget(startMenu);
    connect(startMenu, &StartMenu::beginDrawing, this, &MainWindow::newFile);
    setupToolInterface();
}

void MainWindow::setupToolInterface()
{
    // Create diagram view controller for navigation
    m_diagramViewController = new DiagramViewController(this);
    connect(m_diagramViewController, &DiagramViewController::viewChanged, 
            this, &MainWindow::onViewChanged);

    // Create the BDD graphics scene
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::lightGray);

    // Create the state machine scene
    m_stateMachineScene = new QGraphicsScene(this);
    m_stateMachineScene->setBackgroundBrush(QColor(245, 245, 250)); // Slightly different background

    // Create the graphics view
    dropGraphicsView = new DropGraphicsView(scene, this);
    dropGraphicsView->setSceneRect(-400, -300, 800, 600);

    // Create the block menu view
    blockMenuView = new BlockMenuView(this);

    // Create the hierarchy tree view
    hierarchyTreeView = new HierarchyTreeView(this);

    // Add items with colored rectangle previews
    QListWidgetItem *redItem = new QListWidgetItem("Red Block");
    QPixmap redPixmap(50, 30);
    redPixmap.fill(Qt::red);
    redItem->setIcon(QIcon(redPixmap));
    blockMenuView->addItem(redItem);

    QListWidgetItem *blueItem = new QListWidgetItem("Blue Block");
    QPixmap bluePixmap(50, 30);
    bluePixmap.fill(Qt::blue);
    blueItem->setIcon(QIcon(bluePixmap));
    blockMenuView->addItem(blueItem);

    QListWidgetItem *greenItem = new QListWidgetItem("Green Block");
    QPixmap greenPixmap(50, 30);
    greenPixmap.fill(Qt::green);
    greenItem->setIcon(QIcon(greenPixmap));
    blockMenuView->addItem(greenItem);

    // Create controllers
    connectionController = new ConnectionController(scene, this);
    dropController = new DropController(scene, dropGraphicsView, connectionController, this);
    menuController = new MenuController(blockMenuView, this);

    // Create state machine controller
    m_stateMachineController = new StateMachineController(m_stateMachineScene, this);

    // Create hierarchy controller AFTER other controllers
    hierarchyController = new HierarchyController(hierarchyTreeView, scene, dropGraphicsView, this);

    // Connect drop signal to MainWindow for routing based on current view
    connect(dropGraphicsView, &DropGraphicsView::dropPerformed, this, &MainWindow::onDropPerformed);

    // Connect hierarchy controller to drop controller
    connect(dropController, &DropController::blockCreated,
            hierarchyController, &HierarchyController::onBlockCreated);
    connect(dropController, &DropController::blockDeleted,
            hierarchyController, &HierarchyController::onBlockDeleted);

    // Connect hierarchy controller to connection controller
    connect(connectionController, &ConnectionController::connectionCreated,
            hierarchyController, &HierarchyController::onConnectionCreated);
    connect(connectionController, &ConnectionController::connectionDeleted,
            hierarchyController, &HierarchyController::onConnectionDeleted);

    // Connect block creation to state machine entry signal
    connect(dropController, &DropController::blockCreated, this, [this](BlockModel *model, BlockView *view) {
        // Connect each new block view's double-click to enter state machine
        connect(view, &BlockView::enterStateMachineRequested, this, &MainWindow::onEnterStateMachine);
    });

    // Create navigation bar
    m_navigationBar = new QWidget(this);
    QHBoxLayout *navLayout = new QHBoxLayout(m_navigationBar);
    navLayout->setContentsMargins(5, 5, 5, 5);
    
    m_backButton = new QToolButton(this);
    m_backButton->setText("← Back");
    m_backButton->setToolTip("Return to Block Definition Diagram");
    m_backButton->setVisible(false); // Hidden initially
    connect(m_backButton, &QToolButton::clicked, this, &MainWindow::onNavigateBack);
    
    m_viewTitleLabel = new QLabel("Block Definition Diagram", this);
    QFont titleFont = m_viewTitleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    m_viewTitleLabel->setFont(titleFont);
    
    navLayout->addWidget(m_backButton);
    navLayout->addWidget(m_viewTitleLabel);
    navLayout->addStretch();

    // Create main layout with navigation bar
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    mainLayout->addWidget(m_navigationBar);

    // Create horizontal splitter with THREE widgets for layout
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(hierarchyTreeView);
    splitter->addWidget(blockMenuView);
    splitter->addWidget(dropGraphicsView);
    splitter->setStretchFactor(0, 1); // Hierarchy tree: 20%
    splitter->setStretchFactor(1, 1); // Block menu: 20%
    splitter->setStretchFactor(2, 3); // Graphics view: 60%

    mainLayout->addWidget(splitter);

    // Setup menu bar
    m_menuBar = menuBar();
    m_fileMenu = m_menuBar->addMenu(tr("&File"));
    
    m_newAction = new QAction(tr("&New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    m_fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    m_fileMenu->addAction(m_openAction);
    
    m_saveAction = new QAction(tr("&Save..."), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    m_fileMenu->addAction(m_saveAction);

    // Setup delete shortcut
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::deleteSelectedItems);

    // Set the main widget as the central widget
    setCentralWidget(mainWidget);
}

void MainWindow::newFile()
{
    // Clear existing scene and start fresh
    if (scene) {
        scene->clear();
    }
    if (m_stateMachineScene) {
        m_stateMachineScene->clear();
    }
    
    // Reset to BDD view
    if (m_diagramViewController) {
        while (m_diagramViewController->canGoBack()) {
            m_diagramViewController->goBack();
        }
    }
    switchToBDDView();
    
    // Hide start menu and show tool interface
    if (startMenu) {
        startMenu->hide();
    }
    setupToolInterface();
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open SysML Model"), QString(), tr("JSON Files (*.json);;All Files (*)"));
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // Clear existing model first
    if (scene) {
        scene->clear();
    }
    if (m_stateMachineScene) {
        m_stateMachineScene->clear();
    }
    if (dropController) {
        dropController->clear();
    }
    if (connectionController) {
        connectionController->clearAllConnections();
    }
    
    // Reset navigation to BDD
    if (m_diagramViewController) {
        while (m_diagramViewController->canGoBack()) {
            m_diagramViewController->goBack();
        }
    }
    
    // Load from file
    QList<BlockModel*> blocks;
    QList<ConnectionModel*> connections;
    
    if (ModelSerializer::loadFromFile(filePath, blocks, connections)) {
        // Add loaded blocks
        for (BlockModel *block : blocks) {
            dropController->addBlock(block);
        }
        
        // Add loaded connections
        for (ConnectionModel *conn : connections) {
            connectionController->addConnection(conn);
        }
        
        qDebug() << "Loaded" << blocks.size() << "blocks and" << connections.size() << "connections from" << filePath;
        QMessageBox::information(this, tr("Success"), 
            tr("Model loaded successfully!\n%1 blocks, %2 connections").arg(blocks.size()).arg(connections.size()));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load model from file."));
    }
}

void MainWindow::saveFile()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Save SysML Model"), QString(), tr("JSON Files (*.json);;All Files (*)"));
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // Ensure .json extension
    if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
        filePath += ".json";
    }
    
    // Get blocks and connections from controllers
    QList<BlockModel*> blocks = dropController->blocks();
    QList<ConnectionModel*> connections = connectionController->connections();
    
    if (ModelSerializer::saveToFile(filePath, blocks, connections)) {
        qDebug() << "Saved" << blocks.size() << "blocks and" << connections.size() << "connections to" << filePath;
        QMessageBox::information(this, tr("Success"), 
            tr("Model saved successfully!\n%1 blocks, %2 connections").arg(blocks.size()).arg(connections.size()));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save model to file."));
    }
}

void MainWindow::onEnterStateMachine(BlockView *blockView)
{
    if (!blockView || !blockView->model())
        return;

    BlockModel *block = blockView->model();
    qDebug() << "MainWindow: Entering state machine for block:" << block->label();
    
    // Navigate to state machine view
    m_diagramViewController->enterStateMachine(block);
    switchToStateMachineView(block);
}

void MainWindow::onNavigateBack()
{
    qDebug() << "MainWindow: Navigating back";
    m_diagramViewController->goBack();
    
    if (m_diagramViewController->currentViewType() == DiagramContext::Type::BDD)
    {
        switchToBDDView();
    }
}

void MainWindow::onViewChanged()
{
    // Update the title label
    m_viewTitleLabel->setText(m_diagramViewController->currentTitle());
    
    // Update back button visibility
    m_backButton->setVisible(m_diagramViewController->canGoBack());
}

void MainWindow::switchToBDDView()
{
    qDebug() << "Switching to BDD view";
    
    // Switch the scene in the graphics view
    dropGraphicsView->setScene(scene);
    
    // Show block menu (for creating blocks in BDD)
    blockMenuView->setVisible(true);
    
    // Update UI
    onViewChanged();
}

void MainWindow::switchToStateMachineView(BlockModel *block)
{
    qDebug() << "Switching to State Machine view for:" << block->label();
    
    // Get or create the state machine for this block
    StateMachineModel *stateMachine = block->getOrCreateStateMachine();
    
    // Set up the state machine controller with this state machine
    m_stateMachineController->setStateMachine(stateMachine);
    
    // Switch the scene in the graphics view
    dropGraphicsView->setScene(m_stateMachineScene);
    
    // Hide regular block menu (state machine has different elements)
    // For now, we'll keep it visible but could replace with state menu later
    blockMenuView->setVisible(true);
    
    // Update UI
    onViewChanged();
}

void MainWindow::onDropPerformed(const QString &itemType, const QPointF &position)
{
    // Route drops to the appropriate controller based on current view
    if (m_diagramViewController->currentViewType() == DiagramContext::Type::StateMachine)
    {
        // In state machine view - create states
        qDebug() << "MainWindow: Routing drop to StateMachineController";
        m_stateMachineController->handleDrop(itemType, position);
    }
    else
    {
        // In BDD view - create blocks
        qDebug() << "MainWindow: Routing drop to DropController";
        dropController->handleDrop(itemType, position);
    }
}

void MainWindow::deleteSelectedItems()
{
    // Get the current scene based on view type
    QGraphicsScene *currentScene = nullptr;
    if (m_diagramViewController->currentViewType() == DiagramContext::Type::StateMachine)
    {
        currentScene = m_stateMachineScene;
    }
    else
    {
        currentScene = scene;
    }
    
    if (!currentScene)
        return;
    
    QList<QGraphicsItem*> selected = currentScene->selectedItems();
    if (selected.isEmpty())
    {
        qDebug() << "MainWindow: No items selected for deletion";
        return;
    }
    
    qDebug() << "MainWindow: Deleting" << selected.size() << "selected items";
    
    // Solution D: Use QPointer for safe deletion tracking
    // QPointer automatically becomes null when the QObject is deleted,
    // preventing crashes from cascade deletes
    QList<QPointer<ConnectionView>> connectionsToDelete;
    QList<QPointer<TransitionView>> transitionsToDelete;
    QList<QPointer<BlockView>> blocksToDelete;
    QList<QPointer<StateView>> statesToDelete;
    
    // Separate items by type
    for (QGraphicsItem* item : selected)
    {
        if (BlockView* blockView = dynamic_cast<BlockView*>(item))
        {
            blocksToDelete.append(QPointer<BlockView>(blockView));
        }
        else if (StateView* stateView = dynamic_cast<StateView*>(item))
        {
            statesToDelete.append(QPointer<StateView>(stateView));
        }
        else if (ConnectionView* connView = dynamic_cast<ConnectionView*>(item))
        {
            connectionsToDelete.append(QPointer<ConnectionView>(connView));
        }
        else if (TransitionView* transView = dynamic_cast<TransitionView*>(item))
        {
            transitionsToDelete.append(QPointer<TransitionView>(transView));
        }
    }
    
    // Delete connections first (QPointer will become null if cascade-deleted by blocks)
    for (const QPointer<ConnectionView>& connView : connectionsToDelete)
    {
        if (connView && connView->model())
        {
            qDebug() << "MainWindow: Deleting connection";
            connectionController->deleteConnection(connView->model());
        }
    }
    
    // Delete transitions
    for (const QPointer<TransitionView>& transView : transitionsToDelete)
    {
        if (transView && transView->model())
        {
            qDebug() << "MainWindow: Deleting transition";
            m_stateMachineController->deleteTransition(transView->model());
        }
    }
    
    // Delete blocks (may cascade-delete remaining connections - QPointers handle this safely)
    for (const QPointer<BlockView>& blockView : blocksToDelete)
    {
        if (blockView && blockView->model())
        {
            BlockModel* block = blockView->model();
            qDebug() << "MainWindow: Deleting block:" << block->label();
            
            // Clear state machine controller if needed
            if (block->hasStateMachine() && 
                m_stateMachineController->currentStateMachine() == block->stateMachine())
            {
                qDebug() << "MainWindow: Clearing state machine controller before deletion";
                m_stateMachineController->setStateMachine(nullptr);
            }
            
            dropController->deleteBlock(block);
        }
    }
    
    // Delete states
    for (const QPointer<StateView>& stateView : statesToDelete)
    {
        if (stateView && stateView->model())
        {
            qDebug() << "MainWindow: Deleting state:" << stateView->model()->label();
            m_stateMachineController->deleteState(stateView->model());
        }
    }
}

MainWindow::~MainWindow()
{
}
