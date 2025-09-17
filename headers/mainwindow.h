#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSplitter>
#include <QGraphicsView>

#include "views/blockmenuview.h"
#include "views/dropgraphicsview.h"
#include "views/startmenu.h"
#include "models/blockmodel.h"
#include "controllers/dropcontroller.h"
#include "controllers/menucontroller.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();

private:
    void setupToolInterface();

    QGraphicsScene *scene;
    QSplitter *splitter;
    BlockMenuView *blockMenuView;
    DropGraphicsView *dropGraphicsView;
    DropController *dropController;
    MenuController *menuController;
    StartMenu *startMenu;

    // Menu bar members
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QAction *m_newAction;
};

#endif // MAINWINDOW_H