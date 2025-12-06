#ifndef DIAGRAMVIEWCONTROLLER_H
#define DIAGRAMVIEWCONTROLLER_H

#include <QObject>
#include <QStack>
#include <QString>

class BlockDefinition;
class QGraphicsScene;

// Represents a diagram context in the navigation stack
struct DiagramContext
{
    enum class Type {
        BDD,           // Block Definition Diagram
        StateMachine   // State Machine Diagram
    };
    
    Type type;
    QString title;
    BlockDefinition *ownerDefinition = nullptr;  // For state machines, the parent block definition
};

class DiagramViewController : public QObject
{
    Q_OBJECT

public:
    explicit DiagramViewController(QObject *parent = nullptr);
    
    // Navigation methods
    void enterStateMachine(BlockDefinition *definition);
    void goBack();
    
    // Query methods
    bool canGoBack() const { return m_navigationStack.size() > 1; }
    DiagramContext currentContext() const;
    QString currentTitle() const;
    DiagramContext::Type currentViewType() const;
    
    // Get the block whose state machine is currently shown (nullptr if at BDD level)
    BlockDefinition *currentStateMachineOwner() const;
    
signals:
    void viewChanged(const DiagramContext &context);
    void canGoBackChanged(bool canGoBack);
    void enteringStateMachine(BlockDefinition *definition);
    void returningToBDD();

private:
    QStack<DiagramContext> m_navigationStack;
    
    void pushContext(const DiagramContext &context);
    void popContext();
    void notifyViewChange();
};

#endif // DIAGRAMVIEWCONTROLLER_H
