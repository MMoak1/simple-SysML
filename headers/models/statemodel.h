#ifndef STATEMODEL_H
#define STATEMODEL_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QSizeF>
#include <QUuid>

// Enum for different state types in SysML state machines
enum class StateType {
    Initial,    // Starting state (filled circle)
    Normal,     // Regular state (rounded rectangle)
    Final       // End state (circled filled circle)
};

class StateModel : public QObject
{
    Q_OBJECT

public:
    explicit StateModel(const QString &label, 
                        StateType type = StateType::Normal,
                        const QPointF &position = QPointF(0, 0), 
                        const QSizeF &size = QSizeF(100, 60), 
                        QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }
    QString label() const { return m_label; }
    StateType stateType() const { return m_stateType; }
    QPointF position() const { return m_position; }
    QSizeF size() const { return m_size; }

    // Setters
    void setLabel(const QString &label);
    void setStateType(StateType type);
    void setPosition(const QPointF &position);
    void setSize(const QSizeF &size);

    // Clone method for instance creation
    StateModel *clone() const;

signals:
    void labelChanged(const QString &label);
    void stateTypeChanged(StateType type);
    void positionChanged(const QPointF &position);
    void sizeChanged(const QSizeF &size);

private:
    QString m_id;
    QString m_label;
    StateType m_stateType;
    QPointF m_position;
    QSizeF m_size;
};

#endif // STATEMODEL_H
