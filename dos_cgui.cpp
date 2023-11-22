// Encoding: IBM 437
#include "dos_cgui.h"
#include "dos_scrn.h"
#include "filelist.h"
#include "list.h"
#include "cldutils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* __far header_1 = "ษออออออออออออหออออออออออหออออออออออออออป";
static const char* __far header_2 = "บ    Name    บ   Size   บ     Date     บ";
static const char* __far header_3 = "ฬออออออออออออออออออออออออออออออออออออออน";
static const char* __far footer_1 = "ฬออออออออออออสอออออหออออสออออออออออออออน";
static const char* __far format_R = "บ%12sบ%10sบ%02d/%02d/%02d-%02d:%02dบ"; // the format of one line in frame
static const char* __far format_E = "บ            บ          บ              บ"; // the format of one line in frame

static const char* __far main_header_connect_screen = " cloudy (c) 2023 - 2024 fritzone                                          ";
static const char* __far main_header_connect_http_screen = "http://cloudy.sh";

static const char* __far header_1_connect_win = "ษออออออออออออออออออออออออออออออออออออออออออป";
static const char* __far header_2_connect_win = "บ    Please enter the IP of the remote     บ";
static const char* __far header_3_connect_win = "บ       host you want to connect to:       บ";
static const char* __far header_4_connect_win = "ฬออออออออออออออออออออออออออออออออออออออออออน";
static const char* __far header_5_connect_win = "บ                 .   .   .                บ";
static const char* __far header_6_connect_win = "ศออออออออออออออออออออออออออออออออออออออออออผ";
/*
 * The bottom of the frame
 */
static void __far frameBottom(int x, int fgc, int bgc, void*scrSeg)
{
  writeString(x, frameContentSize() + 3, bgc, fgc, footer_1, scrSeg);
}

/*
 * The top frame, ie. header
 */
static void __far frameTop(int x, int fgc, int bgc, void*scrSeg)
{
  writeString(x, 0, bgc, fgc, header_1, scrSeg);
  writeString(x, 1, bgc, fgc, header_2, scrSeg);
  writeString(x, 2, bgc, fgc, header_3, scrSeg);
}


/*
 * Renders the footer of the frame wit hthe given data
 */
static void __far footer(int col, int fg, int bg, LinkedList* files,
                         const std::set<char>& drives,
                         unsigned workDrive,
                         unsigned long diskFree,
                         unsigned selectedCount,
                         unsigned long selectedBytes,
                         void* scrSeg)
{
  char s[81] = {0};
  sprintf(s, "บTot:%3d   %8sบSel:%3d   %8lubบ",
             files ? files->count : 0 , files ? renderHumanReadableSize(files->size) : "0K",
             selectedCount, selectedBytes
             //renderHumanReadableSize(selectedBytes)
  );

  writeString(col, 22, bg, fg, s, scrSeg);

  // disk free status
  char dfree[81] = {0};
  sprintf(dfree, "บ                  บ  %c Free: %7s  บ",
    workDrive == 0 ? ' ' : workDrive + 'A' - 1,
    renderHumanReadableSize(diskFree)
  );

  writeString(col, 23, bg, fg, dfree, scrSeg);

  // do we have the concept of drive?
  if(workDrive != 0)
  {
    // current drive
    char cdrvstring[2] = {workDrive + 'A' - 1, 0}; // current drive ,will be highlighted
    int cdrvidx = -1;
    int i = 0;
    char drvstring[40] = {0}; // all drives

    strcpy(drvstring, "[");
    std::set<char>::iterator it = drives.begin();
    while(it != drives.end() )
    {
      char smstr[2] = {0};
      smstr[0] = *it;
      strcat(drvstring, smstr);
      it ++;

      if(workDrive + 'A' - 1 == smstr[0])
      {
        cdrvidx = i;
      }
      i++;
    }

    strcat(drvstring, "]");
    writeString(col + 1 , 23, bg, fg, drvstring, scrSeg);
    // highlight the current drive
    writeString(col + 2 + cdrvidx, 23, bg, Yellow, cdrvstring, scrSeg); // 2:it starts with |[
  }

}

static void __far renderString(char* dest, FileStructure* fs)
{
  if(fs)
  {
    sprintf(dest, format_R, fs->sname,
                fs->is_dir ? "     <DIR>" : renderHumanReadableSize(fs->file_size),
                fs->year % 100,
                fs->month, fs->day, fs->hour, fs->minute);
  }
  else
  {
    sprintf(dest, "%s", format_E);
  }
}


