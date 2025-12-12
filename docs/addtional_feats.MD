## Additional Features

---

## 0. Type/Instance Block Relationships

### Problem Statement

Right now when we clone a block (drag from hierarchy), we create a **full deep copy** of the BlockDefinition, including its state machine. This means:
- Changes to the original type's state machine don't affect clones
- Each "copy" is actually a completely independent type
- There's no concept of "this block is an instance of that type"

### Current Architecture

```
Palette Drop → Creates NEW BlockDefinition (unique type)
Hierarchy Drag → clone() → Creates ANOTHER NEW BlockDefinition (independent copy)

BlockDefinition "Car1" (type)
   ├── stateMachine (copy 1)
   └── partProperties...

BlockDefinition "Car2" (cloned, independent type)
   ├── stateMachine (copy 2 - NOT linked to original)
   └── partProperties...
```

### Desired Behavior (SysML-Compliant)

In SysML:
- **Block Definition Diagram (BDD)** shows **types** (classes)
- **Internal Block Diagram (IBD)** shows **instances** of those types
- A single type can have multiple instances (parts)

### Proposed Type/Instance Model

```
                        ┌─────────────────────────────────────────┐
                        │         BlockType (NEW)                 │
                        │  - typeName: "Car"                      │
                        │  - stateMachineDefinition: SM*          │
                        │  - color, defaultSize (visual template) │
                        └─────────────────────────────────────────┘
                                        │
                        ┌───────────────┼───────────────┐
                        │               │               │
                        ▼               ▼               ▼
              ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
              │  BlockInstance  │ │  BlockInstance  │ │  BlockInstance  │
              │  "myCar"        │ │  "yourCar"      │ │  "theirCar"     │
              │  type → Car     │ │  type → Car     │ │  type → Car     │
              │  stateMachine   │ │  stateMachine   │ │  stateMachine   │
              │  (runtime copy)│  │  (runtime copy) │ │  (runtime copy) │
              │  position: ...  │ │  position: ...  │ │  position: ...  │
              └─────────────────┘ └─────────────────┘ └─────────────────┘
```

### Key Concepts

| Concept | Description |
|---------|-------------|
| **BlockType** | The definition/class (e.g., "Wheel"). Defines structure and state machine template. Edited in BDD. |
| **BlockInstance** | A usage of a type (e.g., "frontLeftWheel"). Has its own runtime state. Shown in BDD and IBD. |
| **State Machine Template** | The structure (states, transitions) defined at the type level. |
| **State Machine Instance** | A runtime copy with its own current state. Each instance has one. |

### Implementation Options

#### Option A: Introduce BlockType + BlockInstance Classes

Most SysML-compliant approach, but needs significant refactoring:

```cpp
class BlockType : public QObject {
    QString m_typeName;
    StateMachineModel *m_stateMachineDefinition;  // Template
    QList<PartProperty*> m_partProperties;         // Composition structure
    // Visual defaults for instances
    QColor m_defaultColor;
    QSizeF m_defaultSize;
};

class BlockInstance : public QObject {
    QString m_instanceName;      // e.g., "myCar"
    BlockType *m_type;           // Reference to type (NOT owned)
    StateMachineModel *m_stateMachineInstance;  // Runtime copy
    QPointF m_position;          // Instance-specific position
    // Could override color/size per instance if needed
};
```

**Pros:**
- Proper SysML semantics
- Clear separation of concerns
- Type changes propagate to all instances (for structure)

**Cons:**
- Significant refactor
- Need to update all controllers, views, serialization

#### Option B: Shared Type Reference (Simpler)

Keep current BlockDefinition but add a "typeRef" pointer:

```cpp
class BlockDefinition : public QObject {
    // Existing...
    QString m_typeName;
    StateMachineModel *m_stateMachine;
    
    // NEW: Type reference
    BlockDefinition *m_typeRef = nullptr;  // Points to original type if this is an instance
    
    bool isInstance() const { return m_typeRef != nullptr; }
    BlockDefinition* getType() const { return m_typeRef ? m_typeRef : this; }
    
    // For instances, create runtime state machine from type's template
    StateMachineModel* getRuntimeStateMachine();
};
```

**Pros:**
- Minimal changes
- Backward compatible with current JSON format

**Cons:**
- Less clean semantically
- Type deletion handling is tricky

