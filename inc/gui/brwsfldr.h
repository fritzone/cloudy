#ifndef BRWSFLDR_H
#define BRWSFLDR_H

#include "guistate.h"

#include <set>

/**
 * @brief The BrowseFoldersState class The state in which the Gui is, when we browse two folders
 */
class BrowseFoldersState : public GuiState
{
public:

    BrowseFoldersState();
    ~BrowseFoldersState();


    virtual void onEnter();
    virtual void paint(void* screen);
    virtual void onUpArrow();
    virtual void onDownArrow();
    virtual void onInsert();
    virtual void onRightArrow();
    virtual void onBackspace();
    virtual void onTab();
    virtual void onChar(char c);

    virtual void onRefreshContent();

    virtual const char* name() const {return "BrowseFolders";}

    // All the drives in the system
    std::set<char> drives;
    char cwd[PATH_MAX + 1 ];
    unsigned workDrive;
    unsigned long diskFree;
    LinkedList* localFiles;
    LinkedList* focusedPanel;
};

#endif // BRWSFLDR_H
