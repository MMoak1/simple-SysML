#include "../../headers/views/connectionview.h"
#include "../../headers/models/connectionmodel.h" // Explicitly include full definition
#include "../../headers/models/partproperty.h"
#include "../../headers/models/blockdefinition.h" // For PartProperty owner/type access
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QtMath>
#include <QInputDialog>
#include <QLineEdit>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <QMessageBox> // For error dialogs

ConnectionView::ConnectionView(ConnectionModel *model, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(model), m_partProperty(nullptr), m_color(Qt::black)
{
    // Make connection selectable
    setFlags(ItemIsSelectable);
    
    if (m_model) {
        connect(m_model, &ConnectionModel::connectionChanged, this, &ConnectionView::updateConnection);
    }
}

ConnectionView::ConnectionView(PartProperty *partProperty, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_model(nullptr), m_partProperty(partProperty), m_color(Qt::black)
{
    setFlags(ItemIsSelectable);
    
    if (m_partProperty) {
        connect(m_partProperty, &PartProperty::multiplicityChanged, this, &ConnectionView::updateConnection);
        connect(m_partProperty, &PartProperty::nameChanged, this, &ConnectionView::updateConnection);
        connect(m_partProperty, &PartProperty::typeChanged, this, &ConnectionView::updateConnection);
        
        // Store pointers in QPointer for safe tracking
        m_ownerBlock = m_partProperty->owner();
        m_typeBlock = m_partProperty->type();
        
        // Connect to owner block signals if it exists
        if (m_ownerBlock) {
            connect(m_ownerBlock.data(), &BlockDefinition::positionChanged, this, &ConnectionView::updateConnection);
            connect(m_ownerBlock.data(), &BlockDefinition::sizeChanged, this, &ConnectionView::updateConnection);
            connect(m_ownerBlock.data(), &QObject::destroyed, this, &ConnectionView::onBlockDestroyed);
        }
        
        // Connect to type block signals if it exists
        if (m_typeBlock) {
            connect(m_typeBlock.data(), &BlockDefinition::positionChanged, this, &ConnectionView::updateConnection);
            connect(m_typeBlock.data(), &BlockDefinition::sizeChanged, this, &ConnectionView::updateConnection);
            connect(m_typeBlock.data(), &QObject::destroyed, this, &ConnectionView::onBlockDestroyed);
        }
        
        // Handle dynamic type changes - reconnect signals to new type
        connect(m_partProperty, &PartProperty::typeChanged, this, [this](BlockDefinition *newType) {
            // Disconnect from old type if still valid
            if (m_typeBlock) {
                disconnect(m_typeBlock.data(), nullptr, this, nullptr);
            }
            
            // Store and connect to new type
            m_typeBlock = newType;
            if (m_typeBlock) {
                connect(m_typeBlock.data(), &BlockDefinition::positionChanged, this, &ConnectionView::updateConnection);
                connect(m_typeBlock.data(), &BlockDefinition::sizeChanged, this, &ConnectionView::updateConnection);
                connect(m_typeBlock.data(), &QObject::destroyed, this, &ConnectionView::onBlockDestroyed);
            }
            updateConnection();
        });
    }
}

QRectF ConnectionView::boundingRect() const
{
    // Basic validation
    if (m_model && !m_model->isValid()) return QRectF();
    if (m_partProperty && (!m_partProperty->owner() || !m_partProperty->type())) return QRectF();
    if (!m_model && !m_partProperty) return QRectF();

    QPointF start = getStartPosition();
    QPointF end = getEndPosition();

    // Calculate bounding rect for orthogonal path with padding for arrow
    qreal padding = 15.0;
    qreal midX = (start.x() + end.x()) / 2.0;

    qreal left = qMin(qMin(start.x(), end.x()), midX) - padding;
    qreal top = qMin(start.y(), end.y()) - padding;
    qreal right = qMax(qMax(start.x(), end.x()), midX) + padding;
    qreal bottom = qMax(start.y(), end.y()) + padding;

    // Account for label text area - labels can extend beyond the path
    QString labelText = formatLabel();
    if (!labelText.isEmpty()) {
        QFont font;
        font.setPointSize(9);
        QFontMetrics fm(font);
        QRectF textRect = fm.boundingRect(labelText);
        
        // Calculate midpoint of path for label positioning
        QPainterPath path;
        path.moveTo(start);
        path.lineTo(midX, start.y());
        path.lineTo(midX, end.y());
        path.lineTo(end);
        
        qreal pathLength = path.length();
        if (pathLength >= 1.0) {
            QPointF midPoint = path.pointAtPercent(0.5);
            QRectF labelRect = textRect;
            labelRect.moveCenter(midPoint);
            labelRect.adjust(-8, -6, 8, 6);  // Add more padding for the background rect
            
            // Expand bounding rect to include label
            left = qMin(left, labelRect.left());
            top = qMin(top, labelRect.top());
            right = qMax(right, labelRect.right());
            bottom = qMax(bottom, labelRect.bottom());
        }
    }

    return QRectF(QPointF(left, top), QPointF(right, bottom));
}

QPainterPath ConnectionView::shape() const
{
    // Basic validation
    if (m_model && !m_model->isValid()) return QPainterPath();
    if (m_partProperty && (!m_partProperty->owner() || !m_partProperty->type())) return QPainterPath();
    if (!m_model && !m_partProperty) return QPainterPath();

    QPointF start = getStartPosition();
    QPointF end = getEndPosition();
    
    QPainterPath linePath = calculateOrthogonalPath(start, end);
    
    // Create a stroker to make a wider hit area
    QPainterPathStroker stroker;
    stroker.setWidth(12.0);  // Wide hit area for easy clicking
    
    return stroker.createStroke(linePath);
}

void ConnectionView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Basic validation
    if (m_model && !m_model->isValid()) return;
    if (m_partProperty && (!m_partProperty->owner() || !m_partProperty->type())) return;
    if (!m_model && !m_partProperty) return;

    QPointF start = getStartPosition();
    QPointF end = getEndPosition();

    // Calculate orthogonal path
    QPainterPath path = calculateOrthogonalPath(start, end);

    // Draw the connection line
    painter->save();
    
    // Change color and thickness when selected
    QColor lineColor = getConnectionColor();
    qreal lineWidth = 2.0;
    
    if (isSelected())
    {
        lineColor = QColor(255, 200, 0);  // Yellow/orange when selected
        lineWidth = 3.0;
    }
    
    QPen pen(lineColor, lineWidth);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path);

    // Draw arrow head at end point
    QColor savedColor = m_color;
    if (isSelected())
    {
        m_color = lineColor;  // Temporarily change for arrow
    }
    drawArrowHead(painter, path, end);
    m_color = savedColor;
    
    // Draw label if present
    drawLabel(painter, path);

    painter->restore();
}

