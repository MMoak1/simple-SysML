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

    bool isValid() const { return !m_startBlockId.isEmpty() && !m_endBlockId.isEmpty(); }

signals:
    void connectionChanged();
    void startBlockChanged();
    void endBlockChanged();

private:
    void updateConnection();

    QString m_startBlockId;
    QString m_endBlockId;
    BlockModel *m_startBlock = nullptr;
    BlockModel *m_endBlock = nullptr;
};

#endif // CONNECTIONMODEL_H