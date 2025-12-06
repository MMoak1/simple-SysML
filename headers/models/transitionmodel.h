#ifndef TRANSITIONMODEL_H
#define TRANSITIONMODEL_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QUuid>

class StateModel;

class TransitionModel : public QObject
{
    Q_OBJECT

public:
    explicit TransitionModel(StateModel *startState, 
                             StateModel *endState, 
                             const QString &label = QString(),
                             QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }
    QString startStateId() const { return m_startStateId; }
    QString endStateId() const { return m_endStateId; }
    StateModel *startState() const { return m_startState; }
    StateModel *endState() const { return m_endState; }
    QString label() const { return m_label; }

    // Setters
    void setLabel(const QString &label);
    void setStartState(StateModel *state);
    void setEndState(StateModel *state);

    // Validation
    bool isValid() const { return !m_startStateId.isEmpty() && !m_endStateId.isEmpty(); }

    // Edge point calculation
    QPointF startEdgePoint() const { return m_startEdgePoint; }
    QPointF endEdgePoint() const { return m_endEdgePoint; }

    // Clone method for instance creation
    TransitionModel *clone(const QList<StateModel *> &newStates) const;

signals:
    void labelChanged(const QString &label);
    void transitionChanged();
    void startStateChanged();
    void endStateChanged();

private:
    void updateEdgePoints();
    void connectStateSignals();
    QPointF calculateEdgePoint(StateModel *state, const QPointF &targetCenter) const;

    QString m_id;
    QString m_startStateId;
    QString m_endStateId;
    StateModel *m_startState = nullptr;
    StateModel *m_endState = nullptr;
    QString m_label;
    QPointF m_startEdgePoint;
    QPointF m_endEdgePoint;
};

#endif // TRANSITIONMODEL_H
