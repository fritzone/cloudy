#include "filelist.h"
#include "list.h"

#include <stdio.h>
#include <io.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>

typedef union
{
  struct
  {
    unsigned short day:5;
    unsigned short month:4;
    unsigned short year:7;
  } d;
  unsigned short g;
} DateUnion;

typedef union
{
  struct
  {
    unsigned short sec2:5;
    unsigned short mins:6;
    unsigned short hrs:5;
  } t;
  unsigned short g;
} TimeUnion;


FileStructure* newFileStructure(const char* name, const char* hash)
{
  // one block: the structure, the name, then the hash
  size_t nameLen = strlen(name) + 1;
  size_t hashLen = hash ? strlen(hash) + 1 : 0;
  FileStructure* fs = (FileStructure*)calloc(1, sizeof(FileStructure) + nameLen + hashLen);
  if(fs == NULL)
  {
    return NULL;
  }

  fs->sname = (char*)(fs + 1);
  memcpy(fs->sname, name, nameLen);
  if(hash)
  {
    fs->hash = fs->sname + nameLen;
    memcpy(fs->hash, hash, hashLen);
  }
  return fs;
}

FileStructure* createFileStructure(struct find_t* fi)
{
  FileStructure* fs = newFileStructure(fi->name, NULL);
  if(fs == NULL)
  {
    return NULL;
  }

  fs->is_dir = fi->attrib & _A_SUBDIR;

  DateUnion du; du.g = fi->wr_date;

  fs->year = du.d.year + 1980;
  fs->month = du.d.month;
  fs->day = du.d.day;

  TimeUnion tu; tu.g = fi->wr_time;

  fs->hour = tu.t.hrs;
  fs->minute = tu.t.mins;
  fs->sec = tu.t.sec2 * 2;

  fs->file_size = fi->size;
  fs->is_selected = false;

  return fs;
}

LinkedList* createFileList(const char* cwd)
{
  struct find_t fileinfo;
  struct LinkedList* fls = createLinkedList();
  // files
  {
  int handle = _dos_findfirst("*.*", _A_NORMAL | _A_SUBDIR, &fileinfo);
  int rc = handle;
  while(rc == 0)
  {
    if(! (fileinfo.attrib & _A_SUBDIR))
    {
      FileStructure* fs = createFileStructure(&fileinfo);
      if(fs == NULL || !insertAtBeginning(fls, (void*)fs))
      {
        if(fs) deleteFileStructure(fs);
        break;
      }
      fls->size = fls->size + fileinfo.size;
    }
    rc = _dos_findnext(&fileinfo);
  }
  _dos_findclose(&fileinfo);
  }

  // directories
  {
  find_t parentDir;
  int handle = _dos_findfirst("*.*", _A_NORMAL | _A_SUBDIR, &fileinfo);
  int rc = handle;
  while(rc == 0)
  {
    if(fileinfo.attrib & _A_VOLID)
    {
        rc = _dos_findnext(&fileinfo);
        continue;
    }

    if(fileinfo.attrib & _A_SUBDIR)
    {
      if(fileinfo.name[0] != '.')
      {
        FileStructure* fs = createFileStructure(&fileinfo);
        if(fs && !insertAtBeginning(fls, (void*)fs))
        {
          deleteFileStructure(fs);
        }
      }

      if(!strcmp(fileinfo.name, ".."))
      {
       memcpy(&parentDir, &fileinfo, sizeof(parentDir));
      }
    }

    rc = _dos_findnext(&fileinfo);
  }
  // the ".." at the beginning only if we are not in the root directory
  if(strlen(cwd) > 3)
  {
    FileStructure* fs = createFileStructure(&parentDir);
    if(fs && !insertAtBeginning(fls, (void*)fs))
    {
      deleteFileStructure(fs);
    }
  }

  _dos_findclose(&fileinfo);

  }

  return fls;
}

void deleteFileStructure(void* p)
{
  free(p);
}


