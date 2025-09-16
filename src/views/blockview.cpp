#include "../headers/views/blockview.h"
#include <QPainter>
#include <QBrush>

const QRectF BlockView::rect = QRectF(-50, -30, 100, 60);

BlockView::BlockView(BlockModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model)
{
    // Set initial properties from model
    m_color = model->color();
    m_label = model->label();

    // Make selectable and movable
    setFlags(ItemIsSelectable | ItemIsMovable);

    // Connect to model signals
    connect(model, &BlockModel::colorChanged, this, &BlockView::updateColor);
    connect(model, &BlockModel::labelChanged, this, &BlockView::updateLabel);
    connect(model, &BlockModel::positionChanged, this, &BlockView::updatePosition);

    // Set initial position
    setPos(model->position());
}

QRectF BlockView::boundingRect() const
{
    return rect;
}

void BlockView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw the rectangle background
    painter->fillRect(rect, QBrush(m_color));

    // Draw the label text centered
    painter->save();
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignCenter, m_label);
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