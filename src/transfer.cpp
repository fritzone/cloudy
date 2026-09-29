#include "transfer.h"
#include "prot.h"
#include "base64.h"
#include "cldutils.h"
#include "guistate.h"
#include "strngify.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <direct.h>

extern ProtocolImpl p;

// The size of a piece of file in one message, before base64
#define CHUNK_SIZE 4096

// DOS limits: a directory path is at most 66 characters (with the drive), a file path 79
#define MAX_DIR_PATH 66
#define MAX_FILE_PATH 79

// Every local directory has at most this many entries copied
#define MAX_LOCAL_ENTRIES 1000

static std::string joinPath(const std::string& dir, const std::string& name)
{
    if(!dir.empty() && dir[dir.length() - 1] == '\\')
    {
        return dir + name;
    }
    return dir + "\\" + name;
}

// Linux folks like their file names in lowercase
static std::string lowercase(const std::string& s)
{
    std::string r = s;
    for(size_t i = 0; i < r.length(); i++)
    {
        r[i] = tolower(r[i]);
    }
    return r;
}

Transfer::Transfer(Direction d, const std::vector<Item>& items, const std::string& remoteDirHash) :
    dir(d), waiting(WaitNothing), fp(NULL), offset(0), nextOffset(0), endReached(false),
    outstanding(0), pendingCount(0), finished(false), cancelled(false),
    okCount(0), dirCount(0), failCount(0), bytesDone(0)
{
    cur.size = 0;
    cur.isDir = false;

    buf = (unsigned char*)malloc(CHUNK_SIZE);
    b64 = (char*)malloc(BASE64_ENCODED_LEN(CHUNK_SIZE) + 1);
    if(buf == NULL || b64 == NULL)
    {
        fail("Out of memory");
        return;
    }

    // no reallocation of the levels later
    levels.reserve(MAX_DEPTH + 1);

    // the selection is the first level
    char cwd[PATH_MAX + 1] = {0};
    getcwd(cwd, PATH_MAX);
    pushLevel(cwd, remoteDirHash);
    levels.back().entries = items;
    levels.back().listed = levels.back().total = items.size();
}

Transfer::~Transfer()
{
    if(!finished)
    {
        cancel();
    }
    free(buf);
    free(b64);
}

void Transfer::fail(const std::string& error)
{
    lastError = error;
    failCount++;
    finished = true;
}

void Transfer::pushLevel(const std::string& localDir, const std::string& remoteDirHash)
{
    levels.push_back(Level());
    Level& l = levels.back();
    l.localDir = localDir;
    l.remoteDirHash = remoteDirHash;
    l.next = 0;
    l.listed = 0;
    l.total = -1;
}

void Transfer::step()
{
    if(finished || waiting != WaitNothing)
    {
        return;
    }

    // a file is being copied: keep the pipe full
    if(fp != NULL)
    {
        while(!finished && fp != NULL && outstanding < PIPELINE)
        {
            bool sent = dir == Download ? sendReadRequest() : sendWriteChunk();
            if(!sent)
            {
                break;
            }
        }
        return;
    }

    // the next thing to copy
    while(!levels.empty())
    {
        Level& l = levels.back();

        if(l.next < l.entries.size())
        {
            Item it = l.entries[l.next++];
            if(it.name == ".." || it.name == ".")
            {
                continue;
            }
            bool started = it.isDir ? startDirectory(it) : startFile(it);
            if(started)
            {
                return;
            }
            continue;
        }

        // the next page of a remote directory
        if(dir == Download && l.total >= 0 && l.listed < l.total)
        {
            l.entries.clear();
            l.next = 0;
            DirectoryListRequest* r = p.create_DirectoryListRequest(l.remoteDirHash, l.listed);
            bool sent = p.send(r);
            delete r;
            if(!sent)
            {
                fail("Cannot send to the peer");
                return;
            }
            waiting = WaitListing;
            return;
        }

        levels.pop_back();
    }

    finished = true;
    requestRepaint();
}

static bool contains(const std::vector<std::string>& v, const char* s)
{
    for(size_t i = 0; i < v.size(); i++)
    {
        if(v[i] == s) return true;
    }
    return false;
}

