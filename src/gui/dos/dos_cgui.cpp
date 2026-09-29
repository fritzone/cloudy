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
static const char* __far header_3 = "ฬออออออออออออฮออออออออออฮออออออออออออออน";
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
static const char* __far footer_err_1_connect = "ฬออออออออออออออออออออออออออออออออออออออออออน";
static const char* __far footer_err_2_connect = "บ                                          บ";

static const char* __far header_1_pwd_win = "ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป";
static const char* __far header_2_pwd_win = "บ Please enter the required credentials to access the host บ";
static const char* __far header_3_pwd_win = "ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน";
static const char* __far contnt_1_pwd_win = "บ Username:                                                บ";
static const char* __far contnt_2_pwd_win = "บ Password:                                                บ";
static const char* __far err_1_pwd__contn = "ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน";
static const char* __far err_2_pwd__contn = "บ                                                          บ";
static const char* __far footer_pwd_windw = "ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ";

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
 * Writes the title centered on the top of the frame, if it is too long only its end is shown
 */
static void __far frameTitle(int x, int fgc, int bgc, const char* title, void* scrSeg)
{
  char t[48] = {0};
  int len = strlen(title);
  if(len > 36)
  {
    strcpy(t, "...");
    strcat(t, title + len - 33);
    len = 36;
  }
  else
  {
    strcpy(t, title);
  }
  writeString(x + 20 - len / 2, 0, bgc, fgc, t, scrSeg);
}


/*
 * Renders the footer of the frame wit hthe given data
 */
