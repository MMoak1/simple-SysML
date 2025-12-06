#include "../../headers/models/blockdefinition.h"
#include "../../headers/models/partproperty.h"
#include <QUuid>
#include <QDebug>

BlockDefinition::BlockDefinition(const QString &typeName, const QColor &color, 
                                 const QPointF &position, QObject *parent)
    : QObject(parent), m_typeName(typeName), m_color(color), m_position(position), m_size(100, 80)
{
    m_id = QUuid::createUuid().toString();
}

BlockDefinition::~BlockDefinition()
{
    qDeleteAll(m_partProperties);
    if (m_stateMachine)
    {
        delete m_stateMachine;
    }
}

void BlockDefinition::setId(const QString &id)
{
    m_id = id;
}

void BlockDefinition::setTypeName(const QString &name)
{
    if (m_typeName != name)
    {
        m_typeName = name;
        emit typeNameChanged(name);
    }
}

void BlockDefinition::setPosition(const QPointF &pos)
{
    if (m_position != pos)
    {
        m_position = pos;
        emit positionChanged(pos);
    }
}

void BlockDefinition::setSize(const QSizeF &size)
{
    if (m_size != size)
    {
        m_size = size;
        emit sizeChanged(size);
    }
}

StateMachineModel* BlockDefinition::getOrCreateStateMachine()
{
    if (!m_stateMachine)
    {
        m_stateMachine = new StateMachineModel(this);
    }
    return m_stateMachine;
}

PartProperty* BlockDefinition::addPartProperty(const QString &name, BlockDefinition *type, 
                                               const QString &multiplicity)
{
    PartProperty *part = new PartProperty(name, type, multiplicity, this);
    part->setOwner(this);
    m_partProperties.append(part);
    
    emit partPropertyAdded(part);
    return part;
}

bool BlockDefinition::removePartProperty(PartProperty *part)
{
    if (m_partProperties.removeOne(part))
    {
        emit partPropertyRemoved(part);
        part->deleteLater();
        return true;
    }
    return false;
}

PartProperty* BlockDefinition::findPartPropertyByName(const QString &name) const
{
    for (PartProperty *part : m_partProperties)
    {
        if (part->name() == name)
        {
            return part;
        }
    }
    return nullptr;
}

bool BlockDefinition::hasPartPropertyNamed(const QString &name) const
{
    return findPartPropertyByName(name) != nullptr;
}

bool BlockDefinition::validatePartName(const QString &newName, PartProperty *excludePart) const
{
    for (PartProperty *part : m_partProperties)
    {
        if (part != excludePart && part->name() == newName)
        {
            return false;
        }
    }
    return true;
}

QString BlockDefinition::generateUniquePartName(BlockDefinition *type) const
{
    if (!type) return "part";
    
    // Base name is the type name in lowercase
    QString baseName = type->typeName().toLower();
    if (baseName.isEmpty()) baseName = "part";
    
    // Check if baseName already exists
    if (!hasPartPropertyNamed(baseName)) {
        return baseName;
    }
    
    // Find next available number
    int counter = 2;
    while (hasPartPropertyNamed(baseName + QString::number(counter))) {
        counter++;
    }
    
    return baseName + QString::number(counter);
}
