#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <conio.h>
#include <graph.h>
#include <direct.h>

#include <types.h>

#include "ezxml.h"
#include "dos_scrn.h"
#include "dos_cgui.h"
#include "list.h"
#include "filelist.h"
#include "memhandl.h"
#include "protocol.h"

// the various states of the gui
#include "guistate.h"
#include "inputip.h"
#include "brwsfldr.h"

#include <set>

/* Globals */

// This is the global screen, used to hold the backbuffer
static void* screen = 0;

protocol p;

/* Will parse the arguments as came in from the command line, nothing for now */
static void parseArgs(int argc, char* argv[])
{
}

void onDataReceived(const char* data)
{
    fprintf(stderr, "%s\n", data);
    ezxml_t doc = ezxml_parse_str((char*)data, strlen(data));
    char* s;
    s = ezxml_toxml(doc);
    fprintf(stderr, "%s\n", s);
    free(s);

    ezxml_t cld = ezxml_child(doc, "cld");
    if(cld == NULL)
    {
        fprintf(stderr, "cannot get cloud tag");
        return;
    }
    const char* cloud_version = ezxml_attr(cld, "v");
    fprintf(stderr, "cloud version: %s\n", cloud_version);

    ezxml_t msg = ezxml_child(cld, "msg");
    const char* msg_t = ezxml_attr(msg, "t");
    fprintf(stderr, "message_t: %s\n", msg_t);

    char* msg_data = ezxml_toxml(msg);

    p.message(msg_t, msg_data);

    free(msg_data);

    ezxml_free(doc);

}

/*
static void testRun()
{
    // the network interface
    DosMTcpIpIface netIface;
    if(!netIface.setup())
    {
        fprintf(stderr, "Network setup failed");
        free(screen);
        exit(-1);
    }

    void* clientSocket = netIface.provide_socket();
    if(!netIface.connect(clientSocket, "10.0.2.2"))
    {
        fprintf(stderr, "Cannot connect");
        netIface.shutdown();
        free(screen);
        exit(-1);
    }

    p.setNetworkInterface(&netIface);

    while(1)
    {
        netIface.poll(clientSocket, 300, onDataReceived);
        fprintf(stderr, " ");
        if(kbhit())
        {
            break;
        }

    }

    netIface.shutdown();
    exit(0);

}
*/



int asciitable()
{
    _setvideomode( _TEXTC80 );

    int cc = 0;
    for (int i=0; i< 16; i++)
    {
        for (int j=0; j< 16; j++)
        {
            writeChar(i*5+4, j, Black, White, cc, screen);
            char s[5];
            sprintf(s, "%4i", (int)cc);
            writeString(i*5, j, Blue, LightYellow, s, screen);
            cc ++;
        }
    }
    flip(screen);

    getch();
    exit(1);
}

/*
 * Main entrypoint
 */
int main(int argc, char* argv[])
{
    // read command line, act accordingly
    parseArgs(argc, argv);

    screen = calloc(2000, 2); // the size of the screen: 80 x 25, 1 uint16 for each position
    if(screen == NULL)
    {
        exit(1);
    }

    // manage the cursor
    CursorRaii cursor;


    State* brFoldState = new BrowseFoldersState(NULL, NULL);
    State* ipInputState = new InputIpState(brFoldState, NULL);
    brFoldState->setrPreviousState(ipInputState);

    GuiState::instance().init(ipInputState);
    GuiState::instance().getCurrentState()->setCursor(&cursor);

    // current working directory
    char startupDir[PATH_MAX + 1] = {0};
    getcwd(startupDir, PATH_MAX + 1);

    _setvideomode( _TEXTC80 );

    // show the gui, the code below goes to the gui part after connect

    while(1)
    {
        GuiState::instance().getCurrentState()->onRefreshContent();
        clearscr(screen);
        GuiState::instance().getCurrentState()->paint(screen);
        flip(screen);

        if(kbhit())
        {
            int c = getch();

            if(c == 27) // Escape
            {
                break;
            }

            if(c== 9) // Tab
            {
                GuiState::instance().getCurrentState()->onTab();
            }

            if(c== 8) // Backspace
            {
                GuiState::instance().getCurrentState()->onBackspace();
            }

            if(c == 13) // Enter?
            {
                GuiState::instance().getCurrentState()->onEnter();
            }

            if(isprint(c))
            {
                GuiState::instance().getCurrentState()->onChar(c);
            }

            // special key?
            if(c == 0)
            {
                c = getch();

                //fprintf(stdout, "%d\n", c);
                //break;

                if(c == 82) // Insert - Select/Deselect, usually used only in the browse panel
                {
                    GuiState::instance().getCurrentState()->onInsert();
                    c = 80; // and move to the next element
                }

                if(c == 80) // Down arrow
                {
                    GuiState::instance().getCurrentState()->onDownArrow();
                }

                if(c == 72) // Up arrow
                {
                    GuiState::instance().getCurrentState()->onUpArrow();
                }

                if(c==72) // right
                {
                    GuiState::instance().getCurrentState()->onRightArrow();
                }
            }
        }

//        netIface.poll(clientSocket, 300, onDataReceived);
    }

    delete brFoldState;
    delete ipInputState;

  //  netIface.shutdown();

    free(screen);

    __asm {
        // set 80x25
        xor ah, ah
        mov al, 3
        int 0x10
    }

    _chdir(startupDir);

}
