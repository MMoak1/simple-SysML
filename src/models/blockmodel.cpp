#include "../headers/models/blockmodel.h"
#include <QUuid>

BlockModel::BlockModel(const QColor &color, const QString &label, const QPointF &position, QObject *parent)
    : QObject(parent), m_color(color), m_label(label), m_position(position)
{
    m_id = QUuid::createUuid().toString();
}

void BlockModel::setColor(const QColor &color)
{
    if (m_color != color)
    {
        m_color = color;
        emit colorChanged(color);
    }
}

void BlockModel::setLabel(const QString &label)
{
    if (m_label != label)
    {
        m_label = label;
        emit labelChanged(label);
    }
}

void BlockModel::setPosition(const QPointF &position)
{
    if (m_position != position)
    {
        m_position = position;
        emit positionChanged(position);
    }
}