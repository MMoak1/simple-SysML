#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QPointF>
#include <QSizeF>

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

signals:
    void colorChanged(const QColor &color);
    void labelChanged(const QString &label);
    void positionChanged(const QPointF &position);
    void sizeChanged(const QSizeF &size);

private:
    QColor m_color;
    QString m_label;
    QPointF m_position;
    QSizeF m_size;
    QString m_id; // Unique ID for the block
};

#endif // BLOCKMODEL_H