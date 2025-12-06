#ifndef DROPCONTROLLER_H
#define DROPCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QString>
#include <QList>
#include <QMap>
#include "../models/blockmodel.h"
#include "../views/blockview.h"

class QGraphicsScene;
class DropGraphicsView;
class ConnectionController;

class DropController : public QObject
{
    Q_OBJECT

public:
    explicit DropController(QGraphicsScene *scene, DropGraphicsView *view, ConnectionController *connectionController, QObject *parent = nullptr);

    void handleDrop(const QString &blockType, const QPointF &position);
    void addBlock(BlockModel *model);
    void deleteBlock(BlockModel *block);
    void clear();
    
    QList<BlockModel*> blocks() const { return m_blocks; }
    BlockView* getViewForBlock(BlockModel *block) const;

signals:
    void blockCreated(BlockModel *model, BlockView *view);
    void blockDeleted(BlockModel *model);

private:
    QGraphicsScene *m_scene;
    DropGraphicsView *m_view;
    ConnectionController *m_connectionController;
    QList<BlockModel*> m_blocks;
    QMap<BlockModel*, BlockView*> m_blockViews;

    BlockModel *createBlockModel(const QString &blockType);
    QColor getColorForType(const QString &blockType);
};

#endif // DROPCONTROLLER_H