#include "guistate.h"

#include <string.h>

State::State(State *next, State *prev) : next(NULL), prev(NULL)
{
}

void State::setNextState(State *nexts)
{
    next = nexts;
}

void State::setrPreviousState(State *prevs)
{
    prev = prevs;
}

CursorRaii *State::getCursor() const
{
    return cursor;
}

void State::setCursor(CursorRaii *newCursor)
{
    cursor = newCursor;
}

GuiState &GuiState::instance()
{
    static GuiState st;
    return st;
}

void GuiState::init(State *s)
{
    currentState = s;
}

void GuiState::next()
{
    currentState = currentState->next;
}

void GuiState::previous()
{
    currentState = currentState->prev;
}

State *GuiState::getCurrentState() const
{
    return currentState;
}


