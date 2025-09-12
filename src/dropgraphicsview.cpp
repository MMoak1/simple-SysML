#include "../headers/dropgraphicsview.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>

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
    QColor color = getColorFromText(text);

    if (color.isValid())
    {
        QPointF scenePos = mapToScene(event->position().toPoint());
        BlockItem *item = new BlockItem(color, text, nullptr);
        item->setPos(scenePos);
        m_scene->addItem(item);
        event->acceptProposedAction();
    }
}

QColor DropGraphicsView::getColorFromText(const QString &text)
{
    if (text == "Red Block")
    {
        return Qt::red;
    }
    else if (text == "Blue Block")
    {
        return Qt::blue;
    }
    else if (text == "Green Block")
    {
        return Qt::green;
    }
    return QColor();
}

DropGraphicsView::~DropGraphicsView()
{
}
