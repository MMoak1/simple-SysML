#include "../headers/views/blockview.h"
#include <QPainter>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QCursor>

BlockView::BlockView(BlockModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model)
{
    // Set initial properties from model
    m_color = model->color();
    m_label = model->label();
    m_size = model->size();

    // Make selectable and movable
    setFlags(ItemIsSelectable | ItemIsMovable);

    // Connect to model signals
    connect(model, &BlockModel::colorChanged, this, &BlockView::updateColor);
    connect(model, &BlockModel::labelChanged, this, &BlockView::updateLabel);
    connect(model, &BlockModel::positionChanged, this, &BlockView::updatePosition);
    connect(model, &BlockModel::sizeChanged, this, &BlockView::updateSize);

    // Set initial position
    setPos(model->position());
}

QRectF BlockView::boundingRect() const
{
    return QRectF(-m_size.width() / 2, -m_size.height() / 2, m_size.width(), m_size.height());
}

void BlockView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF localRect(-m_size.width() / 2, -m_size.height() / 2, m_size.width(), m_size.height());
    if (m_resizing)
    {
        // Draw resize border
        painter->save();
        QPen borderPen(Qt::black, 3);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(localRect);
        painter->restore();
    }
    // Draw the rectangle background
    painter->fillRect(localRect, QBrush(m_color));

    // Draw the label text centered
    painter->save();
    painter->setPen(Qt::black);
    painter->drawText(localRect, Qt::AlignCenter, m_label);
    painter->restore();

    // Draw resize handle - black dot
    qreal handleSize = 10.0;
    qreal radius = handleSize / 2;
    QPointF handleCenter(localRect.right() - radius, localRect.bottom() - radius);
    painter->save();
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(handleCenter, radius, radius);
    painter->restore();
}

void BlockView::updateColor(const QColor &color)
{
    m_color = color;
    update(); // Trigger repaint
}

void BlockView::updateLabel(const QString &label)
{
    m_label = label;
    update(); // Trigger repaint
}

void BlockView::updatePosition(const QPointF &position)
{
    setPos(position);
}

void BlockView::updateSize(const QSizeF &size)
{
    prepareGeometryChange();
    m_size = size;
    update();
}

void BlockView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF localPos = mapFromScene(event->scenePos());
        qreal handleSize = 10.0;
        QRectF bounding = boundingRect();
        if (localPos.x() > bounding.width() / 2 - handleSize &&
            localPos.y() > bounding.height() / 2 - handleSize)
        {
            m_resizing = true;
            m_originalSize = m_size;
            m_resizeStartPos = event->scenePos();
            event->accept();
            return;
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

void BlockView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        QPointF delta = event->scenePos() - m_resizeStartPos;
        QSizeF newSize = m_originalSize + QSizeF(delta.x(), delta.y());
        if (newSize.width() > 20 && newSize.height() > 20)
        {
            m_model->setSize(newSize);
        }
        event->accept();
        return;
    }

    // Check for resize cursor
    QPointF localPos = mapFromScene(event->scenePos());
    QRectF bounding = boundingRect();
    qreal handleSize = 10.0;
    if (localPos.x() > bounding.width() / 2 - handleSize &&
        localPos.y() > bounding.height() / 2 - handleSize)
    {
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else
    {
        unsetCursor();
    }
    QGraphicsObject::mouseMoveEvent(event);
}

void BlockView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        m_resizing = false;
        event->accept();
        return;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}