void ConnectionView::updateConnection()
{
    prepareGeometryChange();
    update();
}

void ConnectionView::onBlockDestroyed()
{
    // One of the connected blocks was destroyed
    // The QPointers (m_ownerBlock, m_typeBlock) will auto-null
    // Just trigger an update - paint() and boundingRect() already check for null
    prepareGeometryChange();
    update();
}

QPainterPath ConnectionView::calculateOrthogonalPath(const QPointF &start, const QPointF &end) const
{
    QPainterPath path;
    path.moveTo(start);

    // Simple 3-segment orthogonal routing
    // Start -> Mid1 -> Mid2 -> End

    qreal midX = (start.x() + end.x()) / 2.0;

    // First segment: horizontal from start
    path.lineTo(midX, start.y());

    // Second segment: vertical to end height
    path.lineTo(midX, end.y());

    // Third segment: horizontal to end
    path.lineTo(end);

    return path;
}

void ConnectionView::drawArrowHead(QPainter *painter, const QPainterPath &path, const QPointF &endPoint) const
{
    // Get direction of last segment
    qreal angle = calculateLastSegmentAngle(path);

    // Arrow dimensions
    qreal arrowSize = 10.0;

    // Calculate arrow points
    QPointF arrowP1 = endPoint + QPointF(
                                     sin(angle + M_PI / 3) * arrowSize,
                                     cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = endPoint + QPointF(
                                     sin(angle + M_PI - M_PI / 3) * arrowSize,
                                     cos(angle + M_PI - M_PI / 3) * arrowSize);

    // Draw filled arrow head
    QPolygonF arrowHead;
    arrowHead << endPoint << arrowP1 << arrowP2;
    painter->setBrush(m_color);
    painter->drawPolygon(arrowHead);
}

qreal ConnectionView::calculateLastSegmentAngle(const QPainterPath &path) const
{
    // Get the last two points to determine direction
    int elementCount = path.elementCount();
    if (elementCount < 2)
    {
        return 0.0;
    }

    QPainterPath::Element lastElement = path.elementAt(elementCount - 1);
    QPainterPath::Element secondLastElement = path.elementAt(elementCount - 2);

    QPointF lastPoint(lastElement.x, lastElement.y);
    QPointF secondLastPoint(secondLastElement.x, secondLastElement.y);

    // Calculate angle from second-last to last point
    qreal dx = lastPoint.x() - secondLastPoint.x();
    qreal dy = lastPoint.y() - secondLastPoint.y();

    return atan2(dy, dx);
}

void ConnectionView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    showLabelInputDialog();
}

