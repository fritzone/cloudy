#ifndef _FILELIST_H_
#define _FILELIST_H_

#include "filestrc.h"

struct LinkedList;
struct find_t;


/**
 * Creates a file list from the current working directory
 */
LinkedList* createFileList(const char* cwd);

/**
 * Create a filestructure
 */
FileStructure* createFileStructure(struct find_t* fi);

/**
 * Will free the memory allocated to the sname of the file structure
 */
void deleteFileStructure(void*);


#endif


