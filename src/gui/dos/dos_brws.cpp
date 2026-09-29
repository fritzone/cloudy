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

// Every remote entry takes around 100 bytes, we cannot keep huge directories
#define MAX_REMOTE_ENTRIES 600

BrowseFoldersState::BrowseFoldersState() : GuiState(),
    workDrive(0), diskFree(0), localFiles(NULL), remoteFocused(false),
    remoteFiles(NULL), remoteLoading(false), remoteFreeKB(0),
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
        if(!remoteLoading)
        {
            requestRemoteDir(fs->hash);
        }
        return;
    }

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
        char t[36] = {0};
        strncpy(t, infoText.c_str(), 35);
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
        // the peer sends the parent as the first entry
        Node* first = remoteFiles->head;
        if(first && !remoteLoading && !strcmp(((FileStructure*)first->data)->sname, ".."))
        {
            requestRemoteDir(((FileStructure*)first->data)->hash);
        }
        return;
    }

    if(strlen(cwd) > 3)
    {
        _chdir("..");
        refreshLocal();
    }
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

    // the selected files, or the current one if none is selected
    std::vector<Transfer::Item> items;
    int skipped = 0;
    bool anySelected = false;
    for(Node* q = panel->head; q; q = q->next)
    {
        FileStructure* fs = (FileStructure*)q->data;
        if(!fs->is_selected)
        {
            continue;
        }
        anySelected = true;
        fs->is_selected = false;

        if(fs->is_dir)
        {
            skipped++;
            continue;
        }
        Transfer::Item it;
        it.name = fs->sname;
        it.hash = fs->hash ? fs->hash : "";
        it.size = fs->file_size;
        items.push_back(it);
    }

    if(!anySelected && panel->currentSelected)
    {
        FileStructure* fs = (FileStructure*)panel->currentSelected->data;
        if(fs->is_dir)
        {
            skipped++;
        }
        else
        {
            Transfer::Item it;
            it.name = fs->sname;
            it.hash = fs->hash ? fs->hash : "";
            it.size = fs->file_size;
            items.push_back(it);
        }
    }

    if(items.empty())
    {
        infoText = skipped ? "Copying directories is not supported yet" : "Nothing to copy";
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

        if(d == Transfer::Download)
        {
            refreshLocal();
        }
        else
        {
            requestRemoteDir(remoteDirHash);
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
