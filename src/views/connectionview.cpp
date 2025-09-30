#include "../../headers/views/connectionview.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QtMath>

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

    QPointF start = m_model->startEdgePoint();
    QPointF end = m_model->endEdgePoint();

    // Calculate bounding rect for orthogonal path with padding for arrow
    qreal padding = 15.0;
    qreal midX = (start.x() + end.x()) / 2.0;

    qreal left = qMin(qMin(start.x(), end.x()), midX) - padding;
    qreal top = qMin(start.y(), end.y()) - padding;
    qreal right = qMax(qMax(start.x(), end.x()), midX) + padding;
    qreal bottom = qMax(start.y(), end.y()) + padding;

    return QRectF(QPointF(left, top), QPointF(right, bottom));
}

void ConnectionView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_model || !m_model->isValid())
    {
        return;
    }

    QPointF start = m_model->startEdgePoint();
    QPointF end = m_model->endEdgePoint();

    // Calculate orthogonal path
    QPainterPath path = calculateOrthogonalPath(start, end);

    // Draw the connection line
    painter->save();
    QPen pen(m_color, 2.0);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path);

    // Draw arrow head at end point
    drawArrowHead(painter, path, end);

    painter->restore();
}

void ConnectionView::updateConnection()
{
    prepareGeometryChange();
    update();
}

QPainterPath ConnectionView::calculateOrthogonalPath(const QPointF &start, const QPointF &end) const
{
    QPainterPath path;
    path.moveTo(start);

    // Simple 3-segment orthogonal routing
    // Start -> Mid1 -> Mid2 -> End

    qreal midX = (start.x() + end.x()) / 2.0;

    // First segment: horizontal from start
    path.lineTo(midX, start.y());

    // Second segment: vertical to end height
    path.lineTo(midX, end.y());

    // Third segment: horizontal to end
    path.lineTo(end);

    return path;
}

void ConnectionView::drawArrowHead(QPainter *painter, const QPainterPath &path, const QPointF &endPoint) const
{
    // Get direction of last segment
    qreal angle = calculateLastSegmentAngle(path);

    // Arrow dimensions
    qreal arrowSize = 10.0;

    // Calculate arrow points
    QPointF arrowP1 = endPoint + QPointF(
                                     sin(angle + M_PI / 3) * arrowSize,
                                     cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = endPoint + QPointF(
                                     sin(angle + M_PI - M_PI / 3) * arrowSize,
                                     cos(angle + M_PI - M_PI / 3) * arrowSize);

    // Draw filled arrow head
    QPolygonF arrowHead;
    arrowHead << endPoint << arrowP1 << arrowP2;
    painter->setBrush(m_color);
    painter->drawPolygon(arrowHead);
}

qreal ConnectionView::calculateLastSegmentAngle(const QPainterPath &path) const
{
    // Get the last two points to determine direction
    int elementCount = path.elementCount();
    if (elementCount < 2)
    {
        return 0.0;
    }

    QPainterPath::Element lastElement = path.elementAt(elementCount - 1);
    QPainterPath::Element secondLastElement = path.elementAt(elementCount - 2);

    QPointF lastPoint(lastElement.x, lastElement.y);
    QPointF secondLastPoint(secondLastElement.x, secondLastElement.y);

    // Calculate angle from second-last to last point
    qreal dx = lastPoint.x() - secondLastPoint.x();
    qreal dy = lastPoint.y() - secondLastPoint.y();

    return atan2(dy, dx);
}