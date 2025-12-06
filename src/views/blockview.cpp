#include "../../headers/views/blockview.h"
#include "../../headers/views/temporaryconnectionline.h"
#include <QPainter>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPen>
#include <QCursor>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QApplication>
#include <QInputDialog>
#include <QDebug>

// Initialize static members
bool BlockView::s_drawingConnection = false;
BlockView *BlockView::s_connectionStartBlock = nullptr;
TemporaryConnectionLine *BlockView::s_tempLine = nullptr;

BlockView::BlockView(BlockModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model)
{
    // Set initial properties from model
    m_color = model->color();
    m_label = model->label();
    m_size = model->size();

    // Make selectable and movable, and notify scene of geometry changes
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

    // Connect to model signals
    connect(model, &BlockModel::colorChanged, this, &BlockView::updateColor);
    connect(model, &BlockModel::labelChanged, this, &BlockView::updateLabel);
    connect(model, &BlockModel::positionChanged, this, &BlockView::updatePosition);
    connect(model, &BlockModel::sizeChanged, this, &BlockView::updateSize);

    // Set initial position
    setPos(model->position());
}

QRectF BlockView::boundingRect() const
{
    // Include extra space for selection border (3px) plus margin
    const qreal margin = 5.0;
    return QRectF(-m_size.width() / 2 - margin, -m_size.height() / 2 - margin, 
                  m_size.width() + margin * 2, m_size.height() + margin * 2);
}

void BlockView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    QRectF localRect(-m_size.width() / 2, -m_size.height() / 2, m_size.width(), m_size.height());

    if (m_resizing)
    {
        // Draw resize border
        painter->save();
        QPen borderPen(Qt::black, 3);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(localRect);
        painter->restore();
    }

    // Draw the rectangle background
    painter->fillRect(localRect, QBrush(m_color));

    // Draw title or editing indicator
    painter->save();
    painter->setPen(Qt::black);

    if (m_editingTitle)
    {
        // Draw editing indicator
        QFont font = QApplication::font();
        QFontMetrics fm(font);
        QRectF textRect = fm.boundingRect("Editing: " + m_label);

        // Draw a text input box background
        QRectF editRect(
            -textRect.width() / 2 - 15,
            -m_size.height() / 2 + 5,
            textRect.width() + 30,
            textRect.height() + 10);

        // Draw semi-transparent white background for edit box
        painter->fillRect(editRect, QBrush(QColor(255, 255, 255, 200)));

        // Draw border around edit area
        QPen editPen(QColor(0, 0, 0, 150), 2);
        painter->setPen(editPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(editRect);

        // Draw "Editing:" text
        painter->setPen(Qt::black);
        painter->drawText(editRect, Qt::AlignCenter, "Editing: " + m_label);
    }
    else
    {
        // Draw normal title
        painter->drawText(localRect, Qt::AlignCenter, m_label);
    }

    painter->restore();

    // Draw resize handle - black dot
    qreal handleSize = 10.0;
    qreal radius = handleSize / 2;
    QPointF handleCenter(localRect.right() - radius, localRect.bottom() - radius);
    painter->save();
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(handleCenter, radius, radius);
    painter->restore();
    
    // Draw selection border (light yellow)
    if (isSelected())
    {
        painter->save();
        QPen selectionPen(QColor(255, 255, 0), 3); // Yellow border
        painter->setPen(selectionPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(localRect.adjusted(-2, -2, 2, 2));
        painter->restore();
    }
}

void BlockView::updateColor(const QColor &color)
{
    m_color = color;
    update(); // Trigger repaint
}

void BlockView::updateLabel(const QString &label)
{
    m_label = label;
    update(); // Trigger repaint
}

void BlockView::updatePosition(const QPointF &position)
{
    setPos(position);
}

void BlockView::updateSize(const QSizeF &size)
{
    prepareGeometryChange();
    m_size = size;
    update();
}

void BlockView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF localPos = mapFromScene(event->scenePos());
        qreal handleSize = 10.0;
        QRectF bounding = boundingRect();

        // Check for Ctrl+Click to start connection drawing
        if (event->modifiers() & Qt::ControlModifier)
        {
            s_drawingConnection = true;
            s_connectionStartBlock = this;

            // Calculate start point at nearest edge
            QPointF startPoint = getNearestEdgePoint(event->scenePos());

            // Create temporary line
            s_tempLine = new TemporaryConnectionLine(startPoint);
            scene()->addItem(s_tempLine);

            qDebug() << "Connection started from block:" << m_label;
            emit connectionStarted(this);
            event->accept();
            return;
        }

        // Check if clicking on resize handle
        if (localPos.x() > bounding.width() / 2 - handleSize &&
            localPos.y() > bounding.height() / 2 - handleSize)
        {
            m_resizing = true;
            m_originalSize = m_size;
            m_resizeStartPos = event->scenePos();
            event->accept();
            return;
        }

        // Check if clicking on title area for editing
        if (!m_resizing && !m_editingTitle)
        {
            // Calculate title area (center portion of the block)
            QFont font = QApplication::font();
            QFontMetrics fm(font);
            QRectF textRect = fm.boundingRect(m_label);
            qreal titleHeight = textRect.height();
            qreal titleWidth = textRect.width();

            // Title area is in the center horizontally and upper portion vertically
            QRectF titleArea(
                -titleWidth / 2 - 10, // Add some padding
                -m_size.height() / 2 + 5,
                titleWidth + 20,
                titleHeight + 10);

            if (titleArea.contains(localPos))
            {
                m_editingTitle = true;
                showTitleInputDialog();
                event->accept();
                return;
            }
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

void BlockView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        QPointF delta = event->scenePos() - m_resizeStartPos;
        QSizeF newSize = m_originalSize + QSizeF(delta.x(), delta.y());
        if (newSize.width() > 20 && newSize.height() > 20)
        {
            m_model->setSize(newSize);
        }
        event->accept();
        return;
    }

    // Update temporary connection line if drawing
    if (s_drawingConnection && s_tempLine)
    {
        s_tempLine->updateEndPoint(event->scenePos());
        event->accept();
        return;
    }

    // Check for resize cursor
    QPointF localPos = mapFromScene(event->scenePos());
    QRectF bounding = boundingRect();
    qreal handleSize = 10.0;
    if (localPos.x() > bounding.width() / 2 - handleSize &&
        localPos.y() > bounding.height() / 2 - handleSize)
    {
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else
    {
        unsetCursor();
    }
    // Call base class to handle normal dragging
    QGraphicsObject::mouseMoveEvent(event);

    // Update model position when dragging (not resizing or drawing connection)
    if (!m_resizing && !s_drawingConnection && (flags() & ItemIsMovable))
    {
        m_model->setPosition(pos());
    }
}

void BlockView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        m_resizing = false;
        event->accept();
        return;
    }

    // Handle connection completion
    if (s_drawingConnection && s_connectionStartBlock)
    {
        // Find block at release position
        BlockView *endBlock = findBlockAtPosition(event->scenePos());

        // Only create connection if we released over a different block
        if (endBlock && endBlock != s_connectionStartBlock)
        {
            qDebug() << "Connection completed from" << s_connectionStartBlock->model()->label() << "to" << endBlock->model()->label();
            emit connectionCompleted(s_connectionStartBlock, endBlock);
        }
        else
        {
            qDebug() << "Connection cancelled - no valid target block";
        }

        // Clean up temporary line
        if (s_tempLine)
        {
            scene()->removeItem(s_tempLine);
            delete s_tempLine;
            s_tempLine = nullptr;
        }

        // Reset connection state
        s_drawingConnection = false;
        s_connectionStartBlock = nullptr;
        event->accept();
        return;
    }

    // Update model position after dragging
    if (!m_resizing && !s_drawingConnection)
    {
        m_model->setPosition(pos());
    }

    QGraphicsObject::mouseReleaseEvent(event);
}

