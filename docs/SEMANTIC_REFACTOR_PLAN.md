# SysML Semantic Modeling Refactor Plan

## Overview

This document outlines a comprehensive refactor to transform the application from a **drawing tool** into a proper **SysML modeling tool** with semantic understanding of types, instances, and part properties.

---

## Current Architecture Summary

### Existing Classes

| Class | Purpose |
|-------|---------|
| `BlockModel` | Visual block on canvas (conflates type and instance) |
| `ConnectionModel` | Line between blocks with label/multiplicity |
| `StateMachineModel` | State machine definition owned by a block |
| `StateModel` | State within a state machine |
| `TransitionModel` | Transition between states |

### Current Limitations

1. No distinction between **block type** (e.g., "Wheel") and **block instance** (e.g., "frontLeftWheel")
2. Connections are just visual lines, not semantic part properties
3. Part names are not validated for uniqueness
4. Hierarchy tree shows block names, not instance specifications

---

## New Semantic Model

### Core Concepts

```
┌─────────────────────────────────────────────────────────────────┐
│                    BLOCK DEFINITION (Type)                       │
│  - typeName: "Car"                                               │
│  - color, size (visual properties)                               │
│  - stateMachineTemplate (optional)                               │
│  - partProperties: List<PartProperty>                            │
│    ├── PartProperty: "engine", type="Engine", mult="1"           │
│    ├── PartProperty: "wheels", type="Wheel", mult="4"            │
│    └── PartProperty: "transmission", type="Transmission"         │
└─────────────────────────────────────────────────────────────────┘
```

### Class Diagram

```
┌──────────────────────┐
│  BlockDefinition     │
├──────────────────────┤
│ - id: QString        │
│ - typeName: QString  │
│ - color: QColor      │
│ - size: QSizeF       │
│ - position: QPointF  │
│ - stateMachine: SM*  │
├──────────────────────┤      ┌──────────────────────┐
│ + partProperties()   │──────│    PartProperty      │
│ + addPartProperty()  │ 0..* ├──────────────────────┤
│ + removePartProperty │      │ - id: QString        │
│ + findPartByName()   │      │ - name: QString      │
└──────────────────────┘      │ - multiplicity: str  │
                              │ - typeRef: BlockDef* │
                              │ + displayText()      │
                              └──────────────────────┘
```

---

## New Class Definitions

### 1. BlockDefinition (replaces BlockModel)

```cpp
class BlockDefinition : public QObject
{
    Q_OBJECT

public:
    explicit BlockDefinition(const QString &typeName, const QColor &color, 
                             const QPointF &position, QObject *parent = nullptr);

    // Identity
    QString id() const;
    void setId(const QString &id);
    
    // Type name (e.g., "Car", "Wheel")
    QString typeName() const;
    void setTypeName(const QString &name);

    // Visual properties (for BDD canvas)
    QColor color() const;
    QPointF position() const;
    QSizeF size() const;
    void setPosition(const QPointF &pos);
    void setSize(const QSizeF &size);

    // State machine (optional behavioral definition)
    StateMachineModel* stateMachine() const;
    StateMachineModel* getOrCreateStateMachine();
    bool hasStateMachine() const;

    // Part properties (composition relationships)
    QList<PartProperty*> partProperties() const;
    PartProperty* addPartProperty(const QString &name, BlockDefinition *type, 
                                   const QString &multiplicity = "1");
    bool removePartProperty(PartProperty *part);
    PartProperty* findPartPropertyByName(const QString &name) const;
    bool hasPartPropertyNamed(const QString &name) const;
    
    // Auto-generate unique part name
    QString generateUniquePartName(BlockDefinition *type) const;

signals:
    void typeNameChanged(const QString &name);
    void positionChanged(const QPointF &pos);
    void sizeChanged(const QSizeF &size);
    void partPropertyAdded(PartProperty *part);
    void partPropertyRemoved(PartProperty *part);

private:
    QString m_id;
    QString m_typeName;
    QColor m_color;
    QPointF m_position;
    QSizeF m_size;
    StateMachineModel *m_stateMachine = nullptr;
    QList<PartProperty*> m_partProperties;
};
```

