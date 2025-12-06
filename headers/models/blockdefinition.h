#ifndef BLOCKDEFINITION_H
#define BLOCKDEFINITION_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QPointF>
#include <QSizeF>
#include <QList>
#include "statemachinemodel.h"

class PartProperty;

class BlockDefinition : public QObject
{
    Q_OBJECT

public:
    explicit BlockDefinition(const QString &typeName, const QColor &color, 
                             const QPointF &position, QObject *parent = nullptr);
    ~BlockDefinition();

    // Identity
    QString id() const { return m_id; }
    void setId(const QString &id);
    
    // Type name (e.g., "Car", "Wheel")
    QString typeName() const { return m_typeName; }
    void setTypeName(const QString &name);

    // Visual properties (for BDD canvas)
    QColor color() const { return m_color; }
    QPointF position() const { return m_position; }
    QSizeF size() const { return m_size; }
    void setPosition(const QPointF &pos);
    void setSize(const QSizeF &size);

    // State machine (behavioral definition template)
    StateMachineModel* stateMachine() const { return m_stateMachine; }
    StateMachineModel* getOrCreateStateMachine();
    bool hasStateMachine() const { return m_stateMachine != nullptr; }

    // Part properties (composition relationships)
    QList<PartProperty*> partProperties() const { return m_partProperties; }
    PartProperty* addPartProperty(const QString &name, BlockDefinition *type, 
                                   const QString &multiplicity = "1");
    bool removePartProperty(PartProperty *part);
    PartProperty* findPartPropertyByName(const QString &name) const;
    bool hasPartPropertyNamed(const QString &name) const;
    
    // Check if name is valid (unique among siblings)
    bool validatePartName(const QString &newName, PartProperty *excludePart = nullptr) const;
    
    // Auto-generate unique part name
    QString generateUniquePartName(BlockDefinition *type) const;

signals:
    void typeNameChanged(const QString &name);
    void positionChanged(const QPointF &pos);
    void sizeChanged(const QSizeF &size);
    void partPropertyAdded(PartProperty *part);
    void partPropertyRemoved(PartProperty *part);

private:
    QString m_id;
    QString m_typeName;
    QColor m_color;
    QPointF m_position;
    QSizeF m_size;
    StateMachineModel *m_stateMachine = nullptr;
    QList<PartProperty*> m_partProperties;
};

#endif // BLOCKDEFINITION_H
