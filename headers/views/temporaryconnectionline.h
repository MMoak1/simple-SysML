#ifndef TEMPORARYCONNECTIONLINE_H
#define TEMPORARYCONNECTIONLINE_H

#include <QGraphicsLineItem>
#include <QPointF>
#include <QPen>

class TemporaryConnectionLine : public QGraphicsLineItem
{
public:
    explicit TemporaryConnectionLine(const QPointF &startPoint, QGraphicsItem *parent = nullptr);

    void updateEndPoint(const QPointF &endPoint);
    void setStartPoint(const QPointF &startPoint);

private:
    QPointF m_startPoint;
    QPointF m_endPoint;
};

#endif // TEMPORARYCONNECTIONLINE_H