### 2. PartProperty (new class - replaces semantic role of ConnectionModel)

```cpp
class PartProperty : public QObject
{
    Q_OBJECT

public:
    explicit PartProperty(const QString &name, BlockDefinition *type,
                          const QString &multiplicity = "1", QObject *parent = nullptr);

    // Identity
    QString id() const;
    
    // Part name (e.g., "wheels", "engine")
    QString name() const;
    void setName(const QString &name);
    
    // Multiplicity (e.g., "1", "4", "0..*")
    QString multiplicity() const;
    void setMultiplicity(const QString &mult);
    
    // Type reference (what type does this part hold?)
    BlockDefinition* type() const;
    void setType(BlockDefinition *type);
    
    // Display text for hierarchy tree and canvas
    // Format: "[multiplicity] name : TypeName"
    QString displayText() const;
    
    // Owner (which BlockDefinition owns this part?)
    BlockDefinition* owner() const;

signals:
    void nameChanged(const QString &name);
    void multiplicityChanged(const QString &mult);
    void typeChanged(BlockDefinition *type);

private:
    QString m_id;
    QString m_name;
    QString m_multiplicity;
    BlockDefinition *m_type;
    BlockDefinition *m_owner;  // Set by BlockDefinition::addPartProperty
};
```

### 3. ConnectionView Changes

`ConnectionView` will now visualize a `PartProperty` instead of `ConnectionModel`:

```cpp
class ConnectionView : public QGraphicsObject
{
public:
    explicit ConnectionView(PartProperty *partProperty, 
                            BlockDefinitionView *ownerView,
                            BlockDefinitionView *typeView,
                            QGraphicsItem *parent = nullptr);

    PartProperty* partProperty() const { return m_partProperty; }
    
    // Display text: "[4] wheels : Wheel"
    QString displayText() const;

private:
    PartProperty *m_partProperty;
    BlockDefinitionView *m_ownerView;  // Start of line (e.g., Car)
    BlockDefinitionView *m_typeView;   // End of line (e.g., Wheel)
};
```

---

## Auto-Generated Part Names

When a user draws a connection from `Car` to `Wheel`:

1. Check if any part properties already exist for that type
2. Generate a unique name: `wheel`, `wheel2`, `wheel3`, etc.

```cpp
QString BlockDefinition::generateUniquePartName(BlockDefinition *type) const
{
    // Base name is the type name in lowercase
    QString baseName = type->typeName().toLower();
    
    // Check if baseName already exists
    if (!hasPartPropertyNamed(baseName)) {
        return baseName;
    }
    
    // Find next available number
    int counter = 2;
    while (hasPartPropertyNamed(baseName + QString::number(counter))) {
        counter++;
    }
    
    return baseName + QString::number(counter);
}
```

---

## Part Name Uniqueness Validation

When user tries to rename a part property:

```cpp
bool BlockDefinition::validatePartName(const QString &newName, PartProperty *excludePart) const
{
    for (PartProperty *part : m_partProperties) {
        if (part != excludePart && part->name() == newName) {
            return false;  // Name already exists
        }
    }
    return true;
}
```

In the dialog:

```cpp
void ConnectionView::showLabelInputDialog()
{
    QString newName = QInputDialog::getText(...);
    
    if (!m_partProperty->owner()->validatePartName(newName, m_partProperty)) {
        QMessageBox::warning(nullptr, "Invalid Name",
            QString("A part named '%1' already exists in %2.")
                .arg(newName, m_partProperty->owner()->typeName()));
        return;
    }
    
    m_partProperty->setName(newName);
}
```

---

## Hierarchy Tree Display

The tree should display the **instance specification** format:

