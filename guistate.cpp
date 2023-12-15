#include "guistate.h"
#include "log.h"

#include <string.h>

GuiState::GuiState()
{
}

void GuiState::setNextState(GuiStates state, GuiState *nexts)
{
    m_nexts[state] = nexts;
}

void GuiState::setrPreviousState(GuiStates state, GuiState *prevs)
{
    m_prevs[state] = prevs;
}

CursorRaii *GuiState::getCursor() const
{
    return cursor;
}

void GuiState::setCursor(CursorRaii *newCursor)
{
    cursor = newCursor;
}

GuiStatemachine &GuiStatemachine::instance()
{
    static GuiStatemachine st;
    return st;
}

void GuiStatemachine::logStates()
{
    log_info() << "----- StateCount:" << states.size();
    for(int i=0; i<states.size(); i++)
    {
        log_info() << "State:" << states[i]->name() << " at " << (void*)states[i]
                   <<  (states[i] == currentState ? " ***" : "");
    }
    log_info() << "----- StateCount:" << states.size();

}

void GuiStatemachine::init(GuiState *s)
{
    currentState = s;
    logStates();
}

int GuiStatemachine::advance(void *d, GuiStates state)
{
    logStates();

    if(!currentState)
    {
        log_error() << "No current state for gui statemachine";
        return 1;
    }

    log_info() << "Trying to advance from" << currentState->name();

    if(currentState->m_nexts.count(state) && currentState->m_nexts[state])
    {
        currentState = currentState->m_nexts[state];
    }
    else
    {
        log_error()  << "No next for" << currentState->name();
        return 1;
    }
    return 0;
}

int GuiStatemachine::previous(void *, GuiStates state)
{
    if(!currentState)
    {
        log_error() << "No current state for gui statemachine";
        return 1;
    }

    if(currentState->m_prevs.count(state) && currentState->m_prevs[state])
    {
        currentState = currentState->m_prevs[state];
    }
    return 0;
}

void GuiStatemachine::reportError(const char *s)
{
    log_debug() << "Gui:" << (void*)&instance() << "Reporting error:" << s;
    instance().error = s;
}

const char *GuiStatemachine::getError()
{
    return instance().error.c_str();
}

GuiState *GuiStatemachine::getCurrentState() const
{
    log_info() << "Returns:" <<(void*)currentState;
    return currentState;
}

void GuiStatemachine::addState(GuiState *s)
{
    states.push_back(s);
}

GuiStatemachine::GuiStatemachine() : currentState(NULL), states(), error("")
{
}



