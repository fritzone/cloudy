#include <dos.h>
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

#include "log.h"
#include "ezxml.h"
#include "dos_scrn.h"
#include "dos_cgui.h"
#include "list.h"
#include "filelist.h"
#include "memhandl.h"
#include "messager.h"
#include <shar_ptr.h>

#include "guistmch.h"

// the various states of the gui
#include "guistate.h"
#include "inputip.h"
#include "dos_brws.h"
#include "net_stts.h"
#include "dos_pwds.h"
#include "prot.h"
#include "msg_prot/protocol.h"

#include <set>

/* Globals */

// This is the global screen, used to hold the backbuffer
static void* screen = 0;

ProtocolImpl p;

/* Will parse the arguments as came in from the command line, nothing for now */
static void parseArgs(int argc, char* argv[])
{
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

int messager(int m, void* data)
{
    switch(m)
    {

    case MSG_IP_ENTERED:
    {
        NetStatemachine::instance().advance(data);

        return 0;
    }
    case MSG_CONNECTED:
    {
        NetStatemachine::instance().advance(data);
        return 0;
    }

    case MSG_CONNECTION_FAILED:
    {
        NetStatemachine::instance().go_back(data);
        return 0;
    }

    case MSG_CONNECTION_ACKNOWLEDGED:
    {
        ConnectionRequestReply* crrp =  (ConnectionRequestReply*)(data);
        if(crrp->get_authentication_required())
        {
            GuiStatemachine::instance().advance(NULL, State_PasswordRequested);
        }
        else
        {
            GuiStatemachine::instance().advance(NULL, State_GoBrowsing);
        }
        CursorRaii::hideCursor();
        return 0;
    }

    }

    return 1;
}

/**
 * @brief onConnectRequestReply is called when a a connection request reply was received
 * @param crrp the reply objects
 */
static void __far onConnectRequestReply(const ConnectionRequestReply* crrp)
{
    log_info() << "authentication required:" << crrp->get_authentication_required();
    messager(MSG_CONNECTION_ACKNOWLEDGED, (void*)crrp);
}

void interrupt newInt6Handler() {
    // Your custom interrupt handling code
    printf("int6 handler!\n");

    unsigned int _cs, _ip;
    _asm {
        mov ax,word ptr [bp + 4]
        mov _cs, ax
        mov ax, word ptr [bp + 6]
        mov _ip, ax
    }

    printf("CS:IP pair at the time of interrupt: %x:%x\n", _cs , _ip);

    printf("currentGuiState: %p\n", (void*)GuiStatemachine::instance().getCurrentState());
    printf("currentNetState: %p\n", (void*)NetStatemachine::instance().getCurrentState());
    exit(1);
}

/*
 * Main entrypoint
 */
int main(int argc, char* argv[])
{
    p.set_ConnectionRequestReply_Handler(onConnectRequestReply);

    size_t avl_beg = _memavl(), max_meg = _memmax();

    log_info() << "===============================[Starting]=======================";

     _dos_setvect(6, newInt6Handler);
    // read command line, act accordingly
    parseArgs(argc, argv);

    screen = calloc(4000, 2); // the size of the screen: 80 x 25, 1 uint16 for each position
    if(screen == NULL)
    {
        exit(1);
    }

    // manage the cursor
    CursorRaii cursor;

    // Gui statemachine
    GuiState* brFoldState = new BrowseFoldersState();
    GuiState* ipInputState = new GuiState_InputIp();
    GuiState* getPasswordState = new GuiState_PasswordScreen();

    GuiStatemachine::instance().addState(ipInputState);
    GuiStatemachine::instance().addState(brFoldState);
    GuiStatemachine::instance().addState(getPasswordState);

    GuiStatemachine::instance().init(ipInputState);
    GuiStatemachine::instance().getCurrentState()->setCursor(&cursor);

    ipInputState->setNextState(State_GoBrowsing, brFoldState);
    ipInputState->setNextState(State_PasswordRequested, getPasswordState);

    // Network statemachine
    NetState* netStateNoOp = new NetState_NoOp;
    NetState* netStateTryConnect = new NetState_TryConnect;
    NetState* netStateTryPoll = new NetState_TryPoll;
    NetState* netStateConnectRequest = new NetState_ConnectRequest;
    netStateNoOp->setNext(netStateTryConnect);
    netStateTryConnect->setNext(netStateConnectRequest);
    netStateTryConnect->setPrev(netStateNoOp);

    NetStatemachine::instance().addState(netStateNoOp);
    NetStatemachine::instance().addState(netStateTryConnect);
    NetStatemachine::instance().addState(netStateTryPoll);
    NetStatemachine::instance().addState(netStateConnectRequest);
    NetStatemachine::instance().setCurrentState(netStateNoOp);

    // current working directory
    char startupDir[PATH_MAX + 1] = {0};
    getcwd(startupDir, PATH_MAX + 1);

    _setvideomode( _TEXTC80 );

    // show the gui, the code below goes to the gui part after connect

    while(1)
    {
        //log_debug() << "Loop start:";
        //log_debug() << "Net:" << NetStatemachine::instance().currentState->name();
        //log_debug() << "Gui:" << GuiStatemachine::instance().getCurrentState()->name();

        GuiStatemachine::instance().getCurrentState()->onRefreshContent();

        clearscr(screen);
        GuiStatemachine::instance().getCurrentState()->paint(screen);
        flip(screen);

        if(kbhit())
        {
            log_debug() << "Keypress";

            int c = getch();

            if(c == 27) // Escape
            {
                break;
            }

            if(c== 9) // Tab
            {
                GuiStatemachine::instance().getCurrentState()->onTab();
            }

            if(c== 8) // Backspace
            {
                GuiStatemachine::instance().getCurrentState()->onBackspace();
            }

            if(c == 13) // Enter?
            {
                GuiStatemachine::instance().getCurrentState()->onEnter();
            }

            if(isprint(c))
            {
                GuiStatemachine::instance().getCurrentState()->onChar(c);
            }

            // special key?
            if(c == 0)
            {
                c = getch();

                //fprintf(stdout, "%d\n", c);
                //break;

                if(c == 82) // Insert - Select/Deselect, usually used only in the browse panel
                {
                    GuiStatemachine::instance().getCurrentState()->onInsert();
                    c = 80; // and move to the next element
                }

                if(c == 80) // Down arrow
                {
                    GuiStatemachine::instance().getCurrentState()->onDownArrow();
                }

                if(c == 72) // Up arrow
                {
                    GuiStatemachine::instance().getCurrentState()->onUpArrow();
                }

                if(c==72) // right
                {
                    GuiStatemachine::instance().getCurrentState()->onRightArrow();
                }
            }
        }

        if(NetStatemachine::instance().currentState != NULL)
        {
            NetStatemachine::instance().currentState->execute(NetStatemachine::instance().currentState->stateData);
        }
        else
        {
            log_info() << "Null net state";
        }


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
    fprintf(stderr, "Available at begin:%zu/%zu end:%zu/%zu\n", avl_beg, max_meg, _memavl(), _memmax());

}

