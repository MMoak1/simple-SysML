#include "../../headers/models/statemachinemodel.h"
#include "../../headers/models/blockmodel.h"
#include "../../headers/models/statemodel.h"
#include "../../headers/models/transitionmodel.h"

StateMachineModel::StateMachineModel(QObject *parent)
    : QObject(parent),
      m_parentBlock(nullptr)
{
}

StateMachineModel::StateMachineModel(BlockModel *parentBlock, QObject *parent)
    : QObject(parent),
      m_parentBlock(parentBlock)
{
}

StateMachineModel::~StateMachineModel()
{
    clear();
}

StateMachineModel *StateMachineModel::clone() const
{
    StateMachineModel *newSM = new StateMachineModel();
    
    // 1. Clone states
    QList<StateModel *> newStates;
    for (StateModel *state : m_states)
    {
        StateModel *newState = state->clone();
        newSM->addState(newState);
        newStates.append(newState);
    }
    
    // 2. Clone transitions
    for (TransitionModel *transition : m_transitions)
    {
        TransitionModel *newTransition = transition->clone(newStates);
        if (newTransition)
        {
            newSM->addTransition(newTransition);
        }
    }
    
    return newSM;
}

void StateMachineModel::addState(StateModel *state)
{
    if (state && !m_states.contains(state))
    {
        m_states.append(state);
        state->setParent(this);
        emit stateAdded(state);
    }
}

void StateMachineModel::removeState(StateModel *state)
{
    if (state && m_states.contains(state))
    {
        // First remove any transitions involving this state
        QList<TransitionModel *> transitionsToRemove;
        for (TransitionModel *transition : m_transitions)
        {
            if (transition->startState() == state || transition->endState() == state)
            {
                transitionsToRemove.append(transition);
            }
        }
        for (TransitionModel *transition : transitionsToRemove)
        {
            removeTransition(transition);
        }
        
        m_states.removeOne(state);
        emit stateRemoved(state);
        delete state;
    }
}

StateModel *StateMachineModel::findStateById(const QString &id) const
{
    for (StateModel *state : m_states)
    {
        if (state->id() == id)
        {
            return state;
        }
    }
    return nullptr;
}

void StateMachineModel::addTransition(TransitionModel *transition)
{
    if (transition && !m_transitions.contains(transition))
    {
        m_transitions.append(transition);
        transition->setParent(this);
        emit transitionAdded(transition);
    }
}

void StateMachineModel::removeTransition(TransitionModel *transition)
{
    if (transition && m_transitions.contains(transition))
    {
        m_transitions.removeOne(transition);
        emit transitionRemoved(transition);
        delete transition;
    }
}

TransitionModel *StateMachineModel::findTransitionById(const QString &id) const
{
    for (TransitionModel *transition : m_transitions)
    {
        if (transition->id() == id)
        {
            return transition;
        }
    }
    return nullptr;
}

void StateMachineModel::clear()
{
    // Remove all transitions first
    while (!m_transitions.isEmpty())
    {
        TransitionModel *transition = m_transitions.takeLast();
        emit transitionRemoved(transition);
        delete transition;
    }
    
    // Then remove all states
    while (!m_states.isEmpty())
    {
        StateModel *state = m_states.takeLast();
        emit stateRemoved(state);
        delete state;
    }
    
    emit cleared();
}