static void __far footer(int col, int fg, int bg, LinkedList* files,
                         const std::set<char>& drives,
                         unsigned workDrive,
                         const char* freeStr,
                         unsigned selectedCount,
                         unsigned long selectedBytes,
                         void* scrSeg)
{
  char s[128] = {0};
  sprintf(s, "\xBATot:%3d   %8s\xBASel:%3d   %9s\xBA",
             files ? files->count : 0 , renderHumanReadableSize(files ? (unsigned long)files->size : 0),
             selectedCount, "");
  writeString(col, 22, bg, fg, s, scrSeg);
  // the static buffer of renderHumanReadableSize is reused, so this goes separately
  sprintf(s, "%9s", renderHumanReadableSize(selectedBytes));
  writeString(col + 30, 22, bg, fg, s, scrSeg);

  // disk free status
  char dfree[128] = {0};
  sprintf(dfree, "\xBA                  \xBA  %c Free: %7s  \xBA",
    workDrive == 0 ? ' ' : workDrive + 'A' - 1,
    freeStr
  );

  writeString(col, 23, bg, fg, dfree, scrSeg);

  // do we have the concept of drive?
  if(workDrive != 0)
  {
    // current drive
    char cdrvstring[2] = {workDrive + 'A' - 1, 0}; // current drive ,will be highlighted
    int cdrvidx = -1;
    int i = 0;
    char drvstring[128] = {0}; // all drives

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
    // long (remote) names do not fit, show their beginning with a ~ at the end
    char name[13] = {0};
    strncpy(name, fs->sname, 12);
    if(strlen(fs->sname) > 12)
    {
      name[11] = '~';
    }

    sprintf(dest, format_R, name,
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
 * Draws the rows of the files in a frame, and returns the selected count and size
 */
static void __far drawRows(void* scrSeg, int col, int fg, int bg, LinkedList* files, bool focused,
                           unsigned* selectedCount, unsigned long* selectedBytes)
{
  Node* q = files ? files->displayStart : NULL;
  int ctr = 0;
  while(ctr < frameContentSize() && q)
  {
    char rendered[128] = {0};
    FileStructure* fs =((FileStructure*)(q->data));
    renderString(rendered, fs);

    if(q != files->currentSelected || !focused)
    {
      writeString(col, 3 + ctr, bg, fs->is_selected ? Yellow : fg, rendered, scrSeg);
    }
    else
    {
      writeString(col, 3 + ctr, Aqua, fs->is_selected ? Yellow : Black, rendered, scrSeg);
    }
    writeString(col, 3 + ctr, bg, fg, "\xBA", scrSeg);
    writeString(col + 39, 3 + ctr, bg, fg, "\xBA", scrSeg);

    q = q->next;
    ctr ++;
  }

  while(ctr < frameContentSize())
  {
    char rendered[128] = {0};
    renderString(rendered, NULL);
    writeString(col, 3 + ctr, bg, fg, rendered, scrSeg);
    ctr ++;
  }

  if(files)
  {
    files->displayEnd = q;
  }

  // identify the selected items
  *selectedCount = 0;
  *selectedBytes = 0;
  Node* q1 = files ? files->head : NULL;
  while(q1)
  {
    FileStructure* fs =((FileStructure*)(q1->data));
    if(fs->is_selected)
    {
      (*selectedCount) ++;
      *selectedBytes += fs->file_size;
    }
    q1 = q1->next;
  }
}


/*
 * Draws the left frame, the local computer
 */
void leftFrame(void* scrSeg, const char* cwd, LinkedList* files,
               const std::set<char>& drives,
               unsigned workDrive,
               unsigned long diskFree,
               bool focused)
{
  frameTop(0, BrightWhite, Blue, scrSeg);
  frameTitle(0, focused ? Black : White, focused ? Aqua : Blue, cwd, scrSeg);

  unsigned selectedCount = 0;
  unsigned long selectedBytes = 0;
  drawRows(scrSeg, 0, BrightWhite, Blue, files, focused, &selectedCount, &selectedBytes);

  frameBottom(0, BrightWhite, Blue, scrSeg);
  // the footer with various infos
  char freeStr[16] = {0};
  strcpy(freeStr, renderHumanReadableSize(diskFree));
  footer(0, BrightWhite, Blue, files, drives, workDrive, freeStr, selectedCount, selectedBytes, scrSeg);
}

/*
 * Draws the right frame, the data from the remote computer
 */
void rightFrame(void* scrSeg, const char* rwd, LinkedList* files, bool focused,
                unsigned long freeKB, const char* status)
{
  frameTop(40, BrightWhite, Red, scrSeg);
  frameTitle(40, focused ? Black : White, focused ? Aqua : Red, rwd ? rwd : "", scrSeg);

  unsigned selectedCount = 0;
  unsigned long selectedBytes = 0;
  drawRows(scrSeg, 40, BrightWhite, Red, files, focused, &selectedCount, &selectedBytes);

  frameBottom(40, BrightWhite, Red, scrSeg);
  char freeStr[16] = {0};
  strcpy(freeStr, freeKB ? renderHumanReadableSizeKB(freeKB) : "?");
  footer(40, BrightWhite, Red, files, std::set<char>(), 0, freeStr, selectedCount, selectedBytes, scrSeg);

  // loading, errors, ... in the free line of the footer
  if(status && *status)
  {
    char st[19] = {0};
    strncpy(st, status, 18);
    writeString(41, 23, Red, Yellow, st, scrSeg);
  }
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
  static const char* keys[] = {"Tab", "Ins", "F5", "Bksp", "Esc", NULL};
  static const char* texts[] = {" Switch ", " Select ", " Copy ", " Up ", " Quit ", NULL};

  writeString(0, 24, White, Black, "                                                                                ", scrSeg);
  int x = 1;
  for(int i = 0; keys[i]; i++)
  {
    writeString(x, 24, White, Red, keys[i], scrSeg);
    x += strlen(keys[i]);
    writeString(x, 24, White, Black, texts[i], scrSeg);
    x += strlen(texts[i]) + 1;
  }
}

/*
 * A window in the middle of the screen, with a progress bar
 */
void progress_window(void* scrSeg, const char* title, const char* line, unsigned long done, unsigned long total)
{
  static const int x = 14;
  static const int w = 52;
  char row[64];

  // the frame
  memset(row, '\xCD', w);
  row[0] = '\xC9'; row[w - 1] = '\xBB'; row[w] = 0;
  writeString(x, 9, Blue, BrightWhite, row, scrSeg);
  memset(row, ' ', w);
  row[0] = '\xBA'; row[w - 1] = '\xBA';
  for(int y = 10; y < 14; y++)
  {
    writeString(x, y, Blue, BrightWhite, row, scrSeg);
  }
  memset(row, '\xCD', w);
  row[0] = '\xC8'; row[w - 1] = '\xBC';
  writeString(x, 14, Blue, BrightWhite, row, scrSeg);

  // the texts
  char t[64] = {0};
  strncpy(t, title, w - 4);
  writeString(x + 2, 10, Blue, Yellow, t, scrSeg);
  memset(t, 0, sizeof(t));
  strncpy(t, line, w - 4);
  writeString(x + 2, 11, Blue, BrightWhite, t, scrSeg);

  // the bar
  int barLen = w - 4;
  int filled = total ? (int)((double)done * barLen / total) : barLen;
  if(filled > barLen) filled = barLen;
  memset(row, '\xB0', barLen);
  memset(row, '\xDB', filled);
  row[barLen] = 0;
  writeString(x + 2, 12, Blue, LightAqua, row, scrSeg);

  sprintf(t, "%lu / %lu bytes", done, total);
  writeString(x + 2, 13, Blue, White, t, scrSeg);
}

void password_window(void* scrSeg, char *pwd, int error, const char* errorText)
{
    writeString(0,0, Red, White, main_header_connect_screen, scrSeg);
    writeString(64,0, Red, LightYellow, main_header_connect_http_screen, scrSeg);

    writeString(9, 10, Blue, White, header_1_pwd_win , scrSeg);
    writeString(9, 11, Blue, White, header_2_pwd_win , scrSeg);
    writeString(9, 12, Blue, White, header_3_pwd_win , scrSeg);
    writeString(9, 13, Blue, White, contnt_1_pwd_win , scrSeg);
    writeString(9, 14, Blue, White, err_2_pwd__contn , scrSeg);
    writeString(9, 15, Blue, White, contnt_2_pwd_win , scrSeg);
    writeString(9, 16, Blue, White, footer_pwd_windw , scrSeg);

    writeString(21, 13, Black, White, "                                             ", scrSeg);
    writeString(21, 15, Black, White, "                                             ", scrSeg);
}

void connect_window(void* scrSeg, char *ip[], int error, const char* errorText)
{
    writeString(0,0, Red, White, main_header_connect_screen, scrSeg);
    writeString(64,0, Red, LightYellow, main_header_connect_http_screen, scrSeg);

    writeString(18, 10, Blue, White, header_1_connect_win , scrSeg);
    writeString(18, 11, Blue, White, header_2_connect_win , scrSeg);
    writeString(18, 12, Blue, White, header_3_connect_win , scrSeg);
    writeString(18, 13, Blue, White, header_4_connect_win , scrSeg);
    writeString(18, 14, Blue, White, header_5_connect_win , scrSeg);

    if(errorText != NULL && strlen((char*)errorText) == 0)
    {
        writeString(18, 15, Blue, White, header_6_connect_win , scrSeg);
    }
    else
    {
        writeString(18, 15, Blue, White, footer_err_1_connect, scrSeg);
        writeString(18, 16, Blue, White, footer_err_2_connect, scrSeg);
        writeString(18, 17, Blue, White, header_6_connect_win, scrSeg);

        // and the error
        writeString(19, 16, Blue, Red, errorText, scrSeg);
    }

    writeString(33,14, Black, White, "   ", scrSeg);
    writeString(37,14, Black, White, "   ", scrSeg);
    writeString(41,14, Black, White, "   ", scrSeg);
    writeString(45,14, Black, White, "   ", scrSeg);
}
