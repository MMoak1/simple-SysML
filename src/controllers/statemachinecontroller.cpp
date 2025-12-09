#include "../../headers/controllers/statemachinecontroller.h"
#include "../../headers/models/statemachinemodel.h"
#include "../../headers/models/statemodel.h"
#include "../../headers/models/transitionmodel.h"
#include "../../headers/views/stateview.h"
#include "../../headers/views/transitionview.h"
#include <QGraphicsScene>
#include <QDebug>

StateMachineController::StateMachineController(QGraphicsScene *scene, QObject *parent)
    : QObject(parent),
      m_scene(scene)
{
}

StateMachineController::~StateMachineController()
{
    clearScene();
}

void StateMachineController::setStateMachine(StateMachineModel *stateMachine)
{
    // Disconnect from old state machine
    if (m_stateMachine)
    {
        disconnect(m_stateMachine, nullptr, this, nullptr);
    }
    
    // Clear old views
    clearScene();
    
    m_stateMachine = stateMachine;
    
    if (m_stateMachine)
    {
        // Connect to new state machine signals
        connect(m_stateMachine, &StateMachineModel::stateAdded, 
                this, &StateMachineController::onStateAdded);
        connect(m_stateMachine, &StateMachineModel::stateRemoved, 
                this, &StateMachineController::onStateRemoved);
        connect(m_stateMachine, &StateMachineModel::transitionAdded, 
                this, &StateMachineController::onTransitionAdded);
        connect(m_stateMachine, &StateMachineModel::transitionRemoved, 
                this, &StateMachineController::onTransitionRemoved);
        connect(m_stateMachine, &StateMachineModel::cleared, 
                this, &StateMachineController::onStateMachineCleared);
        
        // Load existing content
        loadStateMachineToScene();
    }
    
    qDebug() << "StateMachineController: Set state machine" 
             << (stateMachine ? "with content" : "to null");
}

void StateMachineController::createState(const QString &label, const QPointF &position)
{
    if (!m_stateMachine)
        return;
    
    StateModel *state = new StateModel(label, StateType::Normal, position);
    m_stateMachine->addState(state);
    
    qDebug() << "Created normal state:" << label;
}

void StateMachineController::createInitialState(const QPointF &position)
{
    if (!m_stateMachine)
        return;
    
    StateModel *state = new StateModel("", StateType::Initial, position, QSizeF(20, 20));
    m_stateMachine->addState(state);
    
    qDebug() << "Created initial state";
}

void StateMachineController::createFinalState(const QPointF &position)
{
    if (!m_stateMachine)
        return;
    
    StateModel *state = new StateModel("", StateType::Final, position, QSizeF(30, 30));
    m_stateMachine->addState(state);
    
    qDebug() << "Created final state";
}

void StateMachineController::deleteState(StateModel *state)
{
    if (!m_stateMachine || !state)
        return;
    
    m_stateMachine->removeState(state);
}

void StateMachineController::createTransition(StateModel *startState, StateModel *endState, const QString &label)
{
    if (!m_stateMachine || !startState || !endState)
        return;
    
    TransitionModel *transition = new TransitionModel(startState, endState, label);
    m_stateMachine->addTransition(transition);
    
    qDebug() << "Created transition from" << startState->label() << "to" << endState->label();
}

void StateMachineController::deleteTransition(TransitionModel *transition)
{
    if (!m_stateMachine || !transition)
        return;
    
    m_stateMachine->removeTransition(transition);
}

void StateMachineController::registerStateView(StateView *view)
{
    if (view && !m_stateViews.contains(view))
    {
        m_stateViews.append(view);
        
        // Connect transition signals
        connect(view, &StateView::transitionStarted, 
                this, &StateMachineController::onTransitionStarted);
        connect(view, &StateView::transitionCompleted, 
                this, &StateMachineController::onTransitionCompleted);
        
        // Track when view is destroyed
        connect(view, &QObject::destroyed, this, [this, view]() {
            m_stateViews.removeOne(view);
        });
    }
}

void StateMachineController::unregisterStateView(StateView *view)
{
    if (view)
    {
        disconnect(view, nullptr, this, nullptr);
        m_stateViews.removeOne(view);
    }
}

void StateMachineController::clearScene()
{
    // Clear all tracking maps
    m_stateViewMap.clear();
    m_transitionViewMap.clear();
    m_stateViews.clear();
    
    // Clear the scene
    if (m_scene)
    {
        m_scene->clear();
    }
}

void StateMachineController::handleDrop(const QString &itemType, const QPointF &position)
{
    if (!m_stateMachine)
    {
        qDebug() << "StateMachineController: No state machine set, cannot handle drop";
        return;
    }
    
    qDebug() << "StateMachineController: Handling drop of" << itemType << "at" << position;
    
    // Map block types to state types
    // For now, create normal states from any block drop
    // In the future, could have a dedicated state menu
    QString stateName = itemType;
    stateName.replace(" Block", " State"); // "Red Block" -> "Red State"
    
    createState(stateName, position);
}

void StateMachineController::onStateAdded(StateModel *state)
{
    if (!state)
        return;
    
    StateView *view = createStateView(state);
    if (view)
    {
        emit stateCreated(state, view);
    }
}

void StateMachineController::onStateRemoved(StateModel *state)
{
    if (!state)
        return;
    
    StateView *view = m_stateViewMap.take(state);
    if (view)
    {
        unregisterStateView(view);
        m_scene->removeItem(view);
        view->deleteLater();  // SAFE: defers to event loop
    }
}

void StateMachineController::onTransitionAdded(TransitionModel *transition)
{
    if (!transition)
        return;
    
    TransitionView *view = createTransitionView(transition);
    if (view)
    {
        emit transitionCreated(transition, view);
    }
}

void StateMachineController::onTransitionRemoved(TransitionModel *transition)
{
    if (!transition)
        return;
    
    TransitionView *view = m_transitionViewMap.take(transition);
    if (view)
    {
        m_scene->removeItem(view);
        view->deleteLater();  // SAFE: defers to event loop
    }
}

void StateMachineController::onTransitionStarted(StateView *startState)
{
    Q_UNUSED(startState);
    // Could add visual feedback here if needed
}

void StateMachineController::onTransitionCompleted(StateView *startState, StateView *endState)
{
    if (!startState || !endState)
        return;
    
    createTransition(startState->model(), endState->model());
}

void StateMachineController::onStateMachineCleared()
{
    clearScene();
}

StateView *StateMachineController::createStateView(StateModel *state)
{
    if (!state || !m_scene)
        return nullptr;
    
    StateView *view = new StateView(state);
    m_scene->addItem(view);
    
    m_stateViewMap[state] = view;
    registerStateView(view);
    
    return view;
}

TransitionView *StateMachineController::createTransitionView(TransitionModel *transition)
{
    if (!transition || !m_scene)
        return nullptr;
    
    TransitionView *view = new TransitionView(transition);
    m_scene->addItem(view);
    
    m_transitionViewMap[transition] = view;
    
    return view;
}

void StateMachineController::loadStateMachineToScene()
{
    if (!m_stateMachine)
        return;
    
    // First create all state views
    for (StateModel *state : m_stateMachine->states())
    {
        createStateView(state);
    }
    
    // Then create all transition views
    for (TransitionModel *transition : m_stateMachine->transitions())
    {
        createTransitionView(transition);
    }
    
    qDebug() << "Loaded state machine with" << m_stateMachine->states().size() << "states and"
             << m_stateMachine->transitions().size() << "transitions";
}
