#include "../../headers/models/statemodel.h"

StateModel::StateModel(const QString &label, 
                       StateType type,
                       const QPointF &position, 
                       const QSizeF &size, 
                       QObject *parent)
    : QObject(parent),
      m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_label(label),
      m_stateType(type),
      m_position(position),
      m_size(size)
{
}

void StateModel::setLabel(const QString &label)
{
    if (m_label != label)
    {
        m_label = label;
        emit labelChanged(m_label);
    }
}

void StateModel::setStateType(StateType type)
{
    if (m_stateType != type)
    {
        m_stateType = type;
        emit stateTypeChanged(m_stateType);
    }
}

void StateModel::setPosition(const QPointF &position)
{
    if (m_position != position)
    {
        m_position = position;
        emit positionChanged(m_position);
    }
}

void StateModel::setSize(const QSizeF &size)
{
    if (m_size != size)
    {
        m_size = size;
        emit sizeChanged(m_size);
    }
}
