#ifndef DROPGRAPHICSVIEW_H
#define DROPGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QString>

class DropGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    DropGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);
    virtual ~DropGraphicsView();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void dropPerformed(const QString &text, const QPointF &position);

private:
    QGraphicsScene *m_scene;
};

#endif // DROPGRAPHICSVIEW_H