#include "../../headers/views/blockdefinitionview.h"
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
#include <QLineF>

// Initialize static members
bool BlockDefinitionView::s_drawingConnection = false;
BlockDefinitionView *BlockDefinitionView::s_connectionStartBlock = nullptr;
TemporaryConnectionLine *BlockDefinitionView::s_tempLine = nullptr;

BlockDefinitionView::BlockDefinitionView(BlockDefinition *definition, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_definition(definition)
{
    // Set initial properties from definition
    m_color = definition->color();
    m_typeName = definition->typeName();
    m_size = definition->size();

    // Make selectable and movable, and notify scene of geometry changes
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

    // Connect to definition signals
    // Note: definition has no colorChanged? BlockModel didn't have definition of color setter implementation details in summary but 
    // BlockDefinition has color() getter. BlockDefinition needs colorChanged signal if we want dynamic color updates. 
    // But BlockDefinition implementation shows it uses m_color only in constructor.
    // Wait, BlockModel had colorChanged. BlockDefinition header has setters but I might have missed implementing signals for all props.
    // Let's check BlockDefinition implementation again.
    
    connect(definition, &BlockDefinition::typeNameChanged, this, &BlockDefinitionView::updateTypeName);
    connect(definition, &BlockDefinition::positionChanged, this, &BlockDefinitionView::updatePosition);
    connect(definition, &BlockDefinition::sizeChanged, this, &BlockDefinitionView::updateSize);

    // Set initial position
    setPos(definition->position());
}

QRectF BlockDefinitionView::boundingRect() const
{
    // Include extra space for selection border (3px) plus margin
    const qreal margin = 5.0;
    return QRectF(-m_size.width() / 2 - margin, -m_size.height() / 2 - margin, 
                  m_size.width() + margin * 2, m_size.height() + margin * 2);
}

void BlockDefinitionView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        QRectF textRect = fm.boundingRect("Editing: " + m_typeName);

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
        painter->drawText(editRect, Qt::AlignCenter, "Editing: " + m_typeName);
    }
    else
    {
        // Draw normal title
        painter->drawText(localRect, Qt::AlignCenter, m_typeName);
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

void BlockDefinitionView::updateColor(const QColor &color)
{
    m_color = color;
    update(); // Trigger repaint
}

void BlockDefinitionView::updateTypeName(const QString &name)
{
    m_typeName = name;
    update(); // Trigger repaint
}

void BlockDefinitionView::updatePosition(const QPointF &position)
{
    setPos(position);
}

void BlockDefinitionView::updateSize(const QSizeF &size)
{
    prepareGeometryChange();
    m_size = size;
    update();
}

void BlockDefinitionView::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

            qDebug() << "Connection started from block:" << m_typeName;
            emit connectionStarted(this);
            event->accept();
            return;
        }

        // Check if clicking on resize handle - handle is at bottom-right of the block
        // The block rect is centered at origin: (-m_size.width()/2, -m_size.height()/2) to (+m_size.width()/2, +m_size.height()/2)
        qreal handleRadius = handleSize / 2;
        QPointF handleCenter(m_size.width() / 2 - handleRadius, m_size.height() / 2 - handleRadius);
        qreal distToHandle = QLineF(localPos, handleCenter).length();
        
        if (distToHandle <= handleSize)  // Within handleSize pixels of the handle center
        {
            m_resizing = true;
            m_originalSize = m_size;
            m_resizeStartPos = event->scenePos();
            event->accept();
            return;
        }

        // Check if clicking on title area for editing (single click on title)
        // Title is drawn centered in the block
        if (!m_resizing && !m_editingTitle)
        {
            // Calculate title area - make it generous for easier clicking
            QFont font = QApplication::font();
            QFontMetrics fm(font);
            QRectF textRect = fm.boundingRect(m_typeName);
            qreal titleHeight = textRect.height();
            qreal titleWidth = qMax(textRect.width(), 40.0);  // Minimum width for short names

            // Title area is centered in the block
            QRectF titleArea(
                -titleWidth / 2 - 15,  // Add generous padding
                -titleHeight / 2 - 10,
                titleWidth + 30,
                titleHeight + 20);

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

void BlockDefinitionView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        QPointF delta = event->scenePos() - m_resizeStartPos;
        QSizeF newSize = m_originalSize + QSizeF(delta.x(), delta.y());
        if (newSize.width() > 20 && newSize.height() > 20)
        {
            m_definition->setSize(newSize);
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
    qreal handleSize = 10.0;
    qreal handleRadius = handleSize / 2;
    QPointF handleCenter(m_size.width() / 2 - handleRadius, m_size.height() / 2 - handleRadius);
    qreal distToHandle = QLineF(localPos, handleCenter).length();
    
    if (distToHandle <= handleSize * 1.5)  // Slightly larger area for cursor feedback
    {
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else
    {
        unsetCursor();
    }
    // Call base class to handle normal dragging
    QGraphicsObject::mouseMoveEvent(event);

    // Update definition position when dragging (not resizing or drawing connection)
    if (!m_resizing && !s_drawingConnection && (flags() & ItemIsMovable))
    {
        m_definition->setPosition(pos());
    }
}

void BlockDefinitionView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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
        BlockDefinitionView *endBlock = findBlockAtPosition(event->scenePos());

        // Only create connection if we released over a different block
        if (endBlock && endBlock != s_connectionStartBlock)
        {
            qDebug() << "Connection completed from" << s_connectionStartBlock->definition()->typeName() << "to" << endBlock->definition()->typeName();
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

    // Update definition position after dragging
    if (!m_resizing && !s_drawingConnection)
    {
        m_definition->setPosition(pos());
    }

    QGraphicsObject::mouseReleaseEvent(event);
}

QVariant BlockDefinitionView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_definition)
    {
        // Sync definition position whenever the view position changes
        QPointF newPos = value.toPointF();
        if (m_definition->position() != newPos)
        {
            m_definition->setPosition(newPos);
        }
    }
    return QGraphicsObject::itemChange(change, value);
}

void BlockDefinitionView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        qDebug() << "Double-click on block:" << m_typeName << "- requesting state machine view";
        emit enterStateMachineRequested(this);
        event->accept();
        return;
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

void BlockDefinitionView::showTitleInputDialog()
{
    bool ok;
    QString newText = QInputDialog::getText(
        nullptr, // No parent widget needed for graphics scene
        "Edit Block Type Name",
        "Enter new type name:",
        QLineEdit::Normal,
        m_typeName,
        &ok);

    if (ok && !newText.isEmpty() && newText != m_typeName)
    {
        m_definition->setTypeName(newText);
    }

    m_editingTitle = false;
    update();
}

QPointF BlockDefinitionView::getNearestEdgePoint(const QPointF &targetPoint) const
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

BlockDefinitionView *BlockDefinitionView::findBlockAtPosition(const QPointF &scenePos) const
{
    if (!scene())
        return nullptr;

    // Get all items at the position
    QList<QGraphicsItem *> itemsAtPos = scene()->items(scenePos);

    // Find the first BlockDefinitionView that isn't this one
    for (QGraphicsItem *item : itemsAtPos)
    {
        BlockDefinitionView *blockView = dynamic_cast<BlockDefinitionView *>(item);
        if (blockView && blockView != this)
        {
            return blockView;
        }
    }

    return nullptr;
}
