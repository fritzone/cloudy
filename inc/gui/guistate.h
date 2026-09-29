#ifndef GUISTATE_H
#define GUISTATE_H

#include <state.h>

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
class GuiState : public State
{
public:

    GuiState();
    virtual ~GuiState();

    // called when the content of the GUI needs to be refreshed
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

    // a key with an extended scan code (F keys, PgUp, ...) was pressed
    virtual void onSpecialKey(int scancode) {}

    // Escape was pressed, return true if it was handled, otherwise the application quits
    virtual bool onEscape() { return false; }

    // Draws the current gui state on the backbuffer
    // The parameter is the addres of the screen we paint to
    virtual void paint(void*) = 0;

    // the name of the state
    virtual const char* name() const = 0;

public:

    // cursor handling
    CursorRaii *getCursor() const;

    // sets the cursor handling class
    void setCursor(CursorRaii *newCursor);

public:

    CursorRaii* cursor;
};

/**
 * Asks the main loop to paint the screen again
 */
void requestRepaint();

/**
 * Returns true if a repaint was requested since the last call, and clears the request
 */
bool takeRepaintRequest();

// scan codes of the extended keys
enum ScanCodes
{
    Key_F1 = 59, Key_F2 = 60, Key_F3 = 61, Key_F4 = 62, Key_F5 = 63,
    Key_F6 = 64, Key_F7 = 65, Key_F8 = 66, Key_F9 = 67, Key_F10 = 68,
    Key_Home = 71, Key_Up = 72, Key_PgUp = 73, Key_Left = 75, Key_Right = 77,
    Key_End = 79, Key_Down = 80, Key_PgDn = 81, Key_Insert = 82, Key_Delete = 83
};


#endif // GUISTATE_H
