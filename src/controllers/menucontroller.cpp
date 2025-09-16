#include "../headers/controllers/menucontroller.h"
#include <QDrag>
#include <QMimeData>
#include <QPixmap>

MenuController::MenuController(BlockMenuView *menuView, QObject *parent)
    : QObject(parent), m_menuView(menuView)
{
    connect(menuView, &BlockMenuView::dragStarted, this, &MenuController::onDragStarted);
}

void MenuController::onDragStarted(const QString &blockType)
{
    QDrag *drag = createDrag(blockType);
    if (drag)
    {
        drag->exec(Qt::CopyAction);
        delete drag;
    }
}

QDrag *MenuController::createDrag(const QString &blockType)
{
    QDrag *drag = new QDrag(m_menuView);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(blockType);
    drag->setMimeData(mimeData);

    // Set pixmap for drag cursor
    QPixmap pixmap(50, 30);
    pixmap.fill(getColorForType(blockType));
    drag->setPixmap(pixmap);

    return drag;
}

QColor MenuController::getColorForType(const QString &blockType)
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
    return Qt::black;
}