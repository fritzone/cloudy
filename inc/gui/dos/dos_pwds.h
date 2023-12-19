#ifndef DOS_PWDS_H
#define DOS_PWDS_H

#include "guistate.h"
#include <string>

class GuiState_PasswordScreen : public GuiState
{
public:
    GuiState_PasswordScreen() {}
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

private:

    std::string m_pwd;
    int errorMode;
};

#endif // DOS_PWDS_H
