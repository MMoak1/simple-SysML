#ifndef CONNECTIONMODEL_H
#define CONNECTIONMODEL_H

#include <QObject>
#include <QString>
#include <QPointF>
#include "blockmodel.h"

class ConnectionModel : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionModel(BlockModel *startBlock, BlockModel *endBlock, QObject *parent = nullptr);
    explicit ConnectionModel(const QString &startBlockId, const QString &endBlockId, QObject *parent = nullptr);

    QString startBlockId() const { return m_startBlockId; }
    QString endBlockId() const { return m_endBlockId; }

    BlockModel *startBlock() const { return m_startBlock; }
    BlockModel *endBlock() const { return m_endBlock; }

    void setStartBlock(BlockModel *block);
    void setEndBlock(BlockModel *block);

    QPointF startPoint() const;
    QPointF endPoint() const;

    QPointF startEdgePoint() const { return m_startEdgePoint; }
    QPointF endEdgePoint() const { return m_endEdgePoint; }

    bool isValid() const { return !m_startBlockId.isEmpty() && !m_endBlockId.isEmpty(); }

    // Label and multiplicity for named hierarchies
    QString label() const { return m_label; }
    void setLabel(const QString &label);
    
    QString multiplicity() const { return m_multiplicity; }
    void setMultiplicity(const QString &multiplicity);

signals:
    void connectionChanged();
    void startBlockChanged();
    void endBlockChanged();
    void labelChanged(const QString &label);

private:
    void updateConnection();
    void updateEdgePoints();
    QPointF calculateNearestEdgePoint(const QRectF &rect, const QPointF &target) const;
    QRectF getBlockRect(BlockModel *block) const;

    QString m_startBlockId;
    QString m_endBlockId;
    BlockModel *m_startBlock = nullptr;
    BlockModel *m_endBlock = nullptr;
    QPointF m_startEdgePoint;
    QPointF m_endEdgePoint;
    
    // Label and multiplicity for named hierarchies
    QString m_label;
    QString m_multiplicity;
};

#endif // CONNECTIONMODEL_H