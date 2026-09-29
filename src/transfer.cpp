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

extern ProtocolImpl p;

// The size of a piece of file in one message, before base64
#define CHUNK_SIZE 4096

Transfer::Transfer(Direction d, const std::vector<Item>& p_items, const std::string& p_remoteDirHash) :
    dir(d), items(p_items), remoteDirHash(p_remoteDirHash),
    current(0), fp(NULL), offset(0), nextOffset(0), endReached(false), outstanding(0), pendingCount(0),
    finished(false), cancelled(false), okCount(0), failCount(0), done(0), total(0)
{
    memset(localName, 0, sizeof(localName));
    for(size_t i = 0; i < items.size(); i++)
    {
        total += items[i].size;
    }

    buf = (unsigned char*)malloc(CHUNK_SIZE);
    b64 = (char*)malloc(BASE64_ENCODED_LEN(CHUNK_SIZE) + 1);
    if(buf == NULL || b64 == NULL)
    {
        fail("Out of memory");
    }
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
    failCount += items.size() - current;
    current = items.size();
    finished = true;
}

void Transfer::step()
{
    if(finished)
    {
        return;
    }

    if(current >= items.size())
    {
        finished = true;
        return;
    }

    if(fp == NULL && !startItem())
    {
        return;
    }

    // keep the pipe full
    while(!finished && fp != NULL && outstanding < PIPELINE)
    {
        bool sent = dir == Download ? sendReadRequest() : sendWriteChunk();
        if(!sent)
        {
            break;
        }
    }
}

bool Transfer::startItem()
{
    const Item& it = items[current];
    offset = 0;
    nextOffset = 0;
    endReached = false;
    pendingCount = 0;

    if(dir == Download)
    {
        makeDosName(it.name.c_str(), localName);

        // two long names might give the same DOS name, keep them apart
        for(int n = 1; ; n++)
        {
            bool used = false;
            for(size_t i = 0; i < usedNames.size(); i++)
            {
                if(usedNames[i] == localName) used = true;
            }
            if(!used) break;

            char base[13] = {0};
            makeDosName(it.name.c_str(), base);
            char* dot = strchr(base, '.');
            char ext[5] = {0};
            if(dot) { strcpy(ext, dot); *dot = 0; }
            char suffix[8];
            sprintf(suffix, "~%d", n);
            base[8 - strlen(suffix)] = 0;
            sprintf(localName, "%s%s%s", base, suffix, ext);
        }
        usedNames.push_back(localName);

        fp = fopen(localName, "wb");
        if(fp == NULL)
        {
            finishItem(false, std::string("Cannot create ") + localName);
            return false;
        }
    }
    else
    {
        // Linux folks like their file names in lowercase
        remoteName = it.name;
        for(size_t i = 0; i < remoteName.length(); i++)
        {
            remoteName[i] = tolower(remoteName[i]);
        }

        strncpy(localName, it.name.c_str(), 12);
        fp = fopen(localName, "rb");
        if(fp == NULL)
        {
            finishItem(false, std::string("Cannot open ") + localName);
            return false;
        }
    }

    log_info() << "Copying" << it.name << "as" << localName;
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
        log_error() << "Copying" << items[current].name << "failed:" << error;
        failCount++;
        lastError = error;
        if(dir == Download && localName[0])
        {
            remove(localName);
        }
    }

    // count the whole file as done, even if it changed size meanwhile
    done = 0;
    for(size_t i = 0; i <= current; i++)
    {
        done += items[i].size;
    }

    current++;
    pendingCount = 0;
    if(current >= items.size())
    {
        finished = true;
    }
    requestRepaint();
}

bool Transfer::sendReadRequest()
{
    // up to the size we know, but further if the file grew meanwhile
    if(endReached || (nextOffset >= items[current].size && nextOffset != offset))
    {
        return false;
    }

    FileReadRequest* r = p.create_FileReadRequest(items[current].hash, (long)nextOffset, CHUNK_SIZE);
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
    if(finished || fp == NULL || fd->get_file_hash() != items[current].hash || (unsigned long)fd->get_offset() != offset)
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
        finishItem(false, std::string("Cannot write ") + localName + ", disk full?");
        return;
    }

    offset += n;
    done += n;

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
        finishItem(false, std::string("Cannot read ") + localName);
        return false;
    }

    base64_encode(buf, n, b64);
    bool last = n < CHUNK_SIZE || nextOffset + n >= items[current].size;

    FileWriteRequest* r = p.create_FileWriteRequest(remoteDirHash, remoteName, (long)nextOffset, b64, last);
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
    if(finished || fp == NULL || pendingCount == 0 || r->get_name() != remoteName
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
    done += piece.len;

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
        if(dir == Download)
        {
            remove(localName);
        }
    }
    cancelled = true;
    finished = true;
}

std::string Transfer::title() const
{
    char t[64];
    sprintf(t, "%s %u of %u", dir == Download ? "Downloading" : "Uploading",
            (unsigned)(current < items.size() ? current + 1 : items.size()), (unsigned)items.size());
    return t;
}

std::string Transfer::currentLine() const
{
    if(current >= items.size())
    {
        return "";
    }
    if(dir == Download)
    {
        return items[current].name + " -> " + localName;
    }
    return items[current].name;
}

std::string Transfer::summary() const
{
    char s[80];
    if(cancelled)
    {
        sprintf(s, "Cancelled, %d file(s) copied", okCount);
        return s;
    }
    if(failCount == 0)
    {
        sprintf(s, "%d file(s) copied", okCount);
        return s;
    }
    sprintf(s, "%d copied, %d failed: ", okCount, failCount);
    return s + lastError;
}