```
Car                              ← Root type (no owner)
├── [4] wheels : Wheel           ← Part property of Car
│   ├── [1] hubcap : Hubcap      ← Part property of Wheel
│   └── [1] tire : Tire          ← Part property of Wheel
├── [1] engine : Engine          ← Part property of Car
└── [1] transmission : Transmission
```

### Tree Item Data

Each tree item stores:
- `BlockDefinition*` - the type being displayed
- `PartProperty*` - the part property (null for root items)

```cpp
struct HierarchyTreeItem {
    BlockDefinition *blockDef;
    PartProperty *partProperty;  // null for roots
    
    QString displayText() const {
        if (partProperty) {
            return partProperty->displayText();  // "[4] wheels : Wheel"
        } else {
            return blockDef->typeName();  // "Car"
        }
    }
};
```

---

## Migration Path

### Phase 1: Create New Classes
1. Create `BlockDefinition` class (copy from `BlockModel`, add part properties)
2. Create `PartProperty` class
3. Both exist alongside old classes temporarily

### Phase 2: Update Controllers
1. `DropController` → creates `BlockDefinition` instead of `BlockModel`
2. `ConnectionController` → creates `PartProperty` on the owner block
3. `HierarchyController` → displays part properties in tree

### Phase 3: Update Views
1. `BlockView` → `BlockDefinitionView` (minimal changes)
2. `ConnectionView` → references `PartProperty` instead of `ConnectionModel`
3. Update all paint/display logic

### Phase 4: Update Serialization
1. `ModelSerializer` saves/loads `BlockDefinition` with nested `PartProperty` list
2. Maintain backward compatibility with old save files (optional)

### Phase 5: Remove Old Classes
1. Delete `BlockModel`
2. Delete `ConnectionModel` (absorbed into `PartProperty`)
3. Update all references

---

## File Changes Summary

| File | Change Type | Notes |
|------|-------------|-------|
| `headers/models/blockdefinition.h` | NEW | Replace BlockModel |
| `src/models/blockdefinition.cpp` | NEW | Implementation |
| `headers/models/partproperty.h` | NEW | New class |
| `src/models/partproperty.cpp` | NEW | Implementation |
| `headers/models/blockmodel.h` | DELETE | Eventually |
| `src/models/blockmodel.cpp` | DELETE | Eventually |
| `headers/models/connectionmodel.h` | DELETE | Replaced by PartProperty |
| `src/models/connectionmodel.cpp` | DELETE | Replaced by PartProperty |
| `headers/views/blockview.h` | MODIFY | Rename to BlockDefinitionView |
| `src/views/blockview.cpp` | MODIFY | Point to BlockDefinition |
| `headers/views/connectionview.h` | MODIFY | Point to PartProperty |
| `src/views/connectionview.cpp` | MODIFY | Update display, dialogs |
| `src/controllers/dropcontroller.cpp` | MODIFY | Create BlockDefinition |
| `src/controllers/connectioncontroller.cpp` | MODIFY | Create PartProperty |
| `src/controllers/hierarchycontroller.cpp` | MODIFY | Display part properties |
| `src/io/modelserializer.cpp` | MODIFY | New save/load format |

---

## Design Decisions (Resolved)

### 1. IBD Support
**Decision**: Defer for now. Internal Block Diagrams will be implemented in a future phase.

### 2. State Machine Instances
**Decision**: Each `PartProperty` gets its **own independent state machine instance**.

- `BlockDefinition` has a state machine **template** (the structure: states, transitions)
- Each `PartProperty` gets a **copy** of that template as its own instance
- Different instances can be in different states simultaneously

**Example**:
```
Light (BlockDefinition)
├── stateMachine template: states = [ON, OFF], initial = OFF

Car (BlockDefinition)
├── light1 : Light → stateMachine instance (current state: ON)
├── light2 : Light → stateMachine instance (current state: OFF)
```

