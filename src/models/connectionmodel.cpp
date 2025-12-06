#include "../../headers/models/connectionmodel.h"
#include <QDebug>
#include <QRectF>

ConnectionModel::ConnectionModel(BlockModel *startBlock, BlockModel *endBlock, QObject *parent)
    : QObject(parent), m_startBlock(startBlock), m_endBlock(endBlock)
{
    if (startBlock)
    {
        m_startBlockId = startBlock->id();
        connect(startBlock, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
        connect(startBlock, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
    }
    if (endBlock)
    {
        m_endBlockId = endBlock->id();
        connect(endBlock, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
        connect(endBlock, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
    }

    // Calculate initial edge points
    updateEdgePoints();
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
            disconnect(m_startBlock, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
            disconnect(m_startBlock, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
        }

        m_startBlock = block;
        m_startBlockId = block ? block->id() : QString();

        // Connect new block
        if (block)
        {
            connect(block, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
            connect(block, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
        }

        emit startBlockChanged();
        updateEdgePoints();
    }
}

void ConnectionModel::setEndBlock(BlockModel *block)
{
    if (m_endBlock != block)
    {
        // Disconnect previous block
        if (m_endBlock)
        {
            disconnect(m_endBlock, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
            disconnect(m_endBlock, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
        }

        m_endBlock = block;
        m_endBlockId = block ? block->id() : QString();

        // Connect new block
        if (block)
        {
            connect(block, &BlockModel::positionChanged, this, &ConnectionModel::updateEdgePoints);
            connect(block, &BlockModel::sizeChanged, this, &ConnectionModel::updateEdgePoints);
        }

        emit endBlockChanged();
        updateEdgePoints();
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

void ConnectionModel::updateEdgePoints()
{
    if (!m_startBlock || !m_endBlock)
    {
        return;
    }

    // Get block rectangles
    QRectF startRect = getBlockRect(m_startBlock);
    QRectF endRect = getBlockRect(m_endBlock);

    // Calculate nearest edge points
    m_startEdgePoint = calculateNearestEdgePoint(startRect, endRect.center());
    m_endEdgePoint = calculateNearestEdgePoint(endRect, startRect.center());

    emit connectionChanged();
}

QPointF ConnectionModel::calculateNearestEdgePoint(const QRectF &rect, const QPointF &target) const
{
    QPointF center = rect.center();

    // Calculate vector from center to target
    qreal dx = target.x() - center.x();
    qreal dy = target.y() - center.y();

    // Determine which edge to use based on dominant direction
    if (qAbs(dx) > qAbs(dy))
    {
        // Horizontal dominant - use left or right edge
        if (dx > 0)
        {
            // Right edge
            return QPointF(rect.right(), center.y());
        }
        else
        {
            // Left edge
            return QPointF(rect.left(), center.y());
        }
    }
    else
    {
        // Vertical dominant - use top or bottom edge
        if (dy > 0)
        {
            // Bottom edge
            return QPointF(center.x(), rect.bottom());
        }
        else
        {
            // Top edge
            return QPointF(center.x(), rect.top());
        }
    }
}

QRectF ConnectionModel::getBlockRect(BlockModel *block) const
{
    if (!block)
    {
        return QRectF();
    }

    QPointF pos = block->position();
    QSizeF size = block->size();

    // BlockView uses centered coordinates, so adjust
    return QRectF(pos.x() - size.width() / 2,
                  pos.y() - size.height() / 2,
                  size.width(),
                  size.height());
}