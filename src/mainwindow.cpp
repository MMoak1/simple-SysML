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
#include "views/startmenu.h"
#include "models/statemachinemodel.h"

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
        tr("Open SysML Model"), QString(), tr("SysML Files (*.sysml);;JSON Files (*.json);;All Files (*)"));
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // TODO: Implement full load functionality
    QMessageBox::information(this, tr("Open"), tr("Open functionality - file selected: ") + filePath);
}

void MainWindow::saveFile()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Save SysML Model"), QString(), tr("SysML Files (*.sysml);;JSON Files (*.json);;All Files (*)"));
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // TODO: Implement full save functionality
    QMessageBox::information(this, tr("Save"), tr("Save functionality - file selected: ") + filePath);
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

MainWindow::~MainWindow()
{
}
