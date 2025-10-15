# Connection Drawing Workflow

## User Interaction Flow

```mermaid
graph TD
    A[User hovers over Block A] --> B{User presses Ctrl+Click?}
    B -->|No| C[Normal block interaction]
    B -->|Yes| D[Start connection drawing]
    
    D --> E[Create temporary line from Block A]
    E --> F[Temporary line follows mouse cursor]
    
    F --> G{User releases mouse?}
    G -->|Over empty space| H[Cancel connection]
    G -->|Over same block| H
    G -->|Over different Block B| I[Validate connection]
    
    H --> J[Remove temporary line]
    J --> K[End]
    
    I --> L{Connection already exists?}
    L -->|Yes| M[Show message/ignore]
    L -->|No| N[Create permanent connection]
    
    M --> J
    N --> O[Calculate edge points]
    O --> P[Draw orthogonal line with arrow]
    P --> K
```

## State Transitions

```mermaid
stateDiagram-v2
    [*] --> Idle
    
    Idle --> DrawingConnection: Ctrl+Click on block
    
    DrawingConnection --> Idle: Release on empty space
    DrawingConnection --> Idle: Release on same block
    DrawingConnection --> ValidatingConnection: Release on different block
    
    ValidatingConnection --> Idle: Connection exists
    ValidatingConnection --> CreatingConnection: Valid new connection
    
    CreatingConnection --> Connected: Connection created
    Connected --> [*]
    
    note right of DrawingConnection
        Temporary line visible
        Following mouse cursor
    end note
    
    note right of Connected
        Permanent connection
        with orthogonal routing
        and arrow head
    end note
```

## Component Interaction Sequence

```mermaid
sequenceDiagram
    participant User
    participant BlockView
    participant TempLine as TemporaryConnectionLine
    participant Controller as ConnectionController
    participant Model as ConnectionModel
    participant View as ConnectionView
    participant Scene as QGraphicsScene
    
    User->>BlockView: Ctrl+Click on Block A
    BlockView->>BlockView: Detect Ctrl modifier
    BlockView->>TempLine: Create temporary line
    BlockView->>Scene: Add temporary line to scene
    BlockView->>Controller: emit connectionStarted(BlockA)
    
    loop Mouse moves
        User->>BlockView: Move mouse
        BlockView->>TempLine: updateEndPoint(mousePos)
        TempLine->>Scene: Update display
    end
    
    User->>BlockView: Release mouse over Block B
    BlockView->>BlockView: Find block at position
    
    alt Valid target block found
        BlockView->>Controller: emit connectionCompleted(BlockA, BlockB)
        Controller->>Controller: Validate connection
        
        alt Connection valid
            Controller->>Model: Create ConnectionModel(BlockA, BlockB)
            Model->>Model: Calculate edge points
            Controller->>View: Create ConnectionView(model)
            Controller->>Scene: Add connection view
            View->>Scene: Draw orthogonal line with arrow
        else Connection invalid
            Controller->>Controller: Log/ignore
        end
    else No valid target
        BlockView->>BlockView: Cancel connection
    end
    
    BlockView->>Scene: Remove temporary line
    BlockView->>TempLine: Delete temporary line
```

## Connection Drawing States

### State 1: Idle
```
┌─────────────┐     ┌─────────────┐
│   Block A   │     │   Block B   │
│             │     │             │
└─────────────┘     └─────────────┘

User can:
- Move blocks
- Resize blocks
- Edit titles
- Ctrl+Click to start connection
```

### State 2: Drawing Connection
```
┌─────────────┐     ┌─────────────┐
│   Block A   │     │   Block B   │
│      ●------│-----│----→ ✕      │  ← Mouse cursor
└─────────────┘     └─────────────┘
      ↑
   Start point
   (edge of Block A)

Temporary dashed line follows cursor
User can:
- Move mouse to target
- Release to complete
- Release on empty space to cancel
```

