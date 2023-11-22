#include "brwsfldr.h"
#include "filestrc.h"
#include "filelist.h"
#include "list.h"
#include "dos_cgui.h"

#include <dos.h>
#include <string.h>
#include <direct.h>

BrowseFoldersState::BrowseFoldersState(State *next, State *prev) : State(next, prev),
    workDrive(0), diskFree(0), localFiles(NULL), focusedPanel(NULL)
{
    memset(cwd, 0, sizeof(cwd));

    _dos_getdrive(&workDrive);

    unsigned total = 0;
    unsigned currentDrive = 0;
    for(char d = 'A'; d <= 'Z'; d++)
    {
        _dos_setdrive(d - 'A' + 1, &total);
        _dos_getdrive(&currentDrive);
        drives.insert(currentDrive + 'A' - 1);
    }

    _dos_setdrive(workDrive, &total);

    // the current working directory

    getcwd(cwd, PATH_MAX + 1);


    localFiles = createFileList(cwd);
    focusedPanel = localFiles;
}

BrowseFoldersState::~BrowseFoldersState()
{
    freeLinkedList(localFiles, deleteFileStructure);
}

void BrowseFoldersState::onEnter()
{
    if(focusedPanel == localFiles)
    {
        FileStructure* fs =((FileStructure*)(focusedPanel->currentSelected->data));
        if(fs->is_dir)
        {
            getcwd(cwd, PATH_MAX + 1);
            if(cwd[strlen(cwd) - 1] != '\\')
            {
                strcat(cwd, "\\");
            }

            strcat(cwd, fs->sname);
            _chdir(cwd);

            freeLinkedList(localFiles, deleteFileStructure);
            localFiles = createFileList(cwd);
            focusedPanel = localFiles;
        }
    }
}

void BrowseFoldersState::paint(void *screen)
{
    leftFrame(screen, cwd, localFiles, drives, workDrive, diskFree);
    rightFrame(screen, NULL);
    menu(screen);
}

void BrowseFoldersState::onUpArrow()
{
    if(focusedPanel->currentSelected != focusedPanel->head) // Go up?
    {
        Node* q = focusedPanel->head;
        while(q && q->next != focusedPanel->currentSelected) q=q->next;
        focusedPanel->previousSelected = focusedPanel->currentSelected;
        focusedPanel->currentSelected = q;
        // move one up?
        if(distance(focusedPanel->currentSelected, focusedPanel->displayEnd) > frameContentSize() )
        {
            Node* q1 = focusedPanel->head;
            while(q1 && q1->next != focusedPanel->displayStart) q1=q1->next;
            focusedPanel->displayStart = q1;
        }
    }
}

void BrowseFoldersState::onDownArrow()
{
    if(focusedPanel->currentSelected->next != NULL) // Can we go down?
    {
        focusedPanel->previousSelected = focusedPanel->currentSelected;
        focusedPanel->currentSelected = focusedPanel->currentSelected->next;

        // do we need to scroll one?
        if(distance(focusedPanel->displayStart, focusedPanel->currentSelected) >= frameContentSize() )
        {
            if(focusedPanel->displayStart->next != NULL)
            {
                focusedPanel->displayStart = focusedPanel->displayStart->next;
            }
        }
    }
}

void BrowseFoldersState::onInsert()
{
    FileStructure* fs =((FileStructure*)(focusedPanel->currentSelected->data));
    fs->is_selected = ! (fs->is_selected);
}

void BrowseFoldersState::onRightArrow()
{

}

void BrowseFoldersState::onBackspace()
{

}

void BrowseFoldersState::onTab()
{

}

void BrowseFoldersState::onChar(char c)
{

}

void BrowseFoldersState::onRefreshContent()
{
    // Need to do it again for the display
    getcwd(cwd, PATH_MAX + 1);

    // calculate the disk free
    struct diskfree_t diskspc;
    unsigned workDrive = 0;
    if(_dos_getdiskfree(workDrive, &diskspc) == 0)
    {
        diskFree = (unsigned long)diskspc.avail_clusters *
                   (unsigned long)diskspc.sectors_per_cluster *
                   (unsigned long)diskspc.bytes_per_sector;

    }

    _dos_getdrive(&workDrive);
}
