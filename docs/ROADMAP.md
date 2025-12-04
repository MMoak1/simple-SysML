# Future Implementation Roadmap

This document outlines the planned features to make the SysML modeling tool production-ready.

---

## 1. Model Execution / Simulation

**Goal**: Allow users to "run" their state machines and see them execute in real-time.

### Features
- **Play/Pause/Step controls** in the toolbar
- **Current state highlighting** - visually show which state is active
- **Transition animation** - animate transitions as they fire
- **Event injection** - allow users to trigger events/signals manually
- **Execution log** - show a console/log of state changes and transitions

### Implementation Notes
```
New files needed:
- headers/controllers/simulationcontroller.h
- src/controllers/simulationcontroller.cpp
- headers/views/simulationtoolbar.h  
- src/views/simulationtoolbar.cpp

SimulationController will:
- Track current state for each state machine
- Process events and determine valid transitions
- Emit signals when states change
- Support step-by-step or continuous execution
```

---

## 2. Explicit Hierarchy (Block Composition)

**Goal**: Make parent-child block relationships explicit in SysML style.

### Features
- **Containment relationships** - blocks can contain other blocks
- **Part properties** - blocks define parts (instances of other blocks)
- **Visual nesting** - show contained blocks inside parent blocks
- **Port connections** - connect ports between blocks

### Implementation Notes
```
Modify BlockModel to add:
- QList<BlockModel*> m_parts;  // contained blocks
- BlockModel* m_parent;        // owning block
- addPart(), removePart()

Update HierarchyTreeView to show:
- Block containment (not just connection-based hierarchy)
- Drag-drop to reparent blocks

Consider adding:
- headers/models/portmodel.h  // for block ports
- headers/views/portview.h
```

---

## 3. Signals and Triggers for Transitions

**Goal**: Make transitions fire based on actual signals, not just connections.

### Features
- **Signal definitions** - define signals at the block level
- **Trigger conditions** - transitions specify what signal triggers them
- **Guard conditions** - boolean expressions that must be true
- **Actions** - code/behavior that executes on transition

### Implementation Notes
```
New files:
- headers/models/signalmodel.h
- src/models/signalmodel.cpp

Modify TransitionModel to add:
- QString m_trigger;      // signal name that triggers this
- QString m_guard;        // guard condition expression  
- QString m_action;       // action to perform

Modify BlockModel to add:
- QList<SignalModel*> m_signals;  // signals this block can send/receive

UI changes:
- Signal palette in block menu
- Transition properties dialog (trigger, guard, action)
```

---

## 4. Save/Load Models (JSON Format)

**Goal**: Persist models to disk and share with others.

### File Format: `.sysml.json`

Using JSON is practical and readable. Example structure:

```json
{
  "version": "1.0",
  "name": "MyModel",
  "blocks": [
    {
      "id": "uuid-1",
      "label": "Controller",
      "color": "#ff0000",
      "position": { "x": 100, "y": 50 },
      "size": { "width": 120, "height": 80 },
      "stateMachine": {
        "states": [
          {
            "id": "state-1",
            "label": "Idle",
            "type": "Normal",
            "position": { "x": 0, "y": 0 }
          }
        ],
        "transitions": [
          {
            "id": "trans-1",
            "from": "state-1",
            "to": "state-2",
            "trigger": "start",
            "guard": "",
            "action": ""
          }
        ]
      }
    }
  ],
  "connections": [
    {
      "id": "conn-1",
      "from": "uuid-1",
      "to": "uuid-2"
    }
  ]
}
```

### Implementation Notes
```
New files:
- headers/io/modelserializer.h
- src/io/modelserializer.cpp

ModelSerializer will have:
- static void saveToFile(const QString &path, QList<BlockModel*> blocks, QList<ConnectionModel*> connections);
- static bool loadFromFile(const QString &path, ...);

Use Qt's QJsonDocument for parsing.

Add to MainWindow:
- File > Save (Ctrl+S)
- File > Save As
- File > Open (Ctrl+O)
- Recent files menu
```

---

## Priority Order

| Priority | Feature | Complexity | User Value |
|----------|---------|------------|------------|
| 1 | **Save/Load (JSON)** | Medium | High - essential for any real use |
| 2 | **Signals on Transitions** | Medium | High - makes state machines meaningful |
| 3 | **Model Execution** | High | High - brings models to life |
| 4 | **Explicit Hierarchy** | Medium | Medium - better organization |

---

## Technical Debt to Address

- [ ] Add undo/redo support (QUndoStack)
- [ ] Add copy/paste for blocks and states
- [ ] Add keyboard shortcuts (Delete to remove, etc.)
- [ ] Add zoom controls for the canvas
- [ ] Add grid snapping for alignment
- [ ] Error validation (orphan states, missing initial state, etc.)

---

## Notes on .mdzip Format

MagicDraw's `.mdzip` format is a proprietary ZIP archive containing XMI (XML Metadata Interchange) files. 
Implementing full compatibility would be complex and might have licensing concerns.

**Recommendation**: Use a simple JSON format (`.sysml.json`) that is:
- Human-readable
- Easy to version control (git-friendly)
- Easy to parse and generate
- Portable across platforms

If interoperability with MagicDraw is needed later, consider adding XMI export as a separate feature.