### State 3: Connection Complete
```
┌─────────────┐     ┌─────────────┐
│   Block A   ├──┐  │   Block B   │
│             │  │  │             │
└─────────────┘  │  └─────────────┘
                 │         ▲
                 └─────────┤
                           │
                      Arrow head

Permanent orthogonal line with arrow
Automatically updates when blocks move
```

## Edge Point Calculation

```
Block Rectangle with Edge Points:

        Top Edge (center.x, rect.top)
              ●
              │
    ┌─────────┼─────────┐
    │         │         │
Left│    ●────┼────●    │Right
Edge│  (center)         │Edge
    │                   │
    └───────────────────┘
              │
              ●
        Bottom Edge
```

### Algorithm:
1. Calculate vector from block center to target point
2. Determine if horizontal or vertical component is larger
3. If horizontal dominant:
   - Use right edge if target is right of center
   - Use left edge if target is left of center
4. If vertical dominant:
   - Use bottom edge if target is below center
   - Use top edge if target is above center

## Orthogonal Routing

### 3-Segment Routing (Simple)
```
Start Block              End Block
┌─────────┐             ┌─────────┐
│         ├──→          │         │
└─────────┘  │          └─────────┘
             │               ▲
             │               │
             └───────────────┘
             
Segments:
1. Horizontal from start edge
2. Vertical to end height
3. Horizontal to end edge
```

### Example Scenarios:

#### Scenario A: Blocks side-by-side
```
┌─────┐         ┌─────┐
│  A  ├────────→│  B  │
└─────┘         └─────┘
```

#### Scenario B: Blocks vertically aligned
```
┌─────┐
│  A  │
└──┬──┘
   │
   │
   ↓
┌─────┐
│  B  │
└─────┘
```

#### Scenario C: Diagonal arrangement
```
┌─────┐
│  A  ├──┐
└─────┘  │
         │
         ↓
      ┌─────┐
      │  B  │
      └─────┘
```

## Arrow Head Drawing

```
Direction calculation:
                ↑ Last segment direction
                │
                │
         ┌──────┴──────┐
         │      ●       │  ← End point
         │    ╱   ╲    │
         │  ╱       ╲  │
         │●           ●│  ← Arrow points
         └─────────────┘

Arrow head is filled triangle:
- Point 1: End point
- Point 2: End point + offset at angle + 30°
- Point 3: End point + offset at angle - 30°
```

## Block Movement Updates

```mermaid
sequenceDiagram
    participant User
    participant BlockView
    participant BlockModel
    participant ConnectionModel
    participant ConnectionView
    
    User->>BlockView: Drag Block A
    BlockView->>BlockModel: setPosition(newPos)
    BlockModel->>BlockModel: Update position
    BlockModel->>ConnectionModel: emit positionChanged()
    
    loop For each connection
        ConnectionModel->>ConnectionModel: Recalculate edge points
        ConnectionModel->>ConnectionView: emit connectionChanged()
        ConnectionView->>ConnectionView: prepareGeometryChange()
        ConnectionView->>ConnectionView: Recalculate orthogonal path
        ConnectionView->>ConnectionView: update() - trigger repaint
    end
```

## Key Features Summary

1. **Ctrl+Click Initiation**: Intuitive gesture to start connection
2. **Visual Feedback**: Temporary line shows what you're doing
3. **Smart Cancellation**: Release on empty space = no connection
4. **Edge Attachment**: Connections attach to nearest block edge
5. **Orthogonal Routing**: Professional right-angled lines
6. **Directional Arrows**: Clear indication of flow direction
7. **Auto-Update**: Connections follow blocks when moved
8. **Duplicate Prevention**: Can't create same connection twice
9. **Bidirectional Support**: A→B and B→A are different connections

## Implementation Priority

### Phase 1 (Core Functionality)
- Ctrl+Click detection
- Temporary line creation
- Basic connection completion

### Phase 2 (Visual Enhancement)
- Edge point calculation
- Orthogonal routing
- Arrow heads

### Phase 3 (Polish)
- Auto-updates on block move
- Connection validation
- Error handling