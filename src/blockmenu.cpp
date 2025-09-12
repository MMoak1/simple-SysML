#include "../headers/blockmenu.h"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QListWidgetItem>
#include <QApplication>

BlockMenu::BlockMenu(QWidget *parent)
    : QListWidget(parent)
{
}

void BlockMenu::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPosition = event->pos();
    }
    QListWidget::mousePressEvent(event);
}

void BlockMenu::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || (event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        QListWidget::mouseMoveEvent(event);
        return;
    }

    startDrag(Qt::CopyAction);
}

void BlockMenu::startDrag(Qt::DropActions actions)
{
    QListWidgetItem *item = itemAt(m_dragStartPosition);
    if (!item)
    {
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText(item->text());
    drag->setMimeData(mimeData);

    // Optional: Set pixmap for drag cursor
    QPixmap pixmap(50, 30);
    pixmap.fill(getColorFromText(item->text())); // Wait, no getColor here; simple for now
    drag->setPixmap(pixmap);

    Qt::DropAction dropAction = drag->exec(actions, Qt::CopyAction);
}

QColor BlockMenu::getColorFromText(const QString &text) // Add this helper if needed, but for drag pixmap
{
    if (text == "Red Block")
        return Qt::red;
    if (text == "Blue Block")
        return Qt::blue;
    if (text == "Green Block")
        return Qt::green;
    return Qt::black;
}

BlockMenu::~BlockMenu()
{
}
