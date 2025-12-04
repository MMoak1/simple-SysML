#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSplitter>
#include <QGraphicsView>
#include <QToolButton>
#include <QLabel>

#include "views/blockmenuview.h"
#include "views/dropgraphicsview.h"
#include "views/startmenu.h"
#include "views/hierarchytreeview.h"
#include "models/blockmodel.h"
#include "controllers/dropcontroller.h"
#include "controllers/menucontroller.h"
#include "controllers/connectioncontroller.h"
#include "controllers/hierarchycontroller.h"
#include "controllers/diagramviewcontroller.h"
#include "controllers/statemachinecontroller.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void onEnterStateMachine(BlockView *blockView);
    void onNavigateBack();
    void onViewChanged();
    void onDropPerformed(const QString &itemType, const QPointF &position);

private:
    void setupToolInterface();
    void switchToBDDView();
    void switchToStateMachineView(BlockModel *block);

    // BDD (Block Definition Diagram) components
    QGraphicsScene *scene;
    QSplitter *splitter;
    BlockMenuView *blockMenuView;
    DropGraphicsView *dropGraphicsView;
    HierarchyTreeView *hierarchyTreeView;
    DropController *dropController;
    MenuController *menuController;
    ConnectionController *connectionController;
    HierarchyController *hierarchyController;
    StartMenu *startMenu;

    // State Machine components
    QGraphicsScene *m_stateMachineScene;
    StateMachineController *m_stateMachineController;
    DiagramViewController *m_diagramViewController;

    // Navigation UI
    QToolButton *m_backButton;
    QLabel *m_viewTitleLabel;
    QWidget *m_navigationBar;

    // Menu bar members
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QAction *m_newAction;
};

#endif // MAINWINDOW_H