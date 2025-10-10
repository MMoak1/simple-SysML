# Connection Drawing System Redesign Plan

## Overview
Redesign the connection drawing system to support:
- Ctrl+Click to initiate connection drawing
- Visual feedback with temporary line following mouse cursor
- Connections attach to nearest edge points of blocks
- Orthogonal (right-angled) line routing
- Directional connections with arrow heads
- Automatic line updates when blocks move
- Connection cancellation if not dropped on valid block

## Current Problems

### 1. No Visual Feedback
- User clicks on a block but sees no indication that connection drawing has started
- No temporary line showing where the connection will go
- Confusing user experience

### 2. Connection Mode Requirement
- Requires toggling a separate "connection mode" before drawing
- Not intuitive - users expect direct interaction
- Extra step in workflow

### 3. Center-to-Center Connections
- Lines always connect to block centers
- Doesn't look professional
- Lines can overlap with block content

### 4. Straight Lines Only
- Simple straight lines look basic
- No routing around obstacles
- Orthogonal lines are more professional for diagrams

### 5. No Direction Indication
- Can't tell which block is source vs target
- Important for flow diagrams and logic

## New Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Connection System                         │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐      ┌──────────────┐                     │
│  │  BlockView   │      │TemporaryLine │                     │
│  │              │      │   (new)      │                     │
│  │ - Ctrl+Click │─────▶│ - Visual     │                     │
│  │   detection  │      │   feedback   │                     │
│  │ - Edge point │      │ - Follows    │                     │
│  │   calculation│      │   cursor     │                     │
│  └──────────────┘      └──────────────┘                     │
│         │                      │                             │
│         │                      │                             │
│         ▼                      ▼                             │
│  ┌─────────────────────────────────────┐                    │
│  │    ConnectionController             │                    │
│  │  - Manages connection lifecycle     │                    │
│  │  - Creates/destroys connections     │                    │
│  │  - Validates connection targets     │                    │
│  └─────────────────────────────────────┘                    │
│         │                                                    │
│         ▼                                                    │
│  ┌──────────────┐      ┌──────────────┐                    │
│  │ConnectionModel│     │ConnectionView│                    │
│  │ - Source block│     │ - Orthogonal │                    │
│  │ - Target block│     │   routing    │                    │
│  │ - Direction   │     │ - Arrow head │                    │
│  │ - Edge points │     │ - Updates on │                    │
│  │               │     │   block move │                    │
│  └──────────────┘      └──────────────┘                    │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Detailed Design

### 1. Temporary Connection Line (New Component)

**Purpose**: Provide visual feedback during connection drawing

**File**: `headers/views/temporaryconnectionline.h`, `src/views/temporaryconnectionline.cpp`

**Class**: `TemporaryConnectionLine : public QGraphicsLineItem`

**Responsibilities**:
- Display a temporary line from start block to current mouse position
- Update position as mouse moves
- Use dashed line style to indicate temporary state
- Automatically removed when connection completes or cancels

**Key Methods**:
```cpp
class TemporaryConnectionLine : public QGraphicsLineItem {
public:
    TemporaryConnectionLine(const QPointF &startPoint, QGraphicsItem *parent = nullptr);
    void updateEndPoint(const QPointF &endPoint);
    void setStartPoint(const QPointF &startPoint);
};
```

### 2. BlockView Modifications

**File**: `headers/views/blockview.h`, `src/views/blockview.cpp`

**Changes**:

#### A. Ctrl+Click Detection
```cpp
void BlockView::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // Check for Ctrl+Click to start connection
    if (event->button() == Qt::LeftButton && 
        event->modifiers() & Qt::ControlModifier) {
        startConnectionDrawing(event->scenePos());
        event->accept();
        return;
    }
    // ... existing code for resize, title edit, etc.
}
```

#### B. Edge Point Calculation
```cpp
// Calculate nearest edge point for connection attachment
QPointF BlockView::getNearestEdgePoint(const QPointF &targetPoint) const {
    QRectF rect = sceneBoundingRect();
    QPointF center = rect.center();
    
    // Calculate intersection with rectangle edges
    // Return point on edge closest to targetPoint
    // Consider all 4 edges and return nearest
}
```

#### C. Connection Drawing State
```cpp
private:
    bool m_drawingConnection = false;
    QPointF m_connectionStartPoint;
    TemporaryConnectionLine *m_tempLine = nullptr;
```

#### D. Mouse Move Tracking
```cpp
void BlockView::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_drawingConnection && m_tempLine) {
        // Update temporary line to follow cursor
        m_tempLine->updateEndPoint(event->scenePos());
        event->accept();
        return;
    }
    // ... existing code
}
```

#### E. Connection Completion
```cpp
void BlockView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_drawingConnection) {
        // Find block under cursor
        BlockView *targetBlock = findBlockAtPosition(event->scenePos());
        
        if (targetBlock && targetBlock != this) {
            // Valid connection - emit signal
            emit connectionCompleted(this, targetBlock);
        } else {
            // Invalid - just remove temporary line
            qDebug() << "Connection cancelled - no valid target";
        }
        
        // Clean up temporary line
        if (m_tempLine) {
            scene()->removeItem(m_tempLine);
            delete m_tempLine;
            m_tempLine = nullptr;
        }
        
        m_drawingConnection = false;
        event->accept();
        return;
    }
    // ... existing code
}
```

