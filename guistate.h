#ifndef GUISTATE_H
#define GUISTATE_H

#include "list.h"

#include <stdio.h>

#include <vector>
#include <map>

/**
 *  Contains all the states the GUI can go through
 **/
enum GuiStates
{
    State_PasswordRequested = 1,    // From InputIp -> InputPassword
    State_GoBrowsing = 2,           // From InputIp -> BrowseFolders

    State_Invalid = 1024
};


class CursorRaii;

/**
 * @brief The GuiState class represents a class that holds the state of the current GUI
 */
class GuiState
{
public:

    GuiState();
    virtual ~GuiState() {}

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

    // the name of the state
    virtual const char* name() const = 0;

public:

    void setNextState(GuiStates state, GuiState* nexts);
    void setrPreviousState(GuiStates state, GuiState* prevs);
    CursorRaii *getCursor() const;
    void setCursor(CursorRaii *newCursor);

public:

    std::map<GuiStates, GuiState*> m_nexts;
    std::map<GuiStates, GuiState*> m_prevs;
    CursorRaii* cursor;
};


class GuiStatemachine
{
public:

    static GuiStatemachine& instance();

    void logStates();

    void init(GuiState* s);
    int advance(void*, GuiStates);
    int previous(void*, GuiStates);

    static void reportError(const char*);
    static const char* getError();

    GuiState *getCurrentState() const;
    void addState(GuiState*);

private:

    GuiStatemachine();

private:

    GuiState* currentState;
    std::vector<GuiState*> states;
    std::string error;
};

#endif // GUISTATE_H