QVariant BlockView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_model)
    {
        // Sync model position whenever the view position changes
        // This handles the case where multiple items are selected and moved together
        QPointF newPos = value.toPointF();
        if (m_model->position() != newPos)
        {
            m_model->setPosition(newPos);
        }
    }
    return QGraphicsObject::itemChange(change, value);
}

void BlockView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        qDebug() << "Double-click on block:" << m_label << "- requesting state machine view";
        emit enterStateMachineRequested(this);
        event->accept();
        return;
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

void BlockView::showTitleInputDialog()
{
    bool ok;
    QString newText = QInputDialog::getText(
        nullptr, // No parent widget needed for graphics scene
        "Edit Block Title",
        "Enter new title:",
        QLineEdit::Normal,
        m_label,
        &ok);

    if (ok && !newText.isEmpty() && newText != m_label)
    {
        m_model->setLabel(newText);
    }

    m_editingTitle = false;
    update();
}

QPointF BlockView::getNearestEdgePoint(const QPointF &targetPoint) const
{
    QRectF rect = sceneBoundingRect();
    QPointF center = rect.center();

    // Calculate vector from center to target
    qreal dx = targetPoint.x() - center.x();
    qreal dy = targetPoint.y() - center.y();

    // Determine which edge to use based on dominant direction
    if (qAbs(dx) > qAbs(dy))
    {
        // Horizontal dominant - use left or right edge
        if (dx > 0)
        {
            // Right edge
            return QPointF(rect.right(), center.y());
        }
        else
        {
            // Left edge
            return QPointF(rect.left(), center.y());
        }
    }
    else
    {
        // Vertical dominant - use top or bottom edge
        if (dy > 0)
        {
            // Bottom edge
            return QPointF(center.x(), rect.bottom());
        }
        else
        {
            // Top edge
            return QPointF(center.x(), rect.top());
        }
    }
}

BlockView *BlockView::findBlockAtPosition(const QPointF &scenePos) const
{
    if (!scene())
        return nullptr;

    // Get all items at the position
    QList<QGraphicsItem *> itemsAtPos = scene()->items(scenePos);

    // Find the first BlockView that isn't this one
    for (QGraphicsItem *item : itemsAtPos)
    {
        BlockView *blockView = dynamic_cast<BlockView *>(item);
        if (blockView && blockView != this)
        {
            return blockView;
        }
    }

    return nullptr;
}