Both `light1` and `light2` have the same structure (ON/OFF states), but `light1` is ON while `light2` is OFF.

**Implementation in PartProperty**:

```cpp
class PartProperty : public QObject
{
    // ... existing members ...
    
    // State machine INSTANCE (cloned from type's template)
    StateMachineModel* stateMachineInstance() const;
    StateMachineModel* getOrCreateStateMachineInstance();
    bool hasStateMachineInstance() const;
    
private:
    StateMachineModel *m_stateMachineInstance = nullptr;  // Independent instance
};
```

**Cloning logic**:
```cpp
StateMachineModel* PartProperty::getOrCreateStateMachineInstance()
{
    if (!m_stateMachineInstance && m_type && m_type->hasStateMachine())
    {
        // Clone the type's state machine template
        m_stateMachineInstance = m_type->stateMachine()->clone();
        m_stateMachineInstance->setParent(this);
    }
    return m_stateMachineInstance;
}
```

### 3. Save File Compatibility
**Decision**: No backward compatibility required. Old JSON files will not load after refactor.

### 4. Value Properties and Ports
**Decision**: Defer implementation, but design architecture for extensibility.

**Extensibility Pattern**:
```cpp
class BlockDefinition : public QObject
{
    // Current
    QList<PartProperty*> m_partProperties;
    
    // Future extensibility (use same pattern)
    // QList<ValueProperty*> m_valueProperties;
    // QList<Port*> m_ports;
    // QList<FlowPort*> m_flowPorts;
};
```

---

## Updated PartProperty Class

With state machine instance support:

```cpp
class PartProperty : public QObject
{
    Q_OBJECT

public:
    explicit PartProperty(const QString &name, BlockDefinition *type,
                          const QString &multiplicity = "1", QObject *parent = nullptr);
    ~PartProperty();

    // Identity
    QString id() const;
    
    // Part name (e.g., "wheels", "engine")
    QString name() const;
    void setName(const QString &name);
    
    // Multiplicity (e.g., "1", "4", "0..*")
    QString multiplicity() const;
    void setMultiplicity(const QString &mult);
    
    // Type reference (what type does this part hold?)
    BlockDefinition* type() const;
    void setType(BlockDefinition *type);
    
    // Owner (which BlockDefinition owns this part?)
    BlockDefinition* owner() const;
    
    // Display text for hierarchy tree and canvas
    // Format: "[multiplicity] name : TypeName"
    QString displayText() const;
    
    // State machine INSTANCE (independent copy of type's template)
    StateMachineModel* stateMachineInstance() const;
    StateMachineModel* getOrCreateStateMachineInstance();
    bool hasStateMachineInstance() const;

signals:
    void nameChanged(const QString &name);
    void multiplicityChanged(const QString &mult);
    void typeChanged(BlockDefinition *type);
    void stateMachineInstanceChanged();

private:
    QString m_id;
    QString m_name;
    QString m_multiplicity;
    BlockDefinition *m_type;
    BlockDefinition *m_owner;
    StateMachineModel *m_stateMachineInstance = nullptr;
};
```

---

## Recommended Next Steps

1. **Review this document** - confirm all decisions are correct
2. **Create BlockDefinition and PartProperty** classes first
3. **Implement StateMachineModel::clone()** for creating instances
4. **Create parallel view classes** (BlockDefinitionView) 
5. **Migrate controllers one at a time**
6. **Update serialization last** (after testing in-memory)
7. **Delete old classes** after full migration

---

## Estimated Effort

| Phase | Effort |
|-------|--------|
| Phase 1: New Classes (BlockDefinition, PartProperty, SM clone) | 3-4 hours |
| Phase 2: Controllers | 2-3 hours |
| Phase 3: Views | 2-3 hours |
| Phase 4: Serialization | 1-2 hours |
| Phase 5: Cleanup | 1 hour |
| **Total** | **9-13 hours** |
