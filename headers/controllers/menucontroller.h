#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <QObject>
#include <QDrag>
#include "../views/blockmenuview.h"

class MenuController : public QObject
{
    Q_OBJECT

public:
    explicit MenuController(BlockMenuView *menuView, QObject *parent = nullptr);

private slots:
    void onDragStarted(const QString &blockType);

private:
    BlockMenuView *m_menuView;
    QDrag *createDrag(const QString &blockType);
    QColor getColorForType(const QString &blockType);
};

#endif // MENUCONTROLLER_H