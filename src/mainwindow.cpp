#include "../headers/mainwindow.h"
#include <QPainter>
#include "../headers/blockmenu.h"
#include "../headers/dropgraphicsview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the graphics scene
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::lightGray); // Optional: light background for scene

    // Create the graphics view
    view = new DropGraphicsView(scene, this);
    view->setSceneRect(-400, -300, 800, 600); // Set a reasonable scene rectangle

    // Create the block menu (left panel)
    blockMenu = new BlockMenu(this);

    // Add items with colored rectangle previews
    QListWidgetItem *redItem = new QListWidgetItem("Red Block");
    QPixmap redPixmap(50, 30);
    redPixmap.fill(Qt::red);
    redItem->setIcon(QIcon(redPixmap));
    blockMenu->addItem(redItem);

    QListWidgetItem *blueItem = new QListWidgetItem("Blue Block");
    QPixmap bluePixmap(50, 30);
    bluePixmap.fill(Qt::blue);
    blueItem->setIcon(QIcon(bluePixmap));
    blockMenu->addItem(blueItem);

    QListWidgetItem *greenItem = new QListWidgetItem("Green Block");
    QPixmap greenPixmap(50, 30);
    greenPixmap.fill(Qt::green);
    greenItem->setIcon(QIcon(greenPixmap));
    blockMenu->addItem(greenItem);

    // Create horizontal splitter for layout
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(blockMenu);
    splitter->addWidget(view);
    splitter->setStretchFactor(0, 1); // Left panel takes less space
    splitter->setStretchFactor(1, 3); // Right panel takes more space

    // Set the splitter as the central widget
    setCentralWidget(splitter);

    // Window configuration
    setWindowTitle("Basic Modeling Tool");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}
