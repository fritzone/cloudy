#include "guistate.h"

#include <string.h>

GuiState::GuiState(GuiState *next, GuiState *prev) : next(NULL), prev(NULL)
{
}

void GuiState::setNextState(GuiState *nexts)
{
    next = nexts;
}

void GuiState::setrPreviousState(GuiState *prevs)
{
    prev = prevs;
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

void GuiStatemachine::init(GuiState *s)
{
    currentState = s;
}

int GuiStatemachine::next(void *d)
{
    if(stateNextOps.count(currentState))
    {
        if(stateNextOps[currentState](d) == 1)
        {
            return 1;
        }
    }
    currentState = currentState->next;
    return 0;
}

void GuiStatemachine::previous()
{
    currentState = currentState->prev;
}

void GuiStatemachine::reportError(const char *s)
{
    instance().error = s;
}

const char *GuiStatemachine::getError()
{
    return instance().error.c_str();
}

GuiState *GuiStatemachine::getCurrentState() const
{
    return currentState;
}

void GuiStatemachine::addState(GuiState *s)
{
    states.push_back(s);
}

void GuiStatemachine::onNext(GuiState *s, int (*cb)(void *))
{
    stateNextOps[s] = cb;
}


