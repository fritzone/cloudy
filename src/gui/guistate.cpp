#include "guistate.h"
#include "log.h"

#include <string.h>

static bool repaintRequested = true;

void requestRepaint()
{
    repaintRequested = true;
}

bool takeRepaintRequest()
{
    bool r = repaintRequested;
    repaintRequested = false;
    return r;
}

GuiState::GuiState() : cursor(NULL)
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


