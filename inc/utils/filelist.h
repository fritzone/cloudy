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
 * Creates an empty file structure with the given name and hash (which can be NULL),
 * everything in one memory block. Returns NULL if there is no memory.
 */
FileStructure* newFileStructure(const char* name, const char* hash);

/**
 * Create a filestructure
 */
FileStructure* createFileStructure(struct find_t* fi);

/**
 * Will free the memory allocated to the file structure
 */
void deleteFileStructure(void*);


#endif


