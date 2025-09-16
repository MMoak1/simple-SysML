#include "../headers/mainwindow.h"
#include <QListWidgetItem>
#include <QPixmap>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the graphics scene
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::lightGray);

    // Create the graphics view
    dropGraphicsView = new DropGraphicsView(scene, this);
    dropGraphicsView->setSceneRect(-400, -300, 800, 600);

    // Create the block menu view
    blockMenuView = new BlockMenuView(this);

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
    dropController = new DropController(scene, dropGraphicsView, this);
    menuController = new MenuController(blockMenuView, this);

    // Connect drop signal to controller
    connect(dropGraphicsView, &DropGraphicsView::dropPerformed, dropController, &DropController::handleDrop);

    // Create horizontal splitter for layout
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(blockMenuView);
    splitter->addWidget(dropGraphicsView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    // Set the splitter as the central widget
    setCentralWidget(splitter);

    // Window configuration
    setWindowTitle("Basic Modeling Tool");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}
