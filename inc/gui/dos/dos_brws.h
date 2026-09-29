#ifndef BRWSFLDR_H
#define BRWSFLDR_H

#include <guistate.h>
#include <list.h>
#include <stdio.h>

#include <set>
#include <string>
#include <vector>

class Status;
class DirectoryList;
class FileData;
class FileWriteReply;
class MakeDirectoryReply;
class Transfer;

/**
 * @brief The BrowseFoldersState class The state in which the Gui is, when we browse two folders:
 * the local one on the left and the remote one on the right.
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
    virtual void onSpecialKey(int scancode);
    virtual bool onEscape();

    virtual void onRefreshContent();

    virtual const char* name() const {return "BrowseFolders";}

    // Called when the connection with the peer is up
    void onConnected(const std::string& hostName);

    // Called when the connection is lost
    void onDisconnected();

    // Messages from the peer
    void onStatus(const Status* st);
    void onDirectoryList(const DirectoryList* dl);
    void onFileData(const FileData* fd);
    void onFileWriteReply(const FileWriteReply* r);
    void onMakeDirectoryReply(const MakeDirectoryReply* r);

private:

    // reads the current local directory again
    void refreshLocal();

    // asks the peer for the listing of the directory, "" is its start directory
    void requestRemoteDir(const std::string& hash);

    // copies the selected (or the current) files from the focused panel to the other one
    void startCopy();

    // go to the parent directory, the cursor goes to the directory we leave
    void localUp();
    void remoteUp();

    // reads the local directory again, keeping the cursor on the same entry
    void refreshLocalKeepingPosition();

    // asks for the listing, the cursor goes to the given entry when it arrives
    void requestRemoteDirSelecting(const std::string& hash, const std::string& name, int row);

    // where the cursor was in a directory, when we entered one of its subdirectories
    struct Position
    {
        std::string name;
        int row;
    };

public:

    // All the drives in the system
    std::set<char> drives;
    char cwd[PATH_MAX + 1 ];
    unsigned workDrive;
    unsigned long diskFree;
    LinkedList* localFiles;

    // which panel has the focus
    bool remoteFocused;
    LinkedList* focusedPanel() const { return remoteFocused ? remoteFiles : localFiles; }

    // the remote side
    LinkedList* remoteFiles;
    std::string hostName;
    std::string remoteDirName;
    std::string remoteDirHash;
    // the directory we asked for, and are receiving pages of
    std::string pendingDirHash;
    bool remoteLoading;
    std::string remoteStatus;
    unsigned long remoteFreeKB;

    // the positions in the directories above the current ones
    std::vector<Position> localHistory;
    std::vector<Position> remoteHistory;

    // the entry to select (at the row) when the remote listing arrives
    std::string remoteSelectName;
    int remoteSelectRow;

    // the running copy, if any
    Transfer* transfer;
    unsigned long lastProgressPaint;

    // shown in the menu line, the result of the last copy
    std::string infoText;
};

#endif // BRWSFLDR_H