std::string Transfer::uniqueDosName(Level& level, const std::string& longName)
{
    char name[13];
    makeDosName(longName.c_str(), name);
    if(!contains(level.usedNames, name))
    {
        level.usedNames.push_back(name);
        return name;
    }

    // two long names give the same DOS name: NAME~1.EXT, NAME~2.EXT, ...
    // continuing from the last ~N given to this name
    size_t si = 0;
    while(si < level.suffixNames.size() && level.suffixNames[si] != name) si++;
    if(si == level.suffixNames.size())
    {
        level.suffixNames.push_back(name);
        level.suffixes.push_back(0);
    }

    char base[13];
    char ext[5] = {0};
    strcpy(base, name);
    char* dot = strchr(base, '.');
    if(dot) { strcpy(ext, dot); *dot = 0; }

    for(int n = level.suffixes[si] + 1; ; n++)
    {
        char suffix[8];
        char shortBase[9];
        sprintf(suffix, "~%d", n);
        strncpy(shortBase, base, 8 - strlen(suffix));
        shortBase[8 - strlen(suffix)] = 0;
        sprintf(name, "%s%s%s", shortBase, suffix, ext);
        if(!contains(level.usedNames, name))
        {
            level.usedNames.push_back(name);
            level.suffixes[si] = n;
            return name;
        }
    }
}

bool Transfer::startFile(const Item& it)
{
    Level& l = levels.back();
    cur = it;
    offset = 0;
    nextOffset = 0;
    endReached = false;
    pendingCount = 0;

    if(dir == Download)
    {
        localPath = joinPath(l.localDir, uniqueDosName(l, it.name));
        if(localPath.length() > MAX_FILE_PATH)
        {
            localPath = "";
            finishItem(false, "Path too long: " + it.name);
            return false;
        }
        fp = fopen(localPath.c_str(), "wb");
        if(fp == NULL)
        {
            finishItem(false, "Cannot create " + localPath);
            return false;
        }
    }
    else
    {
        curRemoteDir = l.remoteDirHash;
        remoteName = lowercase(it.name);
        localPath = joinPath(l.localDir, it.name);
        fp = fopen(localPath.c_str(), "rb");
        if(fp == NULL)
        {
            std::string path = localPath;
            localPath = "";
            finishItem(false, "Cannot open " + path);
            return false;
        }
    }

    log_info() << "Copying" << it.name << "as" << localPath;
    requestRepaint();

    // the pieces go from the next step
    return true;
}

void Transfer::finishItem(bool ok, const std::string& error)
{
    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }

    if(ok)
    {
        okCount++;
    }
    else
    {
        log_error() << "Copying" << cur.name << "failed:" << error;
        failCount++;
        lastError = error;
        if(dir == Download && !localPath.empty())
        {
            remove(localPath.c_str());
        }
    }

    pendingCount = 0;
    offset = 0;
    requestRepaint();
}

void Transfer::failDirectory(const std::string& name, const std::string& error)
{
    log_error() << "Copying the directory" << name << "failed:" << error;
    failCount++;
    lastError = error;
    requestRepaint();
}

bool Transfer::startDirectory(const Item& it)
{
    Level& l = levels.back();

    if(levels.size() > MAX_DEPTH)
    {
        failDirectory(it.name, "Too deep: " + it.name);
        return false;
    }

    if(dir == Download)
    {
        std::string path = joinPath(l.localDir, uniqueDosName(l, it.name));
        if(path.length() > MAX_DIR_PATH)
        {
            failDirectory(it.name, "Path too long: " + it.name);
            return false;
        }

        // an existing directory is fine, the files go into it
        unsigned attr = 0;
        if(_dos_getfileattr(path.c_str(), &attr) == 0)
        {
            if(!(attr & _A_SUBDIR))
            {
                failDirectory(it.name, "A file is in the way: " + path);
                return false;
            }
        }
        else if(mkdir(path.c_str()) != 0)
        {
            failDirectory(it.name, "Cannot create " + path);
            return false;
        }

        std::string hash = it.hash;
        pushLevel(path, hash);
        dirCount++;

        DirectoryListRequest* r = p.create_DirectoryListRequest(hash, 0);
        bool sent = p.send(r);
        delete r;
        if(!sent)
        {
            fail("Cannot send to the peer");
            return true;
        }
        waiting = WaitListing;
        requestRepaint();
        return true;
    }

    // upload: create it on the remote side first, its hash comes in the reply
    pendingDirLocal = joinPath(l.localDir, it.name);
    pendingDirRemote = lowercase(it.name);

    MakeDirectoryRequest* r = p.create_MakeDirectoryRequest(l.remoteDirHash, pendingDirRemote);
    bool sent = p.send(r);
    delete r;
    if(!sent)
    {
        fail("Cannot send to the peer");
        return true;
    }
    waiting = WaitMakeDir;
    requestRepaint();
    return true;
}

