#ifndef DROPGRAPHICSVIEW_H
#define DROPGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "blockitem.h"

class DropGraphicsView : public QGraphicsView
{
public:
    DropGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);
    virtual ~DropGraphicsView();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QColor getColorFromText(const QString &text);
};

#endif // DROPGRAPHICSVIEW_H