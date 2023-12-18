#ifndef INPUTIP_H
#define INPUTIP_H

#include "guistate.h"

/**
 * @brief The GuiState_InputIp class is the state in which the GUI expects for an IP to be entered in the
 * right side of the frame, there is no right frame shown
 */
class GuiState_InputIp : public GuiState
{
public:
    GuiState_InputIp();

    ~GuiState_InputIp();

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
    virtual const char* name() const {return "IpInput";}

    const char* getIp();

private:

    char** ip;
    int currentSegment;
    int ipPosCtrs[4];
    int ipPositions[4];

    bool errorMode;
};

#endif // INPUTIP_H
