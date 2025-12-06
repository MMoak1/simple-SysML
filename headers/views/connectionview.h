#ifndef CONNECTIONVIEW_H
#define CONNECTIONVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include <QPainterPath>
#include "../models/connectionmodel.h"

class ConnectionView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit ConnectionView(ConnectionModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;  // For click detection on thin lines
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    ConnectionModel *model() const { return m_model; }

private slots:
    void updateConnection();

private:
    ConnectionModel *m_model;
    QColor m_color;
    
    // Cached path for shape() calculation
    mutable QPainterPath m_cachedPath;

    QPainterPath calculateOrthogonalPath(const QPointF &start, const QPointF &end) const;
    void drawArrowHead(QPainter *painter, const QPainterPath &path, const QPointF &endPoint) const;
    qreal calculateLastSegmentAngle(const QPainterPath &path) const;
};

#endif // CONNECTIONVIEW_H