#ifndef STATEMACHINEMODEL_H
#define STATEMACHINEMODEL_H

#include <QObject>
#include <QList>
#include <QString>

class BlockModel;
class StateModel;
class TransitionModel;

class StateMachineModel : public QObject
{
    Q_OBJECT

public:
    explicit StateMachineModel(BlockModel *parentBlock, QObject *parent = nullptr);
    ~StateMachineModel();

    // Parent block reference
    BlockModel *parentBlock() const { return m_parentBlock; }

    // State management
    void addState(StateModel *state);
    void removeState(StateModel *state);
    QList<StateModel *> states() const { return m_states; }
    StateModel *findStateById(const QString &id) const;

    // Transition management
    void addTransition(TransitionModel *transition);
    void removeTransition(TransitionModel *transition);
    QList<TransitionModel *> transitions() const { return m_transitions; }
    TransitionModel *findTransitionById(const QString &id) const;

    // Convenience methods
    void clear();
    bool isEmpty() const { return m_states.isEmpty(); }

signals:
    void stateAdded(StateModel *state);
    void stateRemoved(StateModel *state);
    void transitionAdded(TransitionModel *transition);
    void transitionRemoved(TransitionModel *transition);
    void cleared();

private:
    BlockModel *m_parentBlock;
    QList<StateModel *> m_states;
    QList<TransitionModel *> m_transitions;
};

#endif // STATEMACHINEMODEL_H
