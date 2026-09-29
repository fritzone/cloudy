#ifndef _TRANSFER_H_
#define _TRANSFER_H_

#include <stdio.h>
#include <string>
#include <vector>

class FileData;
class FileWriteReply;

/**
 * Copies a list of files between the local directory and a remote one.
 *
 * step() keeps up to PIPELINE requests on the way, the replies arrive (in order)
 * through onFileData / onFileWriteReply. Downloads go into the current local
 * directory, with DOS 8.3 names.
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
        // the hash of the remote file, for downloads
        std::string hash;
        unsigned long size;
    };

    Transfer(Direction d, const std::vector<Item>& items, const std::string& remoteDirHash);
    ~Transfer();

    /**
     * Moves the transfer forward, call it from the main loop
     */
    void step();

    void onFileData(const FileData* fd);
    void onFileWriteReply(const FileWriteReply* r);

    /**
     * Stops the transfer, a partially downloaded file is removed
     */
    void cancel();

    bool isFinished() const { return finished; }
    Direction direction() const { return dir; }

    // for the progress window
    std::string title() const;
    std::string currentLine() const;
    unsigned long doneBytes() const { return done; }
    unsigned long totalBytes() const { return total; }

    /**
     * What happened, once the transfer is finished
     */
    std::string summary() const;

private:

    // how many requests can be on the way at once
    enum { PIPELINE = 2 };

    bool startItem();
    void finishItem(bool ok, const std::string& error);
    bool sendReadRequest();
    bool sendWriteChunk();
    void fail(const std::string& error);

private:

    Direction dir;
    std::vector<Item> items;
    std::string remoteDirHash;

    // the item being copied
    size_t current;
    FILE* fp;
    char localName[13];
    std::vector<std::string> usedNames;

    // the name of the file on the remote side, for uploads
    std::string remoteName;

    // confirmed: written (download) or acknowledged (upload) up to here
    unsigned long offset;
    // the next request goes for this offset
    unsigned long nextOffset;
    // download: the peer said we reached the end, upload: the last chunk is sent
    bool endReached;

    // the requests on the way, of any item
    int outstanding;

    // for uploads, the chunks of the current item waiting for their reply
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
    int failCount;
    std::string lastError;

    unsigned long done;
    unsigned long total;

    unsigned char* buf;
    char* b64;
};

#endif
