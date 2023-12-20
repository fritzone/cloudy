#include "guistate.h"
#include "log.h"

#include <string.h>

GuiState::GuiState()
{
}

GuiState::~GuiState()
{
}

CursorRaii *GuiState::getCursor() const
{
    return cursor;
}

void GuiState::setCursor(CursorRaii *newCursor)
{
    cursor = newCursor;
}


