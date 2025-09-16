#include "../headers/views/dropgraphicsview.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

DropGraphicsView::DropGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent), m_scene(scene)
{
    setAcceptDrops(true);
}

void DropGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}

void DropGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}

void DropGraphicsView::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/plain"))
    {
        return;
    }

    QString text = event->mimeData()->text();
    QPointF scenePos = mapToScene(event->position().toPoint());

    emit dropPerformed(text, scenePos);
    event->acceptProposedAction();
}

DropGraphicsView::~DropGraphicsView()
{
}
