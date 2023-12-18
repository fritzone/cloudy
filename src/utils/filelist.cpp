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


FileStructure* createFileStructure(struct find_t* fi)
{
  FileStructure* fs = (FileStructure*)malloc(sizeof(FileStructure));

  fs->is_dir = fi->attrib & _A_SUBDIR;

  DateUnion du; du.g = fi->wr_date;

  fs->year = du.d.year + 1980;
  fs->month = du.d.month;
  fs->day = du.d.day;

  TimeUnion tu; tu.g = fi->wr_time;

  fs->hour = tu.t.hrs;
  fs->minute = tu.t.mins;
  fs->sec = tu.t.sec2 * 2;

  fs->sname = (char*)calloc(strlen(fi->name) + 1, 1);
  strcpy(fs->sname, fi->name);
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
      insertAtBeginning(fls, (void*)fs);
    }
    fls->size = fls->size + fileinfo.size;
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
        insertAtBeginning(fls, (void*)fs);
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
    insertAtBeginning(fls, (void*)fs);
  }

  _dos_findclose(&fileinfo);

  }

  return fls;
}

void deleteFileStructure(void* p)
{
  free( ((FileStructure*)p)->sname );
}


