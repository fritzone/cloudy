#include "dos_pwds.h"
#include "dos_cgui.h"
#include "log.h"

void GuiState_PasswordScreen::paint(void *screen)
{
    const char* errStr = GuiStatemachine::instance().getError();

    password_window(screen, (char*)m_pwd.c_str(), errorMode, errStr);
}

void GuiState_PasswordScreen::onEnter()
{

}

void GuiState_PasswordScreen::onRightArrow()
{

}

void GuiState_PasswordScreen::onRefreshContent()
{

}

void GuiState_PasswordScreen::onChar(char c)
{

}

void GuiState_PasswordScreen::onTab()
{

}

void GuiState_PasswordScreen::onBackspace()
{

}
