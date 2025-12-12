#include "../headers/views/dropgraphicsview.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QDataStream>
#include <QIODevice>

DropGraphicsView::DropGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent), m_scene(scene)
{
    setAcceptDrops(true);
    
    // Enable rubber band selection for multi-select
    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::IntersectsItemShape);
}

void DropGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain") || 
        event->mimeData()->hasFormat("application/x-sysml-blockdefinition"))
    {
        event->acceptProposedAction();
    }
}

void DropGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain") || 
        event->mimeData()->hasFormat("application/x-sysml-blockdefinition"))
    {
        event->acceptProposedAction();
    }
}

void DropGraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        QString text = event->mimeData()->text();
        QPointF scenePos = mapToScene(event->position().toPoint());

        emit dropPerformed(text, scenePos);
        event->acceptProposedAction();
    }
    else if (event->mimeData()->hasFormat("application/x-sysml-blockdefinition"))
    {
        QByteArray encoded = event->mimeData()->data("application/x-sysml-blockdefinition");
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        QString id;
        stream >> id;
        
        QPointF scenePos = mapToScene(event->position().toPoint());
        emit blockDefinitionDropped(id, scenePos);
        event->acceptProposedAction();
    }
}

void DropGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // Solution A: Toggle drag mode based on what we're clicking on
    QGraphicsItem *item = itemAt(event->pos());
    
    if (item && item->isSelected())
    {
        // Clicking on an already-selected item - disable rubber band for dragging
        setDragMode(QGraphicsView::NoDrag);
    }
    else if (item && (item->flags() & QGraphicsItem::ItemIsSelectable))
    {
        // Clicking on a selectable but unselected item - also disable rubber band
        // (user is clicking to select this item, not starting a rubber band)
        setDragMode(QGraphicsView::NoDrag);
    }
    else
    {
        // Clicking on empty space - enable rubber band selection
        setDragMode(QGraphicsView::RubberBandDrag);
    }
    
    QGraphicsView::mousePressEvent(event);
}

void DropGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    
    // Always restore rubber band mode after mouse release
    setDragMode(QGraphicsView::RubberBandDrag);
}

DropGraphicsView::~DropGraphicsView()
{
}
