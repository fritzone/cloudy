#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <types.h>

/**
 * Clears the screen at the given segment
 */
void clearscr(void * scrSeg);

/**
 * Will flipe the screen from the given segment to the real screen
 */
void flip(void* scrSeg);

/**
 * Writes a character (chh) at (x,y) with the background and col to the given segment
 */
void writeChar(int x, int y, char bck, char col, char chh, void* scrSeg);

/**
 * Writes the given string to the given position using background and color
 */
void writeString(int x, int y, char bck, char col, const char* s, void* seg);

/**
 * Class responsible for hiding the cursor upon start and displaying it when exiting
 * or meanwhile.
 */
class CursorRaii
{
public:

  CursorRaii();
  ~CursorRaii();

  /**
   * Will hide the command cursor
   */
  static void hideCursor();

  /**
   * Will show the command cursor
   */
  static void showCursor();

  /**
   * Will put the cursos to the specific location
   */
  static void placeCursor(short x, short y);

private:
  static short previousCursor;
};

#endif

