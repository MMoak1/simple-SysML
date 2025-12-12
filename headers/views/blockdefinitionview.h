#ifndef BLOCKDEFINITIONVIEW_H
#define BLOCKDEFINITIONVIEW_H

#include <QGraphicsObject>
#include <QColor>
#include <QString>
#include <QSizeF>
#include <QPointF>
#include "../models/blockdefinition.h"

class TemporaryConnectionLine;

class BlockDefinitionView : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit BlockDefinitionView(BlockDefinition *definition, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    BlockDefinition *definition() const { return m_definition; }

signals:
    void connectionStarted(BlockDefinitionView *startBlock);
    void connectionCompleted(BlockDefinitionView *startBlock, BlockDefinitionView *endBlock);
    void enterStateMachineRequested(BlockDefinitionView *block);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private slots:
    void updateColor(const QColor &color);
    void updateTypeName(const QString &name);
    void updatePosition(const QPointF &position);
    void updateSize(const QSizeF &size);

private:
    BlockDefinition *m_definition;
    QString m_typeName;
    QColor m_color;
    QSizeF m_size;

    // Resize state
    bool m_resizing = false;
    QPointF m_resizeStartPos;
    QSizeF m_originalSize;

    // Title editing state
    bool m_editingTitle = false;

    // Connection drawing state (static to share across all BlockDefinitionView instances)
    static bool s_drawingConnection;
    static BlockDefinitionView *s_connectionStartBlock;
    static TemporaryConnectionLine *s_tempLine;

    void showTitleInputDialog();
    QPointF getNearestEdgePoint(const QPointF &targetPoint) const;
    BlockDefinitionView *findBlockAtPosition(const QPointF &scenePos) const;
};

#endif // BLOCKDEFINITIONVIEW_H
