#include "dos_brws.h"
#include "filestrc.h"
#include "filelist.h"
#include "list.h"
#include "dos_cgui.h"
#include "dos_scrn.h"
#include "transfer.h"
#include "prot.h"
#include "log.h"
#include <guistmch.h>

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>

extern ProtocolImpl p;

// The row of the cursor in the panel, 0 is the top
static int cursorRow(LinkedList* list)
{
    if(list == NULL || list->displayStart == NULL || list->currentSelected == NULL)
    {
        return 0;
    }
    int row = distance(list->displayStart, list->currentSelected);
    return row < frameContentSize() ? row : 0;
}

// Puts the cursor on the entry with the name, at the given row if the list allows it.
// Returns false if there is no such entry.
static bool selectEntry(LinkedList* list, const std::string& name, int row, bool ignoreCase)
{
    int index = 0;
    Node* q = list->head;
    for(; q; q = q->next, index++)
    {
        const char* sname = ((FileStructure*)q->data)->sname;
        if(ignoreCase ? !stricmp(sname, name.c_str()) : name == sname)
        {
            break;
        }
    }
    if(q == NULL)
    {
        return false;
    }

    if(row < 0) row = 0;
    if(row >= frameContentSize()) row = frameContentSize() - 1;
    if(row > index) row = index;

    Node* start = list->head;
    for(int i = 0; i < index - row; i++)
    {
        start = start->next;
    }
    list->displayStart = start;
    list->currentSelected = q;
    return true;
}

// The last part of a path, the directory name
static std::string lastPart(const std::string& path, char separator)
{
    std::string p = path;
    while(p.length() > 1 && p[p.length() - 1] == separator)
    {
        p.erase(p.length() - 1);
    }
    size_t at = p.find_last_of(separator);
    return at == std::string::npos ? p : p.substr(at + 1);
}

// Takes the saved position of the directory we leave, if there is one
static int takePosition(std::vector<BrowseFoldersState::Position>& history, const std::string& name, bool ignoreCase)
{
    if(!history.empty())
    {
        BrowseFoldersState::Position pos = history.back();
        history.pop_back();
        if(ignoreCase ? !stricmp(pos.name.c_str(), name.c_str()) : pos.name == name)
        {
            return pos.row;
        }
        // it was not the one we came through, forget the rest too
        history.clear();
    }
    // somewhere in the middle
    return frameContentSize() / 2;
}

// Every remote entry takes around 100 bytes, we cannot keep huge directories
#define MAX_REMOTE_ENTRIES 600

BrowseFoldersState::BrowseFoldersState() : GuiState(),
    workDrive(0), diskFree(0), localFiles(NULL), remoteFocused(false),
    remoteFiles(NULL), remoteLoading(false), remoteFreeKB(0), remoteSelectRow(0),
    transfer(NULL), lastProgressPaint(0)
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

    remoteFiles = createLinkedList();
    remoteStatus = "Not connected";
    refreshLocal();
}

BrowseFoldersState::~BrowseFoldersState()
{
    delete transfer;
    freeLinkedList(localFiles, deleteFileStructure);
    freeLinkedList(remoteFiles, deleteFileStructure);
}

void BrowseFoldersState::refreshLocal()
{
    getcwd(cwd, PATH_MAX);

    if(localFiles)
    {
        freeLinkedList(localFiles, deleteFileStructure);
    }
    localFiles = createFileList(cwd);

    _dos_getdrive(&workDrive);

    // 0: the current drive
    struct diskfree_t diskspc;
    if(_dos_getdiskfree(0, &diskspc) == 0)
    {
        diskFree = (unsigned long)diskspc.avail_clusters *
                   (unsigned long)diskspc.sectors_per_cluster *
                   (unsigned long)diskspc.bytes_per_sector;
    }

    requestRepaint();
}

void BrowseFoldersState::requestRemoteDir(const std::string& hash)
{
    pendingDirHash = hash;
    remoteLoading = true;
    remoteStatus = "Loading...";

    DirectoryListRequest* r = p.create_DirectoryListRequest(hash, 0);
    if(!p.send(r))
    {
        remoteStatus = "Cannot send request";
        remoteLoading = false;
    }
    delete r;
    requestRepaint();
}

