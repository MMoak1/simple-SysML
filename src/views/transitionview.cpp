#include "../../headers/views/transitionview.h"
#include "../../headers/models/transitionmodel.h"
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QtMath>
#include <QDebug>

TransitionView::TransitionView(TransitionModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model)
{
    setFlag(ItemIsSelectable, true);
    
    // Connect to model signals
    connect(model, &TransitionModel::transitionChanged, this, &TransitionView::onTransitionChanged);
    connect(model, &TransitionModel::labelChanged, this, &TransitionView::onLabelChanged);
    
    // Initial update
    updateFromModel();
}

QRectF TransitionView::boundingRect() const
{
    // Create bounding rect that encompasses the entire line plus some padding for the label
    qreal left = qMin(m_startPoint.x(), m_endPoint.x()) - 20;
    qreal top = qMin(m_startPoint.y(), m_endPoint.y()) - 20;
    qreal right = qMax(m_startPoint.x(), m_endPoint.x()) + 20;
    qreal bottom = qMax(m_startPoint.y(), m_endPoint.y()) + 20;
    
    return QRectF(left, top, right - left, bottom - top);
}

void TransitionView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    // Don't draw if points are too close
    qreal dx = m_endPoint.x() - m_startPoint.x();
    qreal dy = m_endPoint.y() - m_startPoint.y();
    qreal length = qSqrt(dx * dx + dy * dy);
    if (length < 1.0)
        return;
    
    // Draw the transition line
    QPen linePen(Qt::black, 2);
    if (isSelected())
    {
        linePen.setColor(Qt::blue);
        linePen.setWidth(3);
    }
    painter->setPen(linePen);
    painter->drawLine(m_startPoint, m_endPoint);
    
    // Draw arrowhead at the end
    drawArrowHead(painter, m_startPoint, m_endPoint);
    
    // Draw label if present
    if (!m_label.isEmpty())
    {
        QPointF midPoint((m_startPoint.x() + m_endPoint.x()) / 2,
                         (m_startPoint.y() + m_endPoint.y()) / 2);
        
        // Offset label slightly above the line
        QPointF labelPos = midPoint + QPointF(0, -10);
        
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPointSize(10);
        painter->setFont(font);
        
        QFontMetrics fm(font);
        QRectF textRect = fm.boundingRect(m_label);
        textRect.moveCenter(labelPos);
        
        // Draw background for label
        painter->setBrush(QColor(255, 255, 255, 200));
        painter->setPen(Qt::NoPen);
        painter->drawRect(textRect.adjusted(-2, -1, 2, 1));
        
        // Draw label text
        painter->setPen(Qt::black);
        painter->drawText(textRect, Qt::AlignCenter, m_label);
    }
}

void TransitionView::drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to)
{
    qreal arrowSize = 12.0;
    
    // Calculate direction
    qreal dx = to.x() - from.x();
    qreal dy = to.y() - from.y();
    qreal length = qSqrt(dx * dx + dy * dy);
    
    if (length < 0.001)
        return;
    
    // Normalize direction
    dx /= length;
    dy /= length;
    
    // Calculate arrow points
    QPointF arrowP1 = to - QPointF(dx * arrowSize - dy * arrowSize / 2,
                                    dy * arrowSize + dx * arrowSize / 2);
    QPointF arrowP2 = to - QPointF(dx * arrowSize + dy * arrowSize / 2,
                                    dy * arrowSize - dx * arrowSize / 2);
    
    // Draw filled arrowhead
    QPolygonF arrowHead;
    arrowHead << to << arrowP1 << arrowP2;
    
    painter->setBrush(isSelected() ? Qt::blue : Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(arrowHead);
}

void TransitionView::onTransitionChanged()
{
    prepareGeometryChange();
    updateFromModel();
    update();
}

void TransitionView::onLabelChanged(const QString &label)
{
    m_label = label;
    update();
}

void TransitionView::updateFromModel()
{
    m_startPoint = m_model->startEdgePoint();
    m_endPoint = m_model->endEdgePoint();
    m_label = m_model->label();
}

void TransitionView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        showLabelInputDialog();
        event->accept();
        return;
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

void TransitionView::showLabelInputDialog()
{
    bool ok;
    QString newLabel = QInputDialog::getText(
        nullptr,
        "Edit Transition Label",
        "Enter transition label (e.g., event / action):",
        QLineEdit::Normal,
        m_label,
        &ok);
    
    if (ok)
    {
        m_model->setLabel(newLabel);
    }
}
