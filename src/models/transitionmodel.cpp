#include "../../headers/models/transitionmodel.h"
#include "../../headers/models/statemodel.h"
#include <QtMath>

TransitionModel::TransitionModel(StateModel *startState, 
                                 StateModel *endState, 
                                 const QString &label,
                                 QObject *parent)
    : QObject(parent),
      m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_label(label)
{
    setStartState(startState);
    setEndState(endState);
}

void TransitionModel::setLabel(const QString &label)
{
    if (m_label != label)
    {
        m_label = label;
        emit labelChanged(m_label);
    }
}

void TransitionModel::setStartState(StateModel *state)
{
    if (m_startState != state)
    {
        // Disconnect old state signals if necessary
        if (m_startState)
        {
            disconnect(m_startState, nullptr, this, nullptr);
        }
        
        m_startState = state;
        m_startStateId = state ? state->id() : QString();
        
        if (m_startState)
        {
            connect(m_startState, &StateModel::positionChanged, 
                    this, &TransitionModel::updateEdgePoints);
            connect(m_startState, &StateModel::sizeChanged, 
                    this, &TransitionModel::updateEdgePoints);
        }
        
        updateEdgePoints();
        emit startStateChanged();
    }
}

void TransitionModel::setEndState(StateModel *state)
{
    if (m_endState != state)
    {
        // Disconnect old state signals if necessary
        if (m_endState)
        {
            disconnect(m_endState, nullptr, this, nullptr);
        }
        
        m_endState = state;
        m_endStateId = state ? state->id() : QString();
        
        if (m_endState)
        {
            connect(m_endState, &StateModel::positionChanged, 
                    this, &TransitionModel::updateEdgePoints);
            connect(m_endState, &StateModel::sizeChanged, 
                    this, &TransitionModel::updateEdgePoints);
        }
        
        updateEdgePoints();
        emit endStateChanged();
    }
}

void TransitionModel::updateEdgePoints()
{
    if (m_startState && m_endState)
    {
        QPointF startCenter = m_startState->position();
        QPointF endCenter = m_endState->position();
        
        m_startEdgePoint = calculateEdgePoint(m_startState, endCenter);
        m_endEdgePoint = calculateEdgePoint(m_endState, startCenter);
        
        emit transitionChanged();
    }
}

QPointF TransitionModel::calculateEdgePoint(StateModel *state, const QPointF &targetCenter) const
{
    if (!state)
        return QPointF();
    
    QPointF center = state->position();
    QSizeF size = state->size();
    
    // Calculate direction from center to target
    qreal dx = targetCenter.x() - center.x();
    qreal dy = targetCenter.y() - center.y();
    
    if (qAbs(dx) < 0.001 && qAbs(dy) < 0.001)
        return center;
    
    qreal halfWidth = size.width() / 2.0;
    qreal halfHeight = size.height() / 2.0;
    
    // Calculate intersection with rectangle edge
    if (qAbs(dx) > qAbs(dy))
    {
        // Intersect with left or right edge
        if (dx > 0)
        {
            return QPointF(center.x() + halfWidth, center.y() + dy * halfWidth / dx);
        }
        else
        {
            return QPointF(center.x() - halfWidth, center.y() - dy * halfWidth / dx);
        }
    }
    else
    {
        // Intersect with top or bottom edge
        if (dy > 0)
        {
            return QPointF(center.x() + dx * halfHeight / dy, center.y() + halfHeight);
        }
        else
        {
            return QPointF(center.x() - dx * halfHeight / dy, center.y() - halfHeight);
        }
    }
}
