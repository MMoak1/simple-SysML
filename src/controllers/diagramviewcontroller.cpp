#include "../../headers/controllers/diagramviewcontroller.h"
#include "../../headers/models/blockmodel.h"
#include <QDebug>

DiagramViewController::DiagramViewController(QObject *parent)
    : QObject(parent)
{
    // Initialize with BDD as the root context
    DiagramContext bddContext;
    bddContext.type = DiagramContext::Type::BDD;
    bddContext.title = "Block Definition Diagram";
    bddContext.ownerBlock = nullptr;
    m_navigationStack.push(bddContext);
}

void DiagramViewController::enterStateMachine(BlockModel *block)
{
    if (!block)
        return;
    
    DiagramContext smContext;
    smContext.type = DiagramContext::Type::StateMachine;
    smContext.title = QString("State Machine: %1").arg(block->label());
    smContext.ownerBlock = block;
    
    pushContext(smContext);
    
    qDebug() << "Entered state machine for block:" << block->label();
    emit enteringStateMachine(block);
}

void DiagramViewController::goBack()
{
    if (!canGoBack())
        return;
    
    DiagramContext oldContext = m_navigationStack.top();
    popContext();
    
    DiagramContext newContext = currentContext();
    
    qDebug() << "Navigated back from" << oldContext.title << "to" << newContext.title;
    
    if (newContext.type == DiagramContext::Type::BDD)
    {
        emit returningToBDD();
    }
}

DiagramContext DiagramViewController::currentContext() const
{
    if (m_navigationStack.isEmpty())
    {
        // Return default BDD context
        DiagramContext ctx;
        ctx.type = DiagramContext::Type::BDD;
        ctx.title = "Block Definition Diagram";
        return ctx;
    }
    return m_navigationStack.top();
}

QString DiagramViewController::currentTitle() const
{
    return currentContext().title;
}

DiagramContext::Type DiagramViewController::currentViewType() const
{
    return currentContext().type;
}

BlockModel *DiagramViewController::currentStateMachineOwner() const
{
    DiagramContext ctx = currentContext();
    if (ctx.type == DiagramContext::Type::StateMachine)
    {
        return ctx.ownerBlock;
    }
    return nullptr;
}

void DiagramViewController::pushContext(const DiagramContext &context)
{
    bool couldGoBack = canGoBack();
    m_navigationStack.push(context);
    
    notifyViewChange();
    
    if (canGoBack() != couldGoBack)
    {
        emit canGoBackChanged(canGoBack());
    }
}

void DiagramViewController::popContext()
{
    if (m_navigationStack.size() <= 1)
        return;
    
    bool couldGoBack = canGoBack();
    m_navigationStack.pop();
    
    notifyViewChange();
    
    if (canGoBack() != couldGoBack)
    {
        emit canGoBackChanged(canGoBack());
    }
}

void DiagramViewController::notifyViewChange()
{
    emit viewChanged(currentContext());
}
