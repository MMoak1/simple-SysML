#ifndef DROPCONTROLLER_H
#define DROPCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QString>
#include <QList>
#include <QMap>
#include "../models/blockdefinition.h"
#include "../views/blockdefinitionview.h"

class QGraphicsScene;
class DropGraphicsView;
class ConnectionController;

class DropController : public QObject
{
    Q_OBJECT

public:
    explicit DropController(QGraphicsScene *scene, DropGraphicsView *view, ConnectionController *connectionController, QObject *parent = nullptr);

    void handleDrop(const QString &blockType, const QPointF &position);
    void addBlock(BlockDefinition *definition);
    void deleteBlock(BlockDefinition *block);
    void clear();
    
    QList<BlockDefinition*> definitions() const { return m_definitions; }
    BlockDefinitionView* getViewForDefinition(BlockDefinition *definition) const;

signals:
    void blockCreated(BlockDefinition *definition, BlockDefinitionView *view);
    void blockDeleted(BlockDefinition *definition);

private:
    QGraphicsScene *m_scene;
    DropGraphicsView *m_view;
    ConnectionController *m_connectionController;
    QList<BlockDefinition*> m_definitions;
    QMap<BlockDefinition*, BlockDefinitionView*> m_definitionViews;

    BlockDefinition *createBlockDefinition(const QString &blockType);
    QColor getColorForType(const QString &blockType);
};

#endif // DROPCONTROLLER_H