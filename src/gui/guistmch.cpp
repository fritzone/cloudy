#include "guistmch.h"

GuiState* GuiStatemachine::getCurrentState() const
{
    return (GuiState*)Statemachine::getCurrentState();
}


GuiStatemachine &GuiStatemachine::instance()
{
    static GuiStatemachine i;
    return i;
}
