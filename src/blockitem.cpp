#include "../headers/blockitem.h"
#include <QPainter>

BlockItem::BlockItem(const QColor &color, const QString &label, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), m_color(color), m_label(label)
{
    // Set rectangle size
    setRect(-50, -30, 100, 60);

    // Set brush for filling
    setBrush(QBrush(color));

    // Make selectable and movable
    setFlags(ItemIsSelectable | ItemIsMovable);
}

void BlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw the rectangle background
    QGraphicsRectItem::paint(painter, option, widget);

    // Draw the label text centered
    painter->save();
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, m_label);
    painter->restore();
}