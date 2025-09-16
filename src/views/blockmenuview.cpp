#include "../headers/views/blockmenuview.h"
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QApplication>

BlockMenuView::BlockMenuView(QWidget *parent)
    : QListWidget(parent)
{
}

void BlockMenuView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPosition = event->pos();
    }
    QListWidget::mousePressEvent(event);
}

void BlockMenuView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || (event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        QListWidget::mouseMoveEvent(event);
        return;
    }

    // Get the item at drag start position
    QListWidgetItem *item = itemAt(m_dragStartPosition);
    if (item)
    {
        emit dragStarted(item->text());
    }
}

BlockMenuView::~BlockMenuView()
{
}
