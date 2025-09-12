#ifndef BLOCKMENU_H
#define BLOCKMENU_H

#include <QListWidget>
#include <QDrag>

class BlockMenu : public QListWidget
{
public:
    BlockMenu(QWidget *parent = nullptr);
    virtual ~BlockMenu();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void startDrag(Qt::DropActions actions);

private:
    QPoint m_dragStartPosition;
    QColor getColorFromText(const QString &text);
};

#endif // BLOCKMENU_H
