#ifndef INPUTIP_H
#define INPUTIP_H

#include "guistate.h"

/**
 * @brief The InputIpState class is the state in which the GUI expects for an IP to be entered in the
 * right side of the frame, there is no right frame shown
 */
class InputIpState : public State
{
public:
    InputIpState(State* next, State* prev);

    ~InputIpState();

    virtual void paint(void* screen);

    virtual void onInsert(){}
    virtual void onEnter();
    virtual void onDownArrow() {}
    virtual void onUpArrow() {}
    virtual void onRightArrow();
    virtual void onRefreshContent();
    virtual void onChar(char c);
    virtual void onTab();
    virtual void onBackspace();

    const char* getIp();

public:

    char** ip;
    int currentSegment;
    int ipPosCtrs[4];
    int ipPositions[4];
};

#endif // INPUTIP_H
