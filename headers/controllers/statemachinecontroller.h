#ifndef STATEMACHINECONTROLLER_H
#define STATEMACHINECONTROLLER_H

#include <QObject>
#include <QList>
#include <QHash>

class QGraphicsScene;
class StateMachineModel;
class StateModel;
class StateView;
class TransitionModel;
class TransitionView;

class StateMachineController : public QObject
{
    Q_OBJECT

public:
    explicit StateMachineController(QGraphicsScene *scene, QObject *parent = nullptr);
    ~StateMachineController();

    // Set the current state machine to display
    void setStateMachine(StateMachineModel *stateMachine);
    StateMachineModel *currentStateMachine() const { return m_stateMachine; }

    // State management
    void createState(const QString &label, const QPointF &position);
    void createInitialState(const QPointF &position);
    void createFinalState(const QPointF &position);
    void deleteState(StateModel *state);

    // Transition management
    void createTransition(StateModel *startState, StateModel *endState, const QString &label = QString());
    void deleteTransition(TransitionModel *transition);

    // View registration
    void registerStateView(StateView *view);
    void unregisterStateView(StateView *view);

    // Clear the scene
    void clearScene();

public slots:
    // Handle drops from the menu (creates states instead of blocks)
    void handleDrop(const QString &itemType, const QPointF &position);

signals:
    void stateCreated(StateModel *state, StateView *view);
    void transitionCreated(TransitionModel *transition, TransitionView *view);

private slots:
    void onStateAdded(StateModel *state);
    void onStateRemoved(StateModel *state);
    void onTransitionAdded(TransitionModel *transition);
    void onTransitionRemoved(TransitionModel *transition);
    void onTransitionStarted(StateView *startState);
    void onTransitionCompleted(StateView *startState, StateView *endState);
    void onStateMachineCleared();

private:
    QGraphicsScene *m_scene;
    StateMachineModel *m_stateMachine = nullptr;
    QList<StateView *> m_stateViews;
    QHash<StateModel *, StateView *> m_stateViewMap;
    QHash<TransitionModel *, TransitionView *> m_transitionViewMap;

    StateView *createStateView(StateModel *state);
    TransitionView *createTransitionView(TransitionModel *transition);
    void loadStateMachineToScene();
};

#endif // STATEMACHINECONTROLLER_H