void ConnectionView::showLabelInputDialog()
{
    QString currentLabel;
    QString currentMult;
    
    if (m_model) {
        currentLabel = m_model->label();
        currentMult = m_model->multiplicity();
    } else if (m_partProperty) {
        currentLabel = m_partProperty->name();
        currentMult = m_partProperty->multiplicity();
    } else {
        return;
    }
    
    bool ok;
    
    // First dialog: Label (free text)
    // Note: For PartProperty, "label" is "name".
    QString promptTtile = m_partProperty ? "Edit Part Name" : "Edit Connection Label";
    QString promptMsg = m_partProperty ? "Enter part name (unique in block):" : "Enter label:";
    
    QString newLabel = QInputDialog::getText(
        nullptr,
        promptTtile,
        promptMsg,
        QLineEdit::Normal,
        currentLabel,
        &ok);
    
    if (!ok)
        return;

    // Validation for PartProperty name uniqueness
    if (m_partProperty && newLabel != currentLabel) {
        if (m_partProperty->owner() && !m_partProperty->owner()->validatePartName(newLabel, m_partProperty)) {
             QMessageBox::warning(nullptr, "Invalid Name",
                QString("A part named '%1' already exists in %2.")
                    .arg(newLabel, m_partProperty->owner()->typeName()));
             return;
        }
    }
    
    // Second dialog: Multiplicity (dropdown with predefined options)
    QStringList multiplicityOptions;
    multiplicityOptions << "" << "1" << "0..1" << "*" << "1..*" << "0..*" << "2" << "3" << "4";
    
    int currentIndex = multiplicityOptions.indexOf(currentMult);
    if (currentIndex < 0) currentIndex = 0;  // Default to empty if not found
    
    QString newMult = QInputDialog::getItem(
        nullptr,
        "Edit Multiplicity",
        "Select multiplicity:",
        multiplicityOptions,
        currentIndex,
        false,  // Not editable (constrained to list)
        &ok);
    
    if (!ok)
        return;
    
    // Apply changes
    if (m_model) {
        m_model->setLabel(newLabel);
        m_model->setMultiplicity(newMult);
    } else if (m_partProperty) {
        m_partProperty->setName(newLabel);
        m_partProperty->setMultiplicity(newMult);
    }
}

QString ConnectionView::formatLabel() const
{
    QString label;
    QString mult;
    
    if (m_model) {
        label = m_model->label();
        mult = m_model->multiplicity();
    } else if (m_partProperty) {
        label = m_partProperty->name();
        mult = m_partProperty->multiplicity();
    } else {
        return QString();
    }
    
    if (label.isEmpty() && mult.isEmpty())
        return QString();
    
    // Format: [multiplicity] label  (PartProperty style: [4] wheels : Wheel)
    // For now we just show [mult] name
    // Optionally we could add type name like the tree view does.
    // The user said: "Should we also update the connection label display on the canvas to match? Yes"
    // So we should show: [mult] name : TypeName
    
    QString result;
    if (!mult.isEmpty())
    {
        result = "[" + mult + "]";
    }
    if (!label.isEmpty())
    {
        if (!result.isEmpty())
            result += " ";
        result += label;
    }
    
    if (m_partProperty && m_partProperty->type()) {
        result += " : " + m_partProperty->type()->typeName();
    }
    
    return result;
}

