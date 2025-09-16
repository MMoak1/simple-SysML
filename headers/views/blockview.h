#ifndef BLOCKVIEW_H
#define BLOCKVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include <QString>
#include "../models/blockmodel.h"

class BlockView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit BlockView(BlockModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private slots:
    void updateColor(const QColor &color);
    void updateLabel(const QString &label);
    void updatePosition(const QPointF &position);

private:
    BlockModel *m_model;
    QString m_label;
    QColor m_color;
    static const QRectF rect;
};

#endif // BLOCKVIEW_H