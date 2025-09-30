#ifndef CONNECTIONVIEW_H
#define CONNECTIONVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include "../models/connectionmodel.h"

class ConnectionView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit ConnectionView(ConnectionModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    ConnectionModel *model() const { return m_model; }

private slots:
    void updateConnection();

private:
    ConnectionModel *m_model;
    QColor m_color;
};

#endif // CONNECTIONVIEW_H