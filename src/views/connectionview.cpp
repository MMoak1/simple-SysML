#include "../../headers/views/connectionview.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

ConnectionView::ConnectionView(ConnectionModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model), m_color(Qt::black)
{
    connect(model, &ConnectionModel::connectionChanged, this, &ConnectionView::updateConnection);
}

QRectF ConnectionView::boundingRect() const
{
    if (!m_model || !m_model->isValid())
    {
        return QRectF();
    }

    QPointF start = m_model->startPoint();
    QPointF end = m_model->endPoint();

    // Calculate bounding rect with some padding for the line width
    qreal padding = 5.0;
    qreal left = qMin(start.x(), end.x()) - padding;
    qreal top = qMin(start.y(), end.y()) - padding;
    qreal width = qAbs(end.x() - start.x()) + 2 * padding;
    qreal height = qAbs(end.y() - start.y()) + 2 * padding;

    return QRectF(left, top, width, height);
}

void ConnectionView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!m_model || !m_model->isValid())
    {
        return;
    }

    QPointF start = m_model->startPoint();
    QPointF end = m_model->endPoint();

    // Draw the connection line
    painter->save();
    QPen pen(m_color, 2.0);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawLine(start, end);
    painter->restore();
}

void ConnectionView::updateConnection()
{
    prepareGeometryChange();
    update();
}