### 3. ConnectionController Modifications

**File**: `headers/controllers/connectioncontroller.h`, `src/controllers/connectioncontroller.cpp`

**Changes**:

#### A. Remove Connection Mode
- Remove `m_connectionMode` flag
- Remove `setConnectionMode()` method
- Connections now work via Ctrl+Click directly

#### B. Simplified Connection Creation
```cpp
void ConnectionController::onConnectionCompleted(BlockView *startBlock, BlockView *endBlock) {
    // No mode check needed anymore
    
    if (!startBlock || !endBlock) {
        return;
    }
    
    // Check if connection already exists (directional check)
    if (connectionExists(startBlock, endBlock)) {
        qDebug() << "Connection already exists";
        return;
    }
    
    // Create directional connection
    BlockModel *startModel = startBlock->model();
    BlockModel *endModel = endBlock->model();
    
    if (startModel && endModel) {
        ConnectionModel *connection = new ConnectionModel(
            startModel, endModel, this);
        ConnectionView *connectionView = new ConnectionView(
            connection, nullptr);
        
        m_connections.append(connection);
        m_connectionViews[connection] = connectionView;
        m_scene->addItem(connectionView);
    }
}
```

#### C. Directional Connection Check
```cpp
bool ConnectionController::connectionExists(BlockView *start, BlockView *end) {
    // Only check start->end direction (not bidirectional)
    for (ConnectionModel *connection : m_connections) {
        if (connection->startBlock() == start->model() && 
            connection->endBlock() == end->model()) {
            return true;
        }
    }
    return false;
}
```

### 4. ConnectionModel Modifications

**File**: `headers/models/connectionmodel.h`, `src/models/connectionmodel.cpp`

**Changes**:

#### A. Edge Point Storage
```cpp
class ConnectionModel : public QObject {
    // ... existing code ...
    
    QPointF startEdgePoint() const;
    QPointF endEdgePoint() const;
    
private:
    void updateEdgePoints();
    QPointF m_startEdgePoint;
    QPointF m_endEdgePoint;
};
```

#### B. Edge Point Calculation
```cpp
void ConnectionModel::updateEdgePoints() {
    if (!m_startBlock || !m_endBlock) return;
    
    // Get block rectangles
    QRectF startRect = getBlockRect(m_startBlock);
    QRectF endRect = getBlockRect(m_endBlock);
    
    // Calculate nearest edge points
    m_startEdgePoint = calculateNearestEdgePoint(
        startRect, endRect.center());
    m_endEdgePoint = calculateNearestEdgePoint(
        endRect, startRect.center());
    
    emit connectionChanged();
}

QPointF ConnectionModel::calculateNearestEdgePoint(
    const QRectF &rect, const QPointF &target) {
    
    QPointF center = rect.center();
    
    // Calculate intersection with rectangle edges
    // based on direction from center to target
    
    qreal dx = target.x() - center.x();
    qreal dy = target.y() - center.y();
    
    // Determine which edge to use based on angle
    if (qAbs(dx) > qAbs(dy)) {
        // Left or right edge
        if (dx > 0) {
            // Right edge
            return QPointF(rect.right(), center.y());
        } else {
            // Left edge
            return QPointF(rect.left(), center.y());
        }
    } else {
        // Top or bottom edge
        if (dy > 0) {
            // Bottom edge
            return QPointF(center.x(), rect.bottom());
        } else {
            // Top edge
            return QPointF(center.x(), rect.top());
        }
    }
}
```

### 5. ConnectionView Modifications

**File**: `headers/views/connectionview.h`, `src/views/connectionview.cpp`

**Changes**:

#### A. Orthogonal Line Drawing
```cpp
void ConnectionView::paint(QPainter *painter, 
    const QStyleOptionGraphicsItem *option, QWidget *widget) {
    
    if (!m_model || !m_model->isValid()) {
        return;
    }
    
    QPointF start = m_model->startEdgePoint();
    QPointF end = m_model->endEdgePoint();
    
    // Calculate orthogonal path
    QPainterPath path = calculateOrthogonalPath(start, end);
    
    // Draw the path
    painter->save();
    QPen pen(m_color, 2.0);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path);
    
    // Draw arrow head at end point
    drawArrowHead(painter, path, end);
    
    painter->restore();
}
```

#### B. Orthogonal Path Algorithm
```cpp
QPainterPath ConnectionView::calculateOrthogonalPath(
    const QPointF &start, const QPointF &end) {
    
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
```

#### C. Arrow Head Drawing
```cpp
void ConnectionView::drawArrowHead(QPainter *painter, 
    const QPainterPath &path, const QPointF &endPoint) {
    
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
```