void BrowseFoldersState::onConnected(const std::string& p_hostName)
{
    hostName = p_hostName;
    remoteFreeKB = 0;

    StatusRequest* sr = p.create_StatusRequest();
    p.send(sr);
    delete sr;

    requestRemoteDir("");
}

void BrowseFoldersState::onDisconnected()
{
    if(transfer)
    {
        transfer->cancel();
        infoText = "Connection lost, " + transfer->summary();
        delete transfer;
        transfer = NULL;
    }

    freeLinkedList(remoteFiles, deleteFileStructure);
    remoteFiles = createLinkedList();
    remoteDirName = "";
    remoteDirHash = "";
    remoteLoading = false;
    remoteStatus = "Not connected";
    remoteHistory.clear();
    remoteSelectName = "";
    remoteFocused = false;
}

void BrowseFoldersState::onStatus(const Status* st)
{
    if(!st->get_free_space().empty())
    {
        remoteFreeKB = st->get_free_space()[0];
    }
    requestRepaint();
}

static FileStructure* remoteFileStructure(const DirectoryEntry& e)
{
    FileStructure* fs = newFileStructure(e.get_name().c_str(), e.get_hash().c_str());
    if(fs == NULL)
    {
        return NULL;
    }
    fs->file_size = e.get_size();
    fs->is_dir = e.get_attrs().find('d') != std::string::npos;

    long date = e.get_date();
    fs->year = date / 10000;
    fs->month = (date / 100) % 100;
    fs->day = date % 100;

    long time = e.get_time();
    fs->hour = time / 10000;
    fs->minute = (time / 100) % 100;
    fs->sec = time % 100;

    return fs;
}

void BrowseFoldersState::onDirectoryList(const DirectoryList* dl)
{
    requestRepaint();

    // the listings of a directory being copied
    if(transfer && transfer->onDirectoryList(dl))
    {
        return;
    }

    if(!dl->get_error().empty())
    {
        remoteStatus = dl->get_error();
        remoteLoading = false;
        return;
    }

    // the first page: a new directory
    if(dl->get_start() == 0)
    {
        freeLinkedList(remoteFiles, deleteFileStructure);
        remoteFiles = createLinkedList();
        remoteDirName = dl->get_directory_name();
        remoteDirHash = dl->get_directory_hash();
    }
    else if(dl->get_directory_hash() != remoteDirHash || dl->get_start() != remoteFiles->count)
    {
        log_warning() << "Unexpected directory page, ignored";
        return;
    }

    const std::vector<DirectoryEntry>& entries = dl->get_entries();
    for(size_t i = 0; i < entries.size(); i++)
    {
        FileStructure* fs = remoteFileStructure(entries[i]);
        if(fs == NULL || !insertAtEnd(remoteFiles, fs))
        {
            if(fs) deleteFileStructure(fs);
            log_error() << "Out of memory after" << remoteFiles->count << "entries";
            remoteLoading = false;
            remoteStatus = "Out of memory";
            return;
        }
        remoteFiles->size += fs->file_size;
    }

    // back in a directory: the cursor goes where it was, unless it was moved meanwhile
    if(!remoteSelectName.empty())
    {
        if(remoteFiles->currentSelected != remoteFiles->head)
        {
            remoteSelectName = "";
        }
        else if(selectEntry(remoteFiles, remoteSelectName, remoteSelectRow, false))
        {
            remoteSelectName = "";
        }
    }

    if(remoteFiles->count >= MAX_REMOTE_ENTRIES && remoteFiles->count < dl->get_total())
    {
        remoteLoading = false;
        remoteStatus = "(truncated)";
        return;
    }

    if(!entries.empty() && remoteFiles->count < dl->get_total())
    {
        char s[32];
        sprintf(s, "Loading %d/%d", remoteFiles->count, dl->get_total());
        remoteStatus = s;

        DirectoryListRequest* r = p.create_DirectoryListRequest(remoteDirHash, remoteFiles->count);
        p.send(r);
        delete r;
        return;
    }

    remoteLoading = false;
    remoteStatus = "";
}

void BrowseFoldersState::onFileData(const FileData* fd)
{
    if(transfer)
    {
        transfer->onFileData(fd);
    }
}

void BrowseFoldersState::onFileWriteReply(const FileWriteReply* r)
{
    if(transfer)
    {
        transfer->onFileWriteReply(r);
    }
}

void BrowseFoldersState::onMakeDirectoryReply(const MakeDirectoryReply* r)
{
    if(transfer)
    {
        transfer->onMakeDirectoryReply(r);
    }
}

