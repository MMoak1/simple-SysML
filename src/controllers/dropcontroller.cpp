#include "../headers/controllers/dropcontroller.h"
#include <QGraphicsScene>
#include "views/blockview.h"
#include "controllers/connectioncontroller.h"

DropController::DropController(QGraphicsScene *scene, DropGraphicsView *view, ConnectionController *connectionController, QObject *parent)
    : QObject(parent), m_scene(scene), m_view(view), m_connectionController(connectionController)
{
}

void DropController::handleDrop(const QString &blockType, const QPointF &position)
{
    BlockModel *model = createBlockModel(blockType);
    if (model)
    {
        model->setPosition(position);

        BlockView *view = new BlockView(model, nullptr);
        m_scene->addItem(view);

        // Register the new block with the connection controller
        if (m_connectionController)
        {
            m_connectionController->registerBlockView(view);
        }
    }
}

BlockModel *DropController::createBlockModel(const QString &blockType)
{
    QColor color = getColorForType(blockType);
    if (!color.isValid())
    {
        return nullptr;
    }

    return new BlockModel(color, blockType, QPointF(0, 0));
}

QColor DropController::getColorForType(const QString &blockType)
{
    if (blockType == "Red Block")
    {
        return Qt::red;
    }
    else if (blockType == "Blue Block")
    {
        return Qt::blue;
    }
    else if (blockType == "Green Block")
    {
        return Qt::green;
    }
    return QColor();
}