#### D. Bounding Rect Update
```cpp
QRectF ConnectionView::boundingRect() const {
    if (!m_model || !m_model->isValid()) {
        return QRectF();
    }
    
    QPointF start = m_model->startEdgePoint();
    QPointF end = m_model->endEdgePoint();
    
    // Calculate bounding rect for orthogonal path
    qreal padding = 15.0; // Extra space for arrow head
    qreal midX = (start.x() + end.x()) / 2.0;
    
    qreal left = qMin(qMin(start.x(), end.x()), midX) - padding;
    qreal top = qMin(start.y(), end.y()) - padding;
    qreal right = qMax(qMax(start.x(), end.x()), midX) + padding;
    qreal bottom = qMax(start.y(), end.y()) + padding;
    
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}
```

## Implementation Sequence

### Phase 1: Basic Infrastructure
1. Create `TemporaryConnectionLine` class
2. Add Ctrl+Click detection in `BlockView`
3. Implement temporary line creation and tracking
4. Test: User can Ctrl+Click and see temporary line

### Phase 2: Connection Completion
5. Implement target block detection on mouse release
6. Add connection validation logic
7. Remove temporary line on completion/cancellation
8. Test: Connections created only when dropped on valid blocks

### Phase 3: Edge Point Calculation
9. Implement edge point calculation in `BlockView`
10. Update `ConnectionModel` to store edge points
11. Connect edge point updates to block position changes
12. Test: Connections attach to block edges

### Phase 4: Orthogonal Routing
13. Implement orthogonal path calculation in `ConnectionView`
14. Update bounding rect calculation
15. Test: Connections display as right-angled lines

### Phase 5: Directional Arrows
16. Implement arrow head drawing
17. Calculate proper arrow orientation
18. Test: Arrows point in correct direction

### Phase 6: Polish & Testing
19. Remove connection mode toggle from UI
20. Update connection tracking on block moves
21. Test complete workflow end-to-end
22. Fix any edge cases or bugs

## Testing Checklist

- [ ] Ctrl+Click on block starts connection drawing
- [ ] Temporary line appears and follows cursor
- [ ] Temporary line disappears if released on empty space
- [ ] Connection created when released on different block
- [ ] Connection NOT created when released on same block
- [ ] Connections attach to nearest edge of blocks
- [ ] Connections display as orthogonal (right-angled) lines
- [ ] Arrow heads point from source to target
- [ ] Connections update when blocks are moved
- [ ] Connections update when blocks are resized
- [ ] Multiple connections can exist between different blocks
- [ ] Duplicate connections (same source->target) are prevented
- [ ] Reverse connections (A->B and B->A) are both allowed
- [ ] Connections are removed when blocks are deleted
- [ ] No connection mode toggle needed

## Files to Modify

### New Files
- `headers/views/temporaryconnectionline.h`
- `src/views/temporaryconnectionline.cpp`

### Modified Files
- `headers/views/blockview.h`
- `src/views/blockview.cpp`
- `headers/controllers/connectioncontroller.h`
- `src/controllers/connectioncontroller.cpp`
- `headers/models/connectionmodel.h`
- `src/models/connectionmodel.cpp`
- `headers/views/connectionview.h`
- `src/views/connectionview.cpp`

### Files to Update (UI)
- Any menu/toolbar files that reference connection mode toggle

## Key Algorithms

### 1. Nearest Edge Point Calculation
```
Given: Block rectangle, Target point
1. Calculate vector from block center to target
2. Determine dominant direction (horizontal vs vertical)
3. If horizontal dominant:
   - Return right edge if target is right of center
   - Return left edge if target is left of center
4. If vertical dominant:
   - Return bottom edge if target is below center
   - Return top edge if target is above center
5. Return point on edge at center height/width
```

### 2. Orthogonal Path Routing (3-Segment)
```
Given: Start point, End point
1. Calculate midpoint X: (start.x + end.x) / 2
2. Create path:
   - Move to start point
   - Line to (midX, start.y) - horizontal segment
   - Line to (midX, end.y) - vertical segment
   - Line to end point - horizontal segment
3. Return path
```

### 3. Arrow Head Orientation
```
Given: Path, End point
1. Get last segment of path
2. Calculate angle of last segment
3. Create two points offset from end point:
   - Point 1: angle + 30 degrees
   - Point 2: angle - 30 degrees
4. Draw filled triangle: end, point1, point2
```

## Benefits of New Design

1. **Intuitive Interaction**: Ctrl+Click is standard for special actions
2. **Visual Feedback**: Users see exactly what they're doing
3. **Professional Appearance**: Orthogonal lines with arrows look polished
4. **Clear Direction**: Arrow heads show data/control flow
5. **Smart Attachment**: Edge points prevent line overlap with content
6. **Automatic Updates**: Connections track block movements
7. **Simplified Workflow**: No mode switching required

## Migration Notes

- Existing connections will need to be updated to use edge points
- Connection mode toggle can be removed from UI
- Any saved connection data may need format update
- Consider adding connection deletion (right-click menu)