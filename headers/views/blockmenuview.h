#ifndef BLOCKMENUVIEW_H
#define BLOCKMENUVIEW_H

#include <QListWidget>
#include <QPoint>

class BlockMenuView : public QListWidget
{
    Q_OBJECT

public:
    explicit BlockMenuView(QWidget *parent = nullptr);
    ~BlockMenuView();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void dragStarted(const QString &blockType);

private:
    QPoint m_dragStartPosition;
};

#endif // BLOCKMENUVIEW_H
