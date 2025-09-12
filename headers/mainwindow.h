#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSplitter>

#include "blockmenu.h"
#include "dropgraphicsview.h"

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private:
    QSplitter *splitter;
    BlockMenu *blockMenu;
    DropGraphicsView *view;
    QGraphicsScene *scene;
};

#endif // MAINWINDOW_H