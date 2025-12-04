#ifndef STATEVIEW_H
#define STATEVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include <QString>
#include <QSizeF>
#include <QPointF>

class StateModel;
class TemporaryConnectionLine;

class StateView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit StateView(StateModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    StateModel *model() const { return m_model; }

signals:
    void transitionStarted(StateView *startState);
    void transitionCompleted(StateView *startState, StateView *endState);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void updateLabel(const QString &label);
    void updatePosition(const QPointF &position);
    void updateSize(const QSizeF &size);
    void updateStateType();

private:
    StateModel *m_model;
    QString m_label;
    QSizeF m_size;

    // Resize state
    bool m_resizing = false;
    QPointF m_resizeStartPos;
    QSizeF m_originalSize;

    // Transition drawing state (static to share across all StateView instances)
    static bool s_drawingTransition;
    static StateView *s_transitionStartState;
    static TemporaryConnectionLine *s_tempLine;

    void showTitleInputDialog();
    QPointF getNearestEdgePoint(const QPointF &targetPoint) const;
    StateView *findStateAtPosition(const QPointF &scenePos) const;
};

#endif // STATEVIEW_H