void BrowseFoldersState::onEnter()
{
    LinkedList* panel = focusedPanel();
    if(transfer || panel->currentSelected == NULL)
    {
        return;
    }

    FileStructure* fs = (FileStructure*)(panel->currentSelected->data);
    if(!fs->is_dir)
    {
        return;
    }

    if(remoteFocused)
    {
        if(remoteLoading)
        {
            return;
        }
        if(!strcmp(fs->sname, ".."))
        {
            remoteUp();
            return;
        }
        Position pos;
        pos.name = fs->sname;
        pos.row = cursorRow(remoteFiles);
        remoteHistory.push_back(pos);
        requestRemoteDir(fs->hash);
        return;
    }

    if(!strcmp(fs->sname, ".."))
    {
        localUp();
        return;
    }

    Position pos;
    pos.name = fs->sname;
    pos.row = cursorRow(localFiles);
    localHistory.push_back(pos);

    getcwd(cwd, PATH_MAX + 1);
    if(cwd[strlen(cwd) - 1] != '\\')
    {
        strcat(cwd, "\\");
    }

    strcat(cwd, fs->sname);
    _chdir(cwd);
    refreshLocal();
}

void BrowseFoldersState::paint(void *screen)
{
    leftFrame(screen, cwd, localFiles, drives, workDrive, diskFree, !remoteFocused);

    std::string title = hostName.empty() ? std::string("(not connected)") : hostName + ":" + remoteDirName;
    rightFrame(screen, title.c_str(), remoteFiles, remoteFocused, remoteFreeKB, remoteStatus.c_str());

    menu(screen);
    if(!infoText.empty())
    {
        // the room right of the menu
        char t[29] = {0};
        strncpy(t, infoText.c_str(), 28);
        writeString(79 - strlen(t), 24, White, Blue, t, screen);
    }

    if(transfer)
    {
        progress_window(screen, (transfer->title() + " - Esc cancels").c_str(), transfer->currentLine().c_str(),
                        transfer->doneBytes(), transfer->totalBytes());
    }
}

void BrowseFoldersState::onUpArrow()
{
    LinkedList* panel = focusedPanel();
    if(transfer || panel->currentSelected == NULL)
    {
        return;
    }

    if(panel->currentSelected != panel->head) // Go up?
    {
        Node* q = panel->head;
        while(q && q->next != panel->currentSelected) q=q->next;
        panel->previousSelected = panel->currentSelected;
        panel->currentSelected = q;
        // scroll one up?
        if(panel->currentSelected->next == panel->displayStart)
        {
            panel->displayStart = panel->currentSelected;
        }
    }
}

void BrowseFoldersState::onDownArrow()
{
    LinkedList* panel = focusedPanel();
    if(transfer || panel->currentSelected == NULL)
    {
        return;
    }

    if(panel->currentSelected->next != NULL) // Can we go down?
    {
        panel->previousSelected = panel->currentSelected;
        panel->currentSelected = panel->currentSelected->next;

        // do we need to scroll one?
        if(distance(panel->displayStart, panel->currentSelected) >= frameContentSize() )
        {
            if(panel->displayStart->next != NULL)
            {
                panel->displayStart = panel->displayStart->next;
            }
        }
    }
}

void BrowseFoldersState::onInsert()
{
    LinkedList* panel = focusedPanel();
    if(transfer || panel->currentSelected == NULL)
    {
        return;
    }

    FileStructure* fs =((FileStructure*)(panel->currentSelected->data));
    if(strcmp(fs->sname, ".."))
    {
        fs->is_selected = ! (fs->is_selected);
    }
}

void BrowseFoldersState::onRightArrow()
{

}

void BrowseFoldersState::onBackspace()
{
    if(transfer)
    {
        return;
    }

    if(remoteFocused)
    {
        remoteUp();
        return;
    }

    localUp();
}

void BrowseFoldersState::localUp()
{
    if(strlen(cwd) <= 3)
    {
        return;
    }

    std::string left = lastPart(cwd, '\\');
    int row = takePosition(localHistory, left, true);

    _chdir("..");
    refreshLocal();
    selectEntry(localFiles, left, row, true);
}

