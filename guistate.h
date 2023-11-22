#ifndef GUISTATE_H
#define GUISTATE_H

#include "list.h"

#include <stdio.h>

#include <vector>


class CursorRaii;

/**
 * @brief The GuiState class represents a class that holds the state of the current GUI
 */
class State
{
public:

    State(State* next, State* prev);

    virtual void onRefreshContent() = 0;

    // called when the Enter key was pressed.
    virtual void onEnter() = 0;

    // called when the tab key was pressed
    virtual void onTab() = 0;

    // called when the up arrow was pressed
    virtual void onDownArrow() = 0;

    // called when the up arrow was pressed
    virtual void onUpArrow() = 0;

    // called when the up arrow was pressed
    virtual void onRightArrow() = 0;

    // called when a character key was pressed
    virtual void onChar(char c) = 0;

    // when the insert key was pressed
    virtual void onInsert() = 0;

    // the backspace was pressed
    virtual void onBackspace() = 0;

    // Draws the current gui state on the backbuffer
    // The parameter is the addres of the screen we paint to
    virtual void paint(void*) = 0;

public:

    void setNextState(State* nexts);
    void setrPreviousState(State* prevs);
    CursorRaii *getCursor() const;
    void setCursor(CursorRaii *newCursor);

public:

    State* next;
    State* prev;
    CursorRaii* cursor;
};


class GuiState
{
public:

    static GuiState& instance();

    void init(State* s);
    void next();
    void previous();

    State *getCurrentState() const;

private:

    State* currentState;
};

#endif // GUISTATE_H
