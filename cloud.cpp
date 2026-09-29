#include <dos.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <conio.h>
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
#include "dos_neti.h"
#include "msg_prot/protocol.h"

#include <set>

/* Globals */

// This is the global screen, used to hold the backbuffer
static void* screen = 0;

ProtocolImpl p;

// The states the message handlers need to reach
static GuiState* ipInputState = NULL;
static BrowseFoldersState* browseState = NULL;
static NetState_TryConnect* netStateTryConnect = NULL;
static NetState* netStateNoOp = NULL;

// mTCP hooks interrupts, so it must be shut down whatever way we exit
static void shutdownNetwork()
{
    if(netStateTryConnect)
    {
        netStateTryConnect->disconnect();
    }
}

// The IP given in the command line, we connect to it right away
static char autoConnectIp[64] = {0};

/* Will parse the arguments as came in from the command line:
 *   -v        log informational messages too
 *   -d        log debug messages too
 *   <ip>      connect to this peer right away
 */
static void parseArgs(int argc, char* argv[])
{
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "/v"))
        {
            g_logLevel = LOG_INFORMATION;
        }
        else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "/d"))
        {
            g_logLevel = LOG_DEBUG;
        }
        else
        {
            strncpy(autoConnectIp, argv[i], sizeof(autoConnectIp) - 1);
        }
    }
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
    setTextMode();

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
        if(!crrp->get_accepted())
        {
            GuiStatemachine::instance().reportError("The peer refused the connection");
            netStateTryConnect->disconnect();
            NetStatemachine::instance().setCurrentState(netStateNoOp);
            return 0;
        }

        if(crrp->get_authentication_required())
        {
            GuiStatemachine::instance().advance(NULL, State_PasswordRequested);
        }
        else
        {
            GuiStatemachine::instance().advance(NULL, State_GoBrowsing);
            browseState->onConnected(crrp->get_host_name());
        }
        CursorRaii::hideCursor();
        return 0;
    }

    case MSG_DISCONNECTED:
    {
        netStateTryConnect->disconnect();
        NetStatemachine::instance().setCurrentState(netStateNoOp);
        browseState->onDisconnected();
        GuiStatemachine::instance().reportError("The connection to the peer was lost");
        GuiStatemachine::instance().init(ipInputState);
        requestRepaint();
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

// The rest of the messages go to the browser
static void __far onStatus(const Status* st)
{
    browseState->onStatus(st);
}

static void __far onDirectoryList(const DirectoryList* dl)
{
    browseState->onDirectoryList(dl);
}

static void __far onFileData(const FileData* fd)
{
    browseState->onFileData(fd);
}

static void __far onFileWriteReply(const FileWriteReply* r)
{
    browseState->onFileWriteReply(r);
}

static void __far onMakeDirectoryReply(const MakeDirectoryReply* r)
{
    browseState->onMakeDirectoryReply(r);
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
    p.set_Status_Handler(onStatus);
    p.set_DirectoryList_Handler(onDirectoryList);
    p.set_FileData_Handler(onFileData);
    p.set_FileWriteReply_Handler(onFileWriteReply);
    p.set_MakeDirectoryReply_Handler(onMakeDirectoryReply);

    size_t avl_beg = _memavl(), max_meg = _memmax();
    atexit(shutdownNetwork);

    log_info() << "===============================[Starting]=======================";

     _dos_setvect(6, newInt6Handler);
    // read command line, act accordingly
    parseArgs(argc, argv);

    // no point in starting without a network, and here the messages can be read
    if(!DosMTcpIpIface::checkEnvironment(argv[0]))
    {
        return 1;
    }

    screen = calloc(4000, 2); // the size of the screen: 80 x 25, 1 uint16 for each position
    if(screen == NULL)
    {
        exit(1);
    }

    // manage the cursor
    CursorRaii cursor;

    // Gui statemachine
    browseState = new BrowseFoldersState();
    GuiState* brFoldState = browseState;
    ipInputState = new GuiState_InputIp();
    GuiState* getPasswordState = new GuiState_PasswordScreen();

    GuiStatemachine::instance().addState(ipInputState);
    GuiStatemachine::instance().addState(brFoldState);
    GuiStatemachine::instance().addState(getPasswordState);

    GuiStatemachine::instance().init(ipInputState);
    GuiStatemachine::instance().getCurrentState()->setCursor(&cursor);

    ipInputState->setNextState(State_GoBrowsing, brFoldState);
    ipInputState->setNextState(State_PasswordRequested, getPasswordState);

    // Network statemachine
    netStateNoOp = new NetState_NoOp;
    netStateTryConnect = new NetState_TryConnect;
    NetState* netStateConnected = new NetState_Connected;
    netStateNoOp->setNext(netStateTryConnect);
    netStateTryConnect->setNext(netStateConnected);
    netStateTryConnect->setPrev(netStateNoOp);
    netStateConnected->setPrev(netStateNoOp);

    NetStatemachine::instance().addState(netStateNoOp);
    NetStatemachine::instance().addState(netStateTryConnect);
    NetStatemachine::instance().addState(netStateConnected);
    NetStatemachine::instance().setCurrentState(netStateNoOp);

    // current working directory
    char startupDir[PATH_MAX + 1] = {0};
    getcwd(startupDir, PATH_MAX + 1);

    setTextMode();

    // show the gui, the code below goes to the gui part after connect

    if(autoConnectIp[0])
    {
        messager(MSG_IP_ENTERED, autoConnectIp);
    }

    while(1)
    {
        GuiState* state = GuiStatemachine::instance().getCurrentState();
        state->onRefreshContent();

        if(takeRepaintRequest())
        {
            clearscr(screen);
            GuiStatemachine::instance().getCurrentState()->paint(screen);
            flip(screen);
        }

        if(kbhit())
        {
            requestRepaint();

            int c = getch();

            if(c == 27) // Escape
            {
                if(!state->onEscape())
                {
                    break;
                }
            }
            else if(c == 9) // Tab
            {
                state->onTab();
            }
            else if(c == 8) // Backspace
            {
                state->onBackspace();
            }
            else if(c == 13) // Enter
            {
                state->onEnter();
            }
            else if(c == 0 || c == 0xE0) // special key, the scan code follows
            {
                c = getch();

                if(c == Key_Insert) // Select/Deselect, usually used only in the browse panel
                {
                    state->onInsert();
                    state->onDownArrow(); // and move to the next element
                }
                else if(c == Key_Down)
                {
                    state->onDownArrow();
                }
                else if(c == Key_Up)
                {
                    state->onUpArrow();
                }
                else if(c == Key_Right)
                {
                    state->onRightArrow();
                }
                else
                {
                    state->onSpecialKey(c);
                }
            }
            else if(isprint(c))
            {
                state->onChar(c);
            }
        }

        if(NetStatemachine::instance().currentState != NULL)
        {
            NetStatemachine::instance().currentState->execute(NetStatemachine::instance().currentState->stateData);
        }
    }

    // leave the network in a clean state, mTCP hooks interrupts
    netStateTryConnect->disconnect();

    delete brFoldState;
    delete ipInputState;
    delete getPasswordState;
    delete netStateTryConnect;
    netStateTryConnect = NULL;
    delete netStateConnected;
    delete netStateNoOp;

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

