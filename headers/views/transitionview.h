#ifndef TRANSITIONVIEW_H
#define TRANSITIONVIEW_H

#include <QGraphicsObject>
#include <QPointF>

class TransitionModel;

class TransitionView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit TransitionView(TransitionModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    TransitionModel *model() const { return m_model; }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void onTransitionChanged();
    void onLabelChanged(const QString &label);

private:
    TransitionModel *m_model;
    QString m_label;
    QPointF m_startPoint;
    QPointF m_endPoint;

    void updateFromModel();
    void drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to);
    void showLabelInputDialog();
};

#endif // TRANSITIONVIEW_H