/*
 * Draws the left frame, the local computer
 */
void leftFrame(void* scrSeg, const char* cwd, LinkedList* files,
               const std::set<char>& drives,
               unsigned workDrive,
               unsigned long diskFree)
{
  frameTop(0, BrightWhite, Blue, scrSeg);

  // draw the current directory on top
  int cwdlen = strlen(cwd);
  writeString(20 - cwdlen / 2, 0, White, Blue, cwd, scrSeg);
  Node* q = files->displayStart;
  int ctr = 0;
  while(ctr < frameContentSize() && q)
  {
    static char rendered[64] = {0};
    FileStructure* fs =((FileStructure*)(q->data));
    renderString(rendered, fs);

    if(q != files->currentSelected)
    {
      if(fs->is_selected)
      {
        writeString(0, 3 + ctr, Blue, Yellow, rendered, scrSeg);
        writeString(0, 3 + ctr, Blue, BrightWhite, "บ", scrSeg);
        writeString(39, 3 + ctr, Blue, BrightWhite, "บ", scrSeg);
      }
      else
      {
        writeString(0, 3 + ctr, Blue, BrightWhite, rendered, scrSeg);
      }
    }
    else
    {
      writeString(0, 3 + ctr, Yellow, BrightWhite, rendered, scrSeg);
      writeString(0, 3 + ctr, Blue, BrightWhite, "บ", scrSeg);
      writeString(39, 3 + ctr, Blue, BrightWhite, "บ", scrSeg);
    }

    q = q->next;
    ctr ++;
  }

  while(ctr < frameContentSize())
  {
    static char rendered[64] = {0};
    renderString(rendered, NULL);
    writeString(0, 3 + ctr, Blue, BrightWhite, rendered, scrSeg);
    ctr ++;
  }

  files->displayEnd = q;
  frameBottom(0, BrightWhite, Blue, scrSeg);
  // identify the selected items
  unsigned selectedCount = 0;
  unsigned long selectedBytes = 0;
  Node* q1 = files->head;
  while(q1)
  {
    FileStructure* fs =((FileStructure*)(q1->data));
    if(fs->is_selected)
    {
      selectedCount ++;
      selectedBytes += fs->file_size;
    }
    q1 = q1->next;
  }
  // the footer with various infos
  footer(0, BrightWhite, Blue, files, drives, workDrive, diskFree, selectedCount, selectedBytes, scrSeg);


}

/*
 * Draws the right frame, the data from the remote computer
 */
void rightFrame(void* scrSeg, const char* rwd)
{
  frameTop(40, White, Red, scrSeg);
  frameBottom(40, White, Red, scrSeg);
  int ctr = 0;

  while(ctr < frameContentSize())
  {
    static char rendered[64] = {0};
    renderString(rendered, NULL);
    writeString(40, 3 + ctr, Red, White, rendered, scrSeg);
    ctr ++;
  }

  footer(40, White, Red, NULL, std::set<char>(), 0, 0, 0, 0, scrSeg);


}

/*
 * On a 80x25 screen
 */
int frameContentSize()
{
  return 18;
}

/*
 * The menu at the bottom of the screen
 */
void menu(void* scrSeg)
{
  writeString(0, 24, White, White, "                                                                                         ", scrSeg);
  writeString(1, 24, White, Red, "F1", scrSeg);
  writeString(3, 24, White, Black, " Help", scrSeg);
  writeString(9, 24, White, Red, "F2", scrSeg);
  writeString(11, 24, White, Black, " Drive", scrSeg);

}

void connect_window(void* scrSeg, char *ip[])
{
    writeString(0,0, Red, White, main_header_connect_screen, scrSeg);
    writeString(64,0, Red, LightYellow, main_header_connect_http_screen, scrSeg);

    writeString(18, 10, Blue, White, header_1_connect_win , scrSeg);
    writeString(18, 11, Blue, White, header_2_connect_win , scrSeg);
    writeString(18, 12, Blue, White, header_3_connect_win , scrSeg);
    writeString(18, 13, Blue, White, header_4_connect_win , scrSeg);
    writeString(18, 14, Blue, White, header_5_connect_win , scrSeg);
    writeString(18, 15, Blue, White, header_6_connect_win , scrSeg);

    writeString(33,14, Black, White, "   ", scrSeg);
    writeString(37,14, Black, White, "   ", scrSeg);
    writeString(41,14, Black, White, "   ", scrSeg);
    writeString(45,14, Black, White, "   ", scrSeg);
}