void BrowseFoldersState::remoteUp()
{
    // the peer sends the parent as the first entry
    Node* first = remoteFiles->head;
    if(first == NULL || remoteLoading || strcmp(((FileStructure*)first->data)->sname, ".."))
    {
        return;
    }

    std::string left = lastPart(remoteDirName, '/');
    int row = takePosition(remoteHistory, left, false);
    requestRemoteDirSelecting(((FileStructure*)first->data)->hash, left, row);
}

void BrowseFoldersState::refreshLocalKeepingPosition()
{
    std::string name;
    int row = cursorRow(localFiles);
    if(localFiles && localFiles->currentSelected)
    {
        name = ((FileStructure*)localFiles->currentSelected->data)->sname;
    }

    refreshLocal();
    if(!name.empty())
    {
        selectEntry(localFiles, name, row, true);
    }
}

void BrowseFoldersState::requestRemoteDirSelecting(const std::string& hash, const std::string& name, int row)
{
    requestRemoteDir(hash);
    remoteSelectName = name;
    remoteSelectRow = row;
}

void BrowseFoldersState::onTab()
{
    if(!transfer)
    {
        remoteFocused = !remoteFocused;
    }
}

void BrowseFoldersState::onChar(char c)
{

}

void BrowseFoldersState::onSpecialKey(int scancode)
{
    if(transfer)
    {
        return;
    }

    LinkedList* panel = focusedPanel();

    switch(scancode)
    {
    case Key_F5:
        startCopy();
        break;
    case Key_PgUp:
        for(int i = 0; i < frameContentSize() - 1; i++) onUpArrow();
        break;
    case Key_PgDn:
        for(int i = 0; i < frameContentSize() - 1; i++) onDownArrow();
        break;
    case Key_Home:
        panel->currentSelected = panel->displayStart = panel->head;
        break;
    case Key_End:
        while(panel->currentSelected && panel->currentSelected->next) onDownArrow();
        break;
    }
}

bool BrowseFoldersState::onEscape()
{
    if(transfer)
    {
        transfer->cancel();
        return true;
    }
    return false;
}

void BrowseFoldersState::startCopy()
{
    LinkedList* panel = focusedPanel();
    if(p.socket() == NULL || remoteDirHash.empty() || remoteLoading)
    {
        infoText = "Not connected";
        return;
    }

    // the selected files and directories, or the current one if none is selected
    std::vector<Transfer::Item> items;
    for(Node* q = panel->head; q; q = q->next)
    {
        FileStructure* fs = (FileStructure*)q->data;
        if(!fs->is_selected)
        {
            continue;
        }
        fs->is_selected = false;

        Transfer::Item it;
        it.name = fs->sname;
        it.hash = fs->hash ? fs->hash : "";
        it.size = fs->file_size;
        it.isDir = fs->is_dir;
        items.push_back(it);
    }

    if(items.empty() && panel->currentSelected)
    {
        FileStructure* fs = (FileStructure*)panel->currentSelected->data;
        if(strcmp(fs->sname, ".."))
        {
            Transfer::Item it;
            it.name = fs->sname;
            it.hash = fs->hash ? fs->hash : "";
            it.size = fs->file_size;
            it.isDir = fs->is_dir;
            items.push_back(it);
        }
    }

    if(items.empty())
    {
        infoText = "Nothing to copy";
        return;
    }

    transfer = new Transfer(remoteFocused ? Transfer::Download : Transfer::Upload, items, remoteDirHash);
    infoText = "";
}

void BrowseFoldersState::onRefreshContent()
{
    if(transfer == NULL)
    {
        return;
    }

    transfer->step();

    if(transfer->isFinished())
    {
        infoText = transfer->summary();
        Transfer::Direction d = transfer->direction();
        delete transfer;
        transfer = NULL;

        // the copied entries show up, the cursors stay where they were
        if(d == Transfer::Download)
        {
            refreshLocalKeepingPosition();
        }
        else
        {
            std::string name;
            if(remoteFiles->currentSelected)
            {
                name = ((FileStructure*)remoteFiles->currentSelected->data)->sname;
            }
            requestRemoteDirSelecting(remoteDirHash, name, cursorRow(remoteFiles));
        }
        requestRepaint();
        return;
    }

    // update the progress a few times a second
    unsigned long now = clock();
    if(now - lastProgressPaint > CLOCKS_PER_SEC / 4)
    {
        lastProgressPaint = now;
        requestRepaint();
    }
}