bool Transfer::onDirectoryList(const DirectoryList* dl)
{
    if(waiting != WaitListing || levels.empty())
    {
        return false;
    }

    Level& l = levels.back();
    if(dl->get_directory_hash() != l.remoteDirHash || dl->get_start() != l.listed)
    {
        return false;
    }
    waiting = WaitNothing;

    if(!dl->get_error().empty())
    {
        failDirectory(l.localDir, dl->get_error());
        levels.pop_back();
        return true;
    }

    const std::vector<DirectoryEntry>& entries = dl->get_entries();
    for(size_t i = 0; i < entries.size(); i++)
    {
        const DirectoryEntry& e = entries[i];
        if(e.get_name() == ".." || e.get_name() == ".")
        {
            continue;
        }
        Item it;
        it.name = e.get_name();
        it.hash = e.get_hash();
        it.size = e.get_size();
        it.isDir = e.get_attrs().find('d') != std::string::npos;
        l.entries.push_back(it);
    }

    l.listed += entries.size();
    l.total = dl->get_total();
    if(entries.empty())
    {
        // nothing more comes
        l.total = l.listed;
    }
    return true;
}

bool Transfer::readLocalDirectory(Level& level)
{
    struct find_t fi;
    std::string pattern = joinPath(level.localDir, "*.*");
    unsigned rc = _dos_findfirst(pattern.c_str(), _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR | _A_ARCH, &fi);

    while(rc == 0)
    {
        if(!(fi.attrib & _A_VOLID) && strcmp(fi.name, ".") && strcmp(fi.name, ".."))
        {
            if(level.entries.size() >= MAX_LOCAL_ENTRIES)
            {
                failDirectory(level.localDir, "Too many files in " + level.localDir);
                break;
            }
            Item it;
            it.name = fi.name;
            it.size = fi.size;
            it.isDir = (fi.attrib & _A_SUBDIR) != 0;
            level.entries.push_back(it);
        }
        rc = _dos_findnext(&fi);
    }
    _dos_findclose(&fi);

    level.listed = level.total = level.entries.size();
    return true;
}

void Transfer::onMakeDirectoryReply(const MakeDirectoryReply* r)
{
    if(waiting != WaitMakeDir || r->get_name() != pendingDirRemote)
    {
        log_debug() << "Unexpected MakeDirectoryReply, ignored";
        return;
    }
    waiting = WaitNothing;

    if(!r->get_ok())
    {
        failDirectory(pendingDirRemote, r->get_error());
        return;
    }

    pushLevel(pendingDirLocal, r->get_hash());
    readLocalDirectory(levels.back());
    dirCount++;
    requestRepaint();
}

bool Transfer::sendReadRequest()
{
    // up to the size we know, but further if the file grew meanwhile
    if(endReached || (nextOffset >= cur.size && nextOffset != offset))
    {
        return false;
    }

    FileReadRequest* r = p.create_FileReadRequest(cur.hash, (long)nextOffset, CHUNK_SIZE);
    bool sent = p.send(r);
    delete r;

    if(!sent)
    {
        finishItem(false, "Cannot send to the peer");
        fail("Cannot send to the peer");
        return false;
    }

    nextOffset += CHUNK_SIZE;
    outstanding++;
    return true;
}