void ConnectionView::drawLabel(QPainter *painter, const QPainterPath &path) const
{
    QString labelText = formatLabel();
    if (labelText.isEmpty())
        return;
    
    // Find the midpoint of the path for label placement
    qreal pathLength = path.length();
    if (pathLength < 1.0)
        return;
    
    qreal midPercent = 0.5;
    QPointF midPoint = path.pointAtPercent(midPercent);
    
    // Draw label with background
    QFont font = painter->font();
    font.setPointSize(9);
    painter->setFont(font);
    
    QFontMetrics fm(font);
    QRectF textRect = fm.boundingRect(labelText);
    textRect.moveCenter(midPoint);
    textRect.adjust(-4, -2, 4, 2);  // Padding
    
    // Draw background rectangle
    painter->setBrush(QColor(255, 255, 255, 220));  // Semi-transparent white
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(textRect, 3, 3);
    
    // Draw text
    painter->setPen(Qt::black);
    painter->drawText(textRect, Qt::AlignCenter, labelText);
}

// Geometry Helpers
QPointF ConnectionView::getStartPosition() const {
    if (m_model) return m_model->startEdgePoint();
    if (m_partProperty && m_partProperty->owner() && m_partProperty->type()) {
        // Calculate edge point on owner towards type
        QPointF center = m_partProperty->owner()->position();
        QSizeF size = m_partProperty->owner()->size();
        QPointF targetCenter = m_partProperty->type()->position();
        
        qreal dx = targetCenter.x() - center.x();
        qreal dy = targetCenter.y() - center.y();

        if (qAbs(dx) < 0.001 && qAbs(dy) < 0.001) return center;

        qreal halfWidth = size.width() / 2.0;
        qreal halfHeight = size.height() / 2.0;

        if (qAbs(dx) > qAbs(dy)) {
            if (dx > 0) return QPointF(center.x() + halfWidth, center.y() + dy * halfWidth / dx);
            return QPointF(center.x() - halfWidth, center.y() - dy * halfWidth / dx);
        } else {
            if (dy > 0) return QPointF(center.x() + dx * halfHeight / dy, center.y() + halfHeight);
            return QPointF(center.x() - dx * halfHeight / dy, center.y() - halfHeight);
        }
    }
    return QPointF();
}

QPointF ConnectionView::getEndPosition() const {
    if (m_model) return m_model->endEdgePoint();
    if (m_partProperty && m_partProperty->owner() && m_partProperty->type()) {
        // Calculate edge point on type towards owner
        QPointF center = m_partProperty->type()->position();
        QSizeF size = m_partProperty->type()->size();
        QPointF targetCenter = m_partProperty->owner()->position();
        
        qreal dx = targetCenter.x() - center.x();
        qreal dy = targetCenter.y() - center.y();

        if (qAbs(dx) < 0.001 && qAbs(dy) < 0.001) return center;

        qreal halfWidth = size.width() / 2.0;
        qreal halfHeight = size.height() / 2.0;

        if (qAbs(dx) > qAbs(dy)) {
            if (dx > 0) return QPointF(center.x() + halfWidth, center.y() + dy * halfWidth / dx);
            return QPointF(center.x() - halfWidth, center.y() - dy * halfWidth / dx);
        } else {
            if (dy > 0) return QPointF(center.x() + dx * halfHeight / dy, center.y() + halfHeight);
            return QPointF(center.x() - dx * halfHeight / dy, center.y() - halfHeight);
        }
    }
    return QPointF();
}

QSizeF ConnectionView::getStartSize() const {
    if (m_model) return QSizeF(100, 80); // Fallback? ConnectionModel doesn't expose size easily
    if (m_partProperty && m_partProperty->owner()) return m_partProperty->owner()->size();
    return QSizeF(100, 80);
}

QSizeF ConnectionView::getEndSize() const {
    if (m_model) return QSizeF(100, 80);
    if (m_partProperty && m_partProperty->type()) return m_partProperty->type()->size();
    return QSizeF(100, 80);
}

QColor ConnectionView::getConnectionColor() const {
    if (m_model) return m_color;
    if (m_partProperty) return Qt::black; // PartProperty doesn't implement color yet?
    return Qt::black;
}