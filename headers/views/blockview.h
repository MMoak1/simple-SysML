#ifndef BLOCKVIEW_H
#define BLOCKVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include <QString>
#include <QSizeF>
#include <QPointF>
#include "../models/blockmodel.h"

class TemporaryConnectionLine;

class BlockView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit BlockView(BlockModel *model, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    BlockModel *model() const { return m_model; }

signals:
    void connectionStarted(BlockView *startBlock);
    void connectionCompleted(BlockView *startBlock, BlockView *endBlock);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void updateColor(const QColor &color);
    void updateLabel(const QString &label);
    void updatePosition(const QPointF &position);
    void updateSize(const QSizeF &size);

private:
    BlockModel *m_model;
    QString m_label;
    QColor m_color;
    QSizeF m_size;

    // Resize state
    bool m_resizing = false;
    QPointF m_resizeStartPos;
    QSizeF m_originalSize;

    // Title editing state
    bool m_editingTitle = false;

    // Connection drawing state
    bool m_drawingConnection = false;
    BlockView *m_connectionStartBlock = nullptr;
    TemporaryConnectionLine *m_tempLine = nullptr;

    void showTitleInputDialog();
    QPointF getNearestEdgePoint(const QPointF &targetPoint) const;
    BlockView *findBlockAtPosition(const QPointF &scenePos) const;
};

#endif // BLOCKVIEW_H