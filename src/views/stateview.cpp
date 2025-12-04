#include "../../headers/views/stateview.h"
#include "../../headers/views/temporaryconnectionline.h"
#include "../../headers/models/statemodel.h"
#include <QPainter>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPen>
#include <QCursor>
#include <QFontMetrics>
#include <QApplication>
#include <QInputDialog>
#include <QDebug>

// Initialize static members
bool StateView::s_drawingTransition = false;
StateView *StateView::s_transitionStartState = nullptr;
TemporaryConnectionLine *StateView::s_tempLine = nullptr;

StateView::StateView(StateModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model)
{
    // Set initial properties from model
    m_label = model->label();
    m_size = model->size();

    // Make selectable and movable, and notify scene of geometry changes
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    
    // Enable caching for better rendering performance
    setCacheMode(DeviceCoordinateCache);

    // Connect to model signals
    connect(model, &StateModel::labelChanged, this, &StateView::updateLabel);
    connect(model, &StateModel::positionChanged, this, &StateView::updatePosition);
    connect(model, &StateModel::sizeChanged, this, &StateView::updateSize);
    connect(model, &StateModel::stateTypeChanged, this, &StateView::updateStateType);

    // Set initial position
    setPos(model->position());
}

QRectF StateView::boundingRect() const
{
    StateType type = m_model->stateType();
    
    if (type == StateType::Initial || type == StateType::Final)
    {
        // Circular shape for initial/final states
        qreal radius = 15.0;
        if (type == StateType::Final)
            radius = 20.0; // Slightly larger for final state (outer circle)
        return QRectF(-radius, -radius, radius * 2, radius * 2);
    }
    
    // Normal state - rounded rectangle
    return QRectF(-m_size.width() / 2, -m_size.height() / 2, m_size.width(), m_size.height());
}

void StateView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    StateType type = m_model->stateType();
    
    if (type == StateType::Initial)
    {
        // Initial state: filled black circle
        qreal radius = 10.0;
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), radius, radius);
    }
    else if (type == StateType::Final)
    {
        // Final state: filled circle inside an outer circle
        qreal outerRadius = 15.0;
        qreal innerRadius = 10.0;
        
        // Draw outer circle
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::black, 2));
        painter->drawEllipse(QPointF(0, 0), outerRadius, outerRadius);
        
        // Draw inner filled circle
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), innerRadius, innerRadius);
    }
    else
    {
        // Normal state: rounded rectangle
        QRectF localRect = boundingRect();
        qreal cornerRadius = 10.0;
        
        // Draw background
        QColor stateColor(200, 220, 240); // Light blue
        painter->setBrush(stateColor);
        painter->setPen(QPen(Qt::black, 2));
        painter->drawRoundedRect(localRect, cornerRadius, cornerRadius);
        
        // Draw label
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(localRect, Qt::AlignCenter, m_label);
        
        // Draw resize handle if normal state
        if (m_resizing || isSelected())
        {
            qreal handleSize = 8.0;
            qreal radius = handleSize / 2;
            QPointF handleCenter(localRect.right() - radius - 2, localRect.bottom() - radius - 2);
            painter->setBrush(Qt::darkGray);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(handleCenter, radius, radius);
        }
    }
    
    // Draw selection indicator
    if (isSelected())
    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->drawRect(boundingRect().adjusted(-2, -2, 2, 2));
    }
}

void StateView::updateLabel(const QString &label)
{
    m_label = label;
    update();
}

void StateView::updatePosition(const QPointF &position)
{
    setPos(position);
}

void StateView::updateSize(const QSizeF &size)
{
    prepareGeometryChange();
    m_size = size;
    update();
}

void StateView::updateStateType()
{
    prepareGeometryChange();
    update();
}

void StateView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF localPos = mapFromScene(event->scenePos());
        QRectF bounding = boundingRect();
        
        // Only allow resize for normal states
        if (m_model->stateType() == StateType::Normal)
        {
            qreal handleSize = 10.0;
            
            // Check for Ctrl+Click to start transition drawing
            if (event->modifiers() & Qt::ControlModifier)
            {
                s_drawingTransition = true;
                s_transitionStartState = this;
                
                QPointF startPoint = getNearestEdgePoint(event->scenePos());
                s_tempLine = new TemporaryConnectionLine(startPoint);
                scene()->addItem(s_tempLine);
                
                qDebug() << "Transition started from state:" << m_label;
                emit transitionStarted(this);
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
            
            // Check for title click to edit
            QFont font = QApplication::font();
            QFontMetrics fm(font);
            QRectF textRect = fm.boundingRect(m_label);
            QRectF titleArea(-textRect.width() / 2 - 10, -textRect.height() / 2 - 5,
                             textRect.width() + 20, textRect.height() + 10);
            
            if (titleArea.contains(localPos))
            {
                showTitleInputDialog();
                event->accept();
                return;
            }
        }
        else
        {
            // For initial/final states, only allow transition creation
            if (event->modifiers() & Qt::ControlModifier)
            {
                s_drawingTransition = true;
                s_transitionStartState = this;
                
                QPointF startPoint = scenePos();
                s_tempLine = new TemporaryConnectionLine(startPoint);
                scene()->addItem(s_tempLine);
                
                emit transitionStarted(this);
                event->accept();
                return;
            }
        }
    }
    
    QGraphicsObject::mousePressEvent(event);
}

