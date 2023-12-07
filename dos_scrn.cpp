#include "dos_scrn.h"

#include <string.h>
#include <i86.h>
#include <types.h>
#include <dos.h>
#include <graph.h>
#include <conio.h>

short CursorRaii::previousCursor = 0;


static const int ROWS = 25;
static const int COLS = 80;

void clearscr(void* scrSeg)
{
    int i = 0;
    for(;i<ROWS * COLS; i++) { *((uint16_t*)scrSeg  + i) = 0x0720;} // color, character
}

void flip(void* scrSeg)
{
    void __far * screen = MK_FP(0xb800, 0);
    int i = 0;
    for(;i<ROWS * COLS * 2; i++) { *((char*)screen  + i) = *((char*)scrSeg + i);}
}

void writeChar(int x, int y, char bck, char col, char chh, void* scrSeg)
{
    *((char*)scrSeg + (y * COLS + x) * 2) = chh;
    *((char*)scrSeg + (y * COLS + x) * 2 + 1) = bck << 4 | col ;
}

void writeString(int x, int y, char bck, char col, const char* s, void* seg)
{
    int l = strlen(s);
    for(int i=0; i<l; i++)
    {
        writeChar(x+i, y, bck, col, s[i], seg);
    }
}

void CursorRaii::hideCursor()
{
    _settextcursor(0x2000);
}

void CursorRaii::showCursor()
{
    _settextcursor(previousCursor);
}

void CursorRaii::placeCursor(short x, short y)
{
    _settextposition(y + 1, x + 1);
}

CursorRaii::CursorRaii()
{
    previousCursor = _gettextcursor();
    hideCursor();
}

CursorRaii::~CursorRaii()
{
    showCursor();
}