#### Option C: Shared StateMachineModel Definition (Middle Ground)

Keep BlockDefinition structure but share state machine definitions:

```cpp
// Types own and define the state machine structure
class BlockDefinition {
    StateMachineModel *m_stateMachineDefinition;  // Structure template
    StateMachineModel *m_stateMachineInstance;    // Runtime state (null for pure types)
    
    bool isType() const { return m_stateMachineInstance == nullptr; }
    bool isInstance() const { return m_stateMachineInstance != nullptr; }
};
```

### Recommended Approach: Option A (Phased)

Long-term, Option A is the cleanest. But we can start with a hybrid:

**Phase 1: Palette creates Types, Hierarchy creates Instances**
1. Dropping from palette → Creates a new `BlockDefinition` (type)
2. Dragging from hierarchy → Creates a `BlockInstance` pointing to that type
3. Type modifications update all instances (for structure)
4. Instance modifications are local (position, runtime state)

**Phase 2: Separate Views**
1. BDD shows types with instances listed underneath
2. IBD shows specific instances and their connections
3. Clicking on instance shows its runtime state machine

**Phase 3: State Machine Synchronization**
1. Type's state machine template = master definition
2. When type's SM is edited, instances get updated structure
3. Instance runtime state is preserved when possible

### UI/UX Changes Needed

1. **Palette behavior**: "New Type from Template" → prompts for type name
2. **Hierarchy behavior**: Drag = create instance of selected type
3. **Visual distinction**: Types have solid border, Instances have dashed border?
4. **Naming**: Types show "«block» Car", Instances show "myCar : Car"
5. **Delete behavior**: 
   - Delete type → Warn if instances exist, cascade delete instances
   - Delete instance → Just removes that instance

### Files to Create/Modify

| File | Change |
|------|--------|
| `headers/models/blocktype.h` | NEW - Type definition |
| `src/models/blocktype.cpp` | NEW - Type implementation |
| `headers/models/blockinstance.h` | NEW - Instance class |
| `src/models/blockinstance.cpp` | NEW - Instance implementation |
| `headers/views/blocktypeview.h` | NEW or modify BlockDefinitionView |
| `headers/views/blockinstanceview.h` | NEW - Different visual style |
| `src/controllers/dropcontroller.cpp` | MODIFY - Create types vs instances |
| `src/controllers/hierarchycontroller.cpp` | MODIFY - Display types and instances |
| `src/mainwindow.cpp` | MODIFY - Handle type/instance creation |
| `src/io/modelserializer.cpp` | MODIFY - Serialize type references |

### State Machine Synchronization Detail

When the type's state machine is modified:

```cpp
// In BlockType or wherever state machine editing happens
void BlockType::onStateMachineStructureChanged() {
    // Notify all instances
    emit stateMachineDefinitionChanged();
    
    // Instances connect to this signal and update their local copy
    // preserving current runtime state if possible
}

// In BlockInstance
void BlockInstance::onTypeStateMachineChanged() {
    if (m_stateMachineInstance) {
        // Get current state
        StateModel* currentState = m_stateMachineInstance->currentState();
        QString currentStateName = currentState ? currentState->label() : "";
        
        // Re-clone from type
        delete m_stateMachineInstance;
        m_stateMachineInstance = m_type->stateMachineDefinition()->clone();
        
        // Try to restore current state if it still exists
        if (!currentStateName.isEmpty()) {
            StateModel* restoredState = m_stateMachineInstance->findStateByLabel(currentStateName);
            if (restoredState) {
                m_stateMachineInstance->setCurrentState(restoredState);
            }
        }
    }
}
```

---

some more features that we can add are:
- 
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


## notes
This will not be easy, to make a simulation actullay run 
we proably need IBDs and actvitiy diagrams if we want to make it  
true to real life sysml.

so those will need to be seperate features. 

## signals and transitions

right now we have a state machine that has a list of states and a list of transitions. 
we could have a very basic signal system where we can send signals to the state machine and it will handle the transitions.

this would be a good place to start.


## other QOL changes we never did 
ctrl s save 
crtl z undo 
crtl y redo 


## Through multithreading at it

use QThread to make the state machine run in a separate thread.

But do we need to do this?

no performance issues so far, could be good to learn though.


## make it into a desktop app

right now it is a literal exe file, we should make it into a desktop app.

no clue how to do this yet.

