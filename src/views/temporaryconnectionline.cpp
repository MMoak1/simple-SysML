#include "../../headers/views/temporaryconnectionline.h"
#include <QPen>
#include <QBrush>

TemporaryConnectionLine::TemporaryConnectionLine(const QPointF &startPoint, QGraphicsItem *parent)
    : QGraphicsLineItem(parent), m_startPoint(startPoint), m_endPoint(startPoint)
{
    // Set up the pen for a dashed temporary line
    QPen pen(Qt::gray, 2.0, Qt::DashLine);
    setPen(pen);

    // Set initial line
    setLine(QLineF(m_startPoint, m_endPoint));

    // Make sure it's drawn on top of other items
    setZValue(1000);
}

void TemporaryConnectionLine::updateEndPoint(const QPointF &endPoint)
{
    m_endPoint = endPoint;
    setLine(QLineF(m_startPoint, m_endPoint));
}

void TemporaryConnectionLine::setStartPoint(const QPointF &startPoint)
{
    m_startPoint = startPoint;
    setLine(QLineF(m_startPoint, m_endPoint));
}