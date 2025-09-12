#ifndef BLOCKITEM_H
#define BLOCKITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QString>

class BlockItem : public QGraphicsRectItem
{
public:
    BlockItem(const QColor &color, const QString &label, QGraphicsItem *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QColor m_color;
    QString m_label;
};

#endif // BLOCKITEM_H