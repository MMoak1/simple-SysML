## Additional Features

right now we clone the state machine, this means that when we change the state machine, the cloned state machine is not changed. we need to make sure that the cloned state machine is updated when the original state machine is updated.

we should switch over to a type instance of the state machine, so that we can have multiple state machines of the same type.

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

