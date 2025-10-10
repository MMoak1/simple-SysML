#ifndef DROPCONTROLLER_H
#define DROPCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QString>
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

signals:
    void blockCreated(BlockModel *model, BlockView *view);

private:
    QGraphicsScene *m_scene;
    DropGraphicsView *m_view;
    ConnectionController *m_connectionController;

    BlockModel *createBlockModel(const QString &blockType);
    QColor getColorForType(const QString &blockType);
};

#endif // DROPCONTROLLER_H