void Transfer::onFileData(const FileData* fd)
{
    if(outstanding > 0)
    {
        outstanding--;
    }

    // the replies to the requests beyond the end of an already finished file end up here too
    if(finished || fp == NULL || dir != Download || fd->get_file_hash() != cur.hash || (unsigned long)fd->get_offset() != offset)
    {
        log_debug() << "Stale FileData, ignored";
        return;
    }

    if(!fd->get_error().empty())
    {
        finishItem(false, fd->get_error());
        return;
    }

    long n = base64_decode(fd->get_data().c_str(), buf, CHUNK_SIZE);
    if(n < 0)
    {
        finishItem(false, "Too much data from the peer");
        return;
    }

    if(n > 0 && fwrite(buf, 1, (size_t)n, fp) != (size_t)n)
    {
        finishItem(false, "Cannot write " + localPath + ", disk full?");
        return;
    }

    offset += n;
    bytesDone += n;

    if(fd->get_eof())
    {
        endReached = true;
        finishItem(true, "");
    }
    else if(n < CHUNK_SIZE)
    {
        // a short piece which is not the end: ask again from where we are
        nextOffset = offset;
    }
}

bool Transfer::sendWriteChunk()
{
    if(endReached)
    {
        return false;
    }

    size_t n = fread(buf, 1, CHUNK_SIZE, fp);
    if(ferror(fp))
    {
        finishItem(false, "Cannot read " + localPath);
        return false;
    }

    base64_encode(buf, n, b64);
    bool last = n < CHUNK_SIZE || nextOffset + n >= cur.size;

    FileWriteRequest* r = p.create_FileWriteRequest(curRemoteDir, remoteName, (long)nextOffset, b64, last);
    bool sent = p.send(r);
    delete r;

    if(!sent)
    {
        finishItem(false, "Cannot send to the peer");
        fail("Cannot send to the peer");
        return false;
    }

    pending[pendingCount].offset = nextOffset;
    pending[pendingCount].len = n;
    pending[pendingCount].last = last;
    pendingCount++;

    nextOffset += n;
    endReached = last;
    outstanding++;
    return true;
}

void Transfer::onFileWriteReply(const FileWriteReply* r)
{
    if(outstanding > 0)
    {
        outstanding--;
    }

    // after a failed piece the replies to the ones sent after it end up here too
    if(finished || fp == NULL || dir != Upload || pendingCount == 0 || r->get_name() != remoteName
       || (unsigned long)r->get_offset() != pending[0].offset)
    {
        log_debug() << "Stale FileWriteReply, ignored";
        return;
    }

    Pending piece = pending[0];
    for(int i = 1; i < pendingCount; i++)
    {
        pending[i - 1] = pending[i];
    }
    pendingCount--;

    if(!r->get_ok())
    {
        finishItem(false, r->get_error());
        return;
    }

    offset += piece.len;
    bytesDone += piece.len;

    if(piece.last)
    {
        finishItem(true, "");
    }
}

void Transfer::cancel()
{
    if(finished)
    {
        return;
    }

    if(fp)
    {
        fclose(fp);
        fp = NULL;
        if(dir == Download && !localPath.empty())
        {
            remove(localPath.c_str());
        }
    }
    cancelled = true;
    finished = true;
}

std::string Transfer::title() const
{
    char t[64];
    sprintf(t, "%s, %d file(s) so far", dir == Download ? "Downloading" : "Uploading", okCount);
    return t;
}

std::string Transfer::currentLine() const
{
    std::string line;
    if(fp == NULL)
    {
        line = waiting == WaitNothing ? "" : "Reading the directories...";
    }
    else if(dir == Download)
    {
        line = cur.name + " -> " + localPath;
    }
    else
    {
        line = localPath;
    }

    // the end is the interesting part
    if(line.length() > 48)
    {
        line = "..." + line.substr(line.length() - 45);
    }
    return line;
}

std::string Transfer::summary() const
{
    char s[80];
    if(cancelled)
    {
        sprintf(s, "Cancelled after %d file(s)", okCount);
        return s;
    }
    if(dirCount > 0)
    {
        sprintf(s, "%d file(s), %d dir(s) copied", okCount, dirCount);
    }
    else
    {
        sprintf(s, "%d file(s) copied", okCount);
    }
    if(failCount == 0)
    {
        return s;
    }
    sprintf(s + strlen(s), ", %d failed: ", failCount);
    return s + lastError;
}
