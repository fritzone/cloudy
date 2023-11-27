#ifndef _CGUI_H_
#define _CGUI_H_

#include <set>

struct LinkedList;

enum Colors
{
 Black      = 0,
 Blue       = 1,
 Green      = 2,
 Aqua       = 3,
 Red        = 4,
 Purple     = 5,
 Yellow     = 6,
 White      = 7,
 Gray       = 8,
 LightBlue  = 9,
 LightGreen = 10,
 LightAqua  = 11,
 LightRed   = 12,
 LightPurple= 13,
 LightYellow= 14,
 BrightWhite= 15
};

/**
 * Will return how many rows are in a frame
 */
int frameContentSize();

/**
 * Will draw the left frame with the files from the current directory
 */
void leftFrame(void* scrSeg, const char* cwd, LinkedList* files,
               const std::set<char>& drives,
               unsigned workDrive,
               unsigned long diskFree);

/**
 * Will draw the right frame with the files from the remote machine's actual directory
 */
void rightFrame(void* scrSeg, const char* rwd);

/**
 * Draws the menu of the application, bottom line
 */
void menu(void* scrSeg);

/**
 * The windows shown when the application tries to connect to an IP
 */
void connect_window(void* scrSeg, char *ip[4], int error, const char *errorText);

#endif


