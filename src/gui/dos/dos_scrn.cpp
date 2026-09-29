#include "dos_scrn.h"

#include <string.h>
#include <i86.h>
#include <types.h>
#include <dos.h>
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

// The screen handling goes through the video BIOS, graph.lib would cost 25K

void setTextMode()
{
    union REGS r;
    r.w.ax = 0x0003; // 80x25 color text
    int86(0x10, &r, &r);
}

static void setCursorShape(unsigned short shape)
{
    union REGS r;
    r.h.ah = 0x01;
    r.w.cx = shape;
    int86(0x10, &r, &r);
}

void CursorRaii::hideCursor()
{
    setCursorShape(0x2000);
}

void CursorRaii::showCursor()
{
    setCursorShape(previousCursor);
}

void CursorRaii::placeCursor(short x, short y)
{
    union REGS r;
    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = (unsigned char)y;
    r.h.dl = (unsigned char)x;
    int86(0x10, &r, &r);
}

CursorRaii::CursorRaii()
{
    union REGS r;
    r.h.ah = 0x03;
    r.h.bh = 0;
    int86(0x10, &r, &r);
    previousCursor = r.w.cx;
    hideCursor();
}

CursorRaii::~CursorRaii()
{
    showCursor();
}


