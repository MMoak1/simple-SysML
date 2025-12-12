#include "../../headers/models/partproperty.h"
#include <QUuid>

PartProperty::PartProperty(const QString &name, BlockDefinition *type,
                           const QString &multiplicity, QObject *parent)
    : QObject(parent), m_name(name), m_type(type), m_multiplicity(multiplicity)
{
    m_id = QUuid::createUuid().toString();
    
    if (m_type)
    {
        connect(m_type.data(), &BlockDefinition::typeNameChanged, this, [this](const QString &) {
            emit typeChanged(m_type.data());
        });
    }
}

PartProperty::~PartProperty()
{
    if (m_stateMachineInstance)
    {
        delete m_stateMachineInstance;
    }
}

void PartProperty::setId(const QString &id)
{
    m_id = id;
}

void PartProperty::setName(const QString &name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged(name);
    }
}

void PartProperty::setMultiplicity(const QString &mult)
{
    if (m_multiplicity != mult)
    {
        m_multiplicity = mult;
        emit multiplicityChanged(mult);
    }
}

void PartProperty::setType(BlockDefinition *type)
{
    if (m_type != type)
    {
        if (m_type)
        {
            disconnect(m_type, &BlockDefinition::typeNameChanged, this, nullptr);
        }
        
        m_type = type;
        
        if (m_type)
        {
            connect(m_type.data(), &BlockDefinition::typeNameChanged, this, [this](const QString &) {
                emit typeChanged(m_type.data());
            });
        }
        
        emit typeChanged(type);
    }
}

void PartProperty::setOwner(BlockDefinition *owner)
{
    m_owner = owner;
    setParent(owner);
}

QString PartProperty::displayText() const
{
    QString typeStr = m_type ? m_type->typeName() : "Unknown";
    QString result;
    
    if (!m_multiplicity.isEmpty())
    {
        result += "[" + m_multiplicity + "] ";
    }
    
    if (!m_name.isEmpty())
    {
        result += m_name + " : ";
    }
    
    result += typeStr;
    return result;
}

StateMachineModel* PartProperty::getOrCreateStateMachineInstance()
{
    if (!m_stateMachineInstance && m_type && m_type->hasStateMachine())
    {
        // Clone the type's state machine template
        // Note: clone() needs to be implemented in StateMachineModel
        m_stateMachineInstance = m_type->stateMachine()->clone();
        m_stateMachineInstance->setParent(this);
        emit stateMachineInstanceChanged();
    }
    return m_stateMachineInstance;
}
