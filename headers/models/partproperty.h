#ifndef PARTPROPERTY_H
#define PARTPROPERTY_H

#include <QObject>
#include <QString>
#include <QPointer>
#include "blockdefinition.h"
#include "statemachinemodel.h"

class PartProperty : public QObject
{
    Q_OBJECT

public:
    explicit PartProperty(const QString &name, BlockDefinition *type,
                          const QString &multiplicity = "1", QObject *parent = nullptr);
    ~PartProperty();

    // Identity
    QString id() const { return m_id; }
    void setId(const QString &id);
    
    // Part name (e.g., "wheels", "engine")
    QString name() const { return m_name; }
    void setName(const QString &name);
    
    // Multiplicity (e.g., "1", "4", "0..*")
    QString multiplicity() const { return m_multiplicity; }
    void setMultiplicity(const QString &mult);
    
    // Type reference (what type does this part hold?)
    BlockDefinition* type() const { return m_type.data(); }
    void setType(BlockDefinition *type);
    
    // Owner (which BlockDefinition owns this part?)
    BlockDefinition* owner() const { return m_owner; }
    void setOwner(BlockDefinition *owner);
    
    // Display text for hierarchy tree and canvas
    // Format: "[multiplicity] name : TypeName"
    QString displayText() const;
    
    // State machine INSTANCE (independent copy of type's template)
    StateMachineModel* stateMachineInstance() const { return m_stateMachineInstance; }
    StateMachineModel* getOrCreateStateMachineInstance();
    bool hasStateMachineInstance() const { return m_stateMachineInstance != nullptr; }

signals:
    void nameChanged(const QString &name);
    void multiplicityChanged(const QString &mult);
    void typeChanged(BlockDefinition *type);
    void stateMachineInstanceChanged();

private:
    QString m_id;
    QString m_name;
    QString m_multiplicity;
    QPointer<BlockDefinition> m_type;
    BlockDefinition *m_owner = nullptr;
    StateMachineModel *m_stateMachineInstance = nullptr;
};

#endif // PARTPROPERTY_H
