#ifndef _TRANSFER_H_
#define _TRANSFER_H_

#include <stdio.h>
#include <string>
#include <vector>

class FileData;
class FileWriteReply;
class DirectoryList;
class MakeDirectoryReply;

/**
 * Copies files and directories (recursively) between the local directory and a
 * remote one.
 *
 * It goes through the directories depth first, keeping only one directory level
 * of entries per depth in memory. A remote directory is read page by page as
 * it is being copied, a local one when it is entered.
 *
 * For the files step() keeps up to PIPELINE requests on the way, the replies
 * arrive (in order) through onFileData / onFileWriteReply. Listing a remote
 * directory and creating one are single requests, answered through
 * onDirectoryList / onMakeDirectoryReply. Downloads get DOS 8.3 names.
 */
class Transfer
{
public:

    enum Direction
    {
        Download,   // remote -> local
        Upload      // local -> remote
    };

    struct Item
    {
        // the remote (long) name for downloads, the local DOS name for uploads
        std::string name;
        // the hash of the remote file or directory, for downloads
        std::string hash;
        unsigned long size;
        bool isDir;
    };

    /**
     * The items are in the current local directory (uploads) or in the remote
     * directory remoteDirHash (downloads), which is also where uploads go.
     */
    Transfer(Direction d, const std::vector<Item>& items, const std::string& remoteDirHash);
    ~Transfer();

    /**
     * Moves the transfer forward, call it from the main loop
     */
    void step();

    void onFileData(const FileData* fd);
    void onFileWriteReply(const FileWriteReply* r);

    /**
     * Returns false if the listing was not asked for by the transfer
     */
    bool onDirectoryList(const DirectoryList* dl);
    void onMakeDirectoryReply(const MakeDirectoryReply* r);

    /**
     * Stops the transfer, a partially downloaded file is removed
     */
    void cancel();

    bool isFinished() const { return finished; }
    Direction direction() const { return dir; }

    // for the progress window: the counts, the current file and its progress
    std::string title() const;
    std::string currentLine() const;
    unsigned long doneBytes() const { return offset; }
    unsigned long totalBytes() const { return fp ? cur.size : 0; }

    /**
     * What happened, once the transfer is finished
     */
    std::string summary() const;

private:

    // how many requests can be on the way at once
    enum { PIPELINE = 2 };

    // how deep we go into directories
    enum { MAX_DEPTH = 16 };

    // what the transfer waits for, besides the file pieces
    enum Waiting { WaitNothing, WaitListing, WaitMakeDir };

    /**
     * One directory of the tree being copied
     */
    struct Level
    {
        // the local directory, a full path
        std::string localDir;
        // the remote directory
        std::string remoteDirHash;
        // the entries to copy, for downloads a page of the remote listing
        std::vector<Item> entries;
        size_t next;
        // downloads: how many entries of the remote listing arrived, and how many there are
        int listed;
        int total;
        // the DOS names given in localDir, two long names might give the same one
        std::vector<std::string> usedNames;
        // for the names given more than once, the last ~N used: "NAME.EXT" -> N
        std::vector<std::string> suffixNames;
        std::vector<int> suffixes;
    };

    bool startFile(const Item& it);
    bool startDirectory(const Item& it);
    void finishItem(bool ok, const std::string& error);
    void failDirectory(const std::string& name, const std::string& error);
    void pushLevel(const std::string& localDir, const std::string& remoteDirHash);
    bool readLocalDirectory(Level& level);
    std::string uniqueDosName(Level& level, const std::string& longName);
    bool sendReadRequest();
    bool sendWriteChunk();
    void fail(const std::string& error);

private:

    Direction dir;
    std::vector<Level> levels;
    Waiting waiting;

    // for uploads, the directory being created
    std::string pendingDirLocal;
    std::string pendingDirRemote;

    // the file being copied
    Item cur;
    std::string curRemoteDir;
    std::string remoteName;
    std::string localPath;
    FILE* fp;

    // confirmed: written (download) or acknowledged (upload) up to here
    unsigned long offset;
    // the next request goes for this offset
    unsigned long nextOffset;
    // download: the peer said we reached the end, upload: the last chunk is sent
    bool endReached;

    // the requests on the way, of any file
    int outstanding;

    // for uploads, the chunks of the current file waiting for their reply
    struct Pending
    {
        unsigned long offset;
        unsigned len;
        bool last;
    };
    Pending pending[PIPELINE];
    int pendingCount;

    bool finished;
    bool cancelled;
    int okCount;
    int dirCount;
    int failCount;
    std::string lastError;
    unsigned long bytesDone;

    unsigned char* buf;
    char* b64;
};

#endif