void StateView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        QPointF delta = event->scenePos() - m_resizeStartPos;
        QSizeF newSize = m_originalSize + QSizeF(delta.x(), delta.y());
        if (newSize.width() > 40 && newSize.height() > 30)
        {
            m_model->setSize(newSize);
        }
        event->accept();
        return;
    }
    
    // Update temporary transition line if drawing
    if (s_drawingTransition && s_tempLine)
    {
        s_tempLine->updateEndPoint(event->scenePos());
        event->accept();
        return;
    }
    
    QGraphicsObject::mouseMoveEvent(event);
    
    // Update model position when dragging
    if (!m_resizing && !s_drawingTransition && (flags() & ItemIsMovable))
    {
        m_model->setPosition(pos());
    }
}

void StateView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing)
    {
        m_resizing = false;
        event->accept();
        return;
    }
    
    // Handle transition completion
    if (s_drawingTransition && s_transitionStartState)
    {
        StateView *endState = findStateAtPosition(event->scenePos());
        
        if (endState && endState != s_transitionStartState)
        {
            qDebug() << "Transition completed from" << s_transitionStartState->model()->label() 
                     << "to" << endState->model()->label();
            emit transitionCompleted(s_transitionStartState, endState);
        }
        else
        {
            qDebug() << "Transition cancelled - no valid target state";
        }
        
        // Clean up temporary line
        if (s_tempLine)
        {
            scene()->removeItem(s_tempLine);
            delete s_tempLine;
            s_tempLine = nullptr;
        }
        
        // Reset transition state
        s_drawingTransition = false;
        s_transitionStartState = nullptr;
        event->accept();
        return;
    }
    
    // Update model position after dragging
    if (!m_resizing && !s_drawingTransition)
    {
        m_model->setPosition(pos());
    }
    
    QGraphicsObject::mouseReleaseEvent(event);
}

void StateView::showTitleInputDialog()
{
    bool ok;
    QString newText = QInputDialog::getText(
        nullptr,
        "Edit State Name",
        "Enter new name:",
        QLineEdit::Normal,
        m_label,
        &ok);
    
    if (ok && !newText.isEmpty() && newText != m_label)
    {
        m_model->setLabel(newText);
    }
    
    update();
}

QPointF StateView::getNearestEdgePoint(const QPointF &targetPoint) const
{
    QRectF rect = sceneBoundingRect();
    QPointF center = rect.center();
    
    qreal dx = targetPoint.x() - center.x();
    qreal dy = targetPoint.y() - center.y();
    
    StateType type = m_model->stateType();
    
    if (type == StateType::Initial || type == StateType::Final)
    {
        // For circular states, calculate point on circle edge
        qreal distance = qSqrt(dx * dx + dy * dy);
        if (distance < 0.001)
            return center;
        
        qreal radius = (type == StateType::Initial) ? 10.0 : 15.0;
        return QPointF(center.x() + dx * radius / distance,
                       center.y() + dy * radius / distance);
    }
    
    // For normal states, calculate edge intersection
    if (qAbs(dx) > qAbs(dy))
    {
        if (dx > 0)
            return QPointF(rect.right(), center.y());
        else
            return QPointF(rect.left(), center.y());
    }
    else
    {
        if (dy > 0)
            return QPointF(center.x(), rect.bottom());
        else
            return QPointF(center.x(), rect.top());
    }
}

StateView *StateView::findStateAtPosition(const QPointF &scenePos) const
{
    if (!scene())
        return nullptr;
    
    QList<QGraphicsItem *> itemsAtPos = scene()->items(scenePos);
    
    for (QGraphicsItem *item : itemsAtPos)
    {
        StateView *stateView = dynamic_cast<StateView *>(item);
        if (stateView && stateView != this)
        {
            return stateView;
        }
    }
    
    return nullptr;
}
