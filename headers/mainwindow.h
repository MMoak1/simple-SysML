#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSplitter>
#include <QGraphicsView>

#include "views/blockmenuview.h"
#include "views/dropgraphicsview.h"
#include "models/blockmodel.h"
#include "controllers/dropcontroller.h"
#include "controllers/menucontroller.h"

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QGraphicsScene *scene;
    QSplitter *splitter;
    BlockMenuView *blockMenuView;
    DropGraphicsView *dropGraphicsView;
    DropController *dropController;
    MenuController *menuController;
};

#endif // MAINWINDOW_H