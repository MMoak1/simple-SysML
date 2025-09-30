#include "../../headers/models/connectionmodel.h"
#include <QDebug>

ConnectionModel::ConnectionModel(BlockModel *startBlock, BlockModel *endBlock, QObject *parent)
    : QObject(parent), m_startBlock(startBlock), m_endBlock(endBlock)
{
    if (startBlock)
    {
        m_startBlockId = startBlock->id();
        connect(startBlock, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
    }
    if (endBlock)
    {
        m_endBlockId = endBlock->id();
        connect(endBlock, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
    }
}

ConnectionModel::ConnectionModel(const QString &startBlockId, const QString &endBlockId, QObject *parent)
    : QObject(parent), m_startBlockId(startBlockId), m_endBlockId(endBlockId)
{
}

void ConnectionModel::setStartBlock(BlockModel *block)
{
    if (m_startBlock != block)
    {
        // Disconnect previous block
        if (m_startBlock)
        {
            disconnect(m_startBlock, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
        }

        m_startBlock = block;
        m_startBlockId = block ? block->id() : QString();

        // Connect new block
        if (block)
        {
            connect(block, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
        }

        emit startBlockChanged();
        emit connectionChanged();
    }
}

void ConnectionModel::setEndBlock(BlockModel *block)
{
    if (m_endBlock != block)
    {
        // Disconnect previous block
        if (m_endBlock)
        {
            disconnect(m_endBlock, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
        }

        m_endBlock = block;
        m_endBlockId = block ? block->id() : QString();

        // Connect new block
        if (block)
        {
            connect(block, &BlockModel::positionChanged, this, &ConnectionModel::connectionChanged);
        }

        emit endBlockChanged();
        emit connectionChanged();
    }
}

QPointF ConnectionModel::startPoint() const
{
    if (m_startBlock)
    {
        QPointF pos = m_startBlock->position();
        QSizeF size = m_startBlock->size();
        return QPointF(pos.x() + size.width() / 2, pos.y() + size.height() / 2);
    }
    return QPointF(0, 0);
}

QPointF ConnectionModel::endPoint() const
{
    if (m_endBlock)
    {
        QPointF pos = m_endBlock->position();
        QSizeF size = m_endBlock->size();
        return QPointF(pos.x() + size.width() / 2, pos.y() + size.height() / 2);
    }
    return QPointF(0, 0);
}

void ConnectionModel::updateConnection()
{
    emit connectionChanged();
}