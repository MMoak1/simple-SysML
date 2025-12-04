#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QPointF>
#include <QSizeF>

class StateMachineModel;

class BlockModel : public QObject
{
    Q_OBJECT

public:
    explicit BlockModel(const QColor &color, const QString &label, const QPointF &position = QPointF(0, 0), const QSizeF &size = QSizeF(100, 60), QObject *parent = nullptr);

    QColor color() const { return m_color; }
    void setColor(const QColor &color);

    QString label() const { return m_label; }
    void setLabel(const QString &label);

    QPointF position() const { return m_position; }
    void setPosition(const QPointF &position);

    QSizeF size() const { return m_size; }
    void setSize(const QSizeF &size);

    // For identification or future use
    QString id() const { return m_id; }

    // State machine association
    StateMachineModel *stateMachine() const { return m_stateMachine; }
    void setStateMachine(StateMachineModel *sm);
    bool hasStateMachine() const { return m_stateMachine != nullptr; }
    StateMachineModel *getOrCreateStateMachine();

signals:
    void colorChanged(const QColor &color);
    void labelChanged(const QString &label);
    void positionChanged(const QPointF &position);
    void sizeChanged(const QSizeF &size);
    void stateMachineChanged();

private:
    QColor m_color;
    QString m_label;
    QPointF m_position;
    QSizeF m_size;
    QString m_id; // Unique ID for the block
    StateMachineModel *m_stateMachine = nullptr;
};

#endif // BLOCKMODEL_H