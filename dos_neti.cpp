#include "dos_neti.h"
#include "guistate.h"
#include "log.h"

#include <timer.h>
#include <trace.h>
#include <utils.h>
#include <packet.h>
#include <arp.h>
#include <tcp.h>
#include <tcpsockm.h>
#include <dns.h>

#include <stdlib.h>
#include <string.h>

DosMTcpIpIface::DosMTcpIpIface() : NetworkInterface(), theSocket(NULL), state(Unknown)
{
    log_debug() << "Created interface:" << (void*)this;
    state = Created;
}

DosMTcpIpIface::~DosMTcpIpIface()
{
    log_debug() << "Deleted interface:" << (void*)this;
}

bool DosMTcpIpIface::setup()
{
    // mTCP environment
    if(state < EnvironmentRead)
    {
        log_debug() << (void*)this << " is parsing environment";

        if(Utils::parseEnv() != 0)
        {
            log_error() << "Cannot set up mTCP environment";
            return false;
        }
        state = EnvironmentRead;
    }


    if(state < StackInited)
    {
        // start the TCP/IP stack
        // 1 - one socket
        log_debug() << "Calling initStatck";
        if(Utils::initStack(1, TCP_SOCKET_RING_SIZE, NULL, NULL))
        {
            log_error() << "Cannot set up mTCP stack";
            return false;
        }
        state = StackInited;
    }

    return true;
}


void DosMTcpIpIface::shutdown()
{
    log_debug() << "Closing socket";
    ((TcpSocket*)theSocket)->close();

    log_debug() << "calling endStack";
    Utils::endStack();

    log_debug() << "done endStack";
}

void* DosMTcpIpIface::provide_socket()
{
    // create the client socket
    TcpSocket* clientSocket = TcpSocketMgr::getSocket();
    log_debug() << "got socket:" << (void*)clientSocket;

    // and set the receive buffer size to be 8192
    int8_t rc = clientSocket->setRecvBuffer(1024);

    theSocket = clientSocket;
    return clientSocket;
}


bool DosMTcpIpIface::connect(void* sock, const char* where, uint16_t port)
{
    TcpSocket *clientSocket = (TcpSocket *)sock;

    if(clientSocket != theSocket)
    {
        log_critical() << "Invalid socket received:" << (void*)sock;
    }

    // connecting to somewhere
    IpAddr_t serverAddr;
    int8_t rc2 = Dns::resolve(where, serverAddr, 1);

    if(rc2 < 0)
    {
        GuiStatemachine::reportError("Connection failed, cannot resolve host?");
        return false;
    }

    log_debug() << "DNS resolved";

    // which port we will receive stuff
    uint16_t clientPort = 1099 + rand() % 1024;

    int8_t rc = clientSocket->connect(clientPort, serverAddr, port, 1000);

    if(rc != 0)
    {
        log_error() << "Cannot connect to "  << serverAddr << "(" << (int)serverAddr[0] << "."
                    << (int)serverAddr[1] << "."
                    << (int)serverAddr[2] << "."
                    << (int)serverAddr[3] << "."
                    << ":" << port  << "/" << where << ", rc =" << rc;
        GuiStatemachine::reportError("Connection failed, is cloudy peer running?");
        return false;
    }

    log_debug() << "Connected with rc:" << rc;

    return true;

}

void DosMTcpIpIface::poll(void* sock, uint32_t timeout, void *data, void (*callback)(void *, const char *) )
{
    TcpSocket *s = (TcpSocket*)sock;

    if(s->isRemoteClosed())
    {
        log_debug() << "Socket closed on the other side, giving up";
        return;
    }

#define INBUFSIZE  8192
    uint8_t* inBuf = (uint8_t*)calloc(INBUFSIZE, 1);
    memset(inBuf, INBUFSIZE, 0);

    int inBufIndex = 0;

    clockTicks_t startTime = TIMER_GET_CURRENT( );

    while ( 1 )
    {

        log_debug() << "Receiving ... ";

        PACKET_PROCESS_SINGLE;
        Arp::driveArp( );
        Tcp::drivePackets( );

        int rc = s->recv( inBuf + inBufIndex, (INBUFSIZE - inBufIndex) );
        log_debug() << "Received, with rc:" << rc;

        if ( rc > 0 )
        {
            log_debug() << "Calling back... ";

            callback(data, (const char*)inBuf);
            memset(inBuf, INBUFSIZE, 0);
            inBufIndex = 0;
        }
        else
        if(rc < 0)
        {
            break;
        }

        log_debug() << "After receiving ... ";

        uint32_t t_ms = Timer_diff( startTime, TIMER_GET_CURRENT( ) ) * TIMER_TICK_LEN;

        // Timeout?
        if ( t_ms > timeout )
        {
            log_debug() << "Breaking ... ";

            break;
        }

    }
    free (inBuf);
}

void DosMTcpIpIface::send(void* sock, const char* data, unsigned short length)
{
    log_debug() << "States before sending";
    GuiStatemachine::instance().logStates();

    TcpSocket *s = (TcpSocket*)sock;
    int rc = s->send((unsigned char*)data, length);
    log_debug() << "Sent, with rc:" << rc;
    log_debug() << "States before drivePackets";
    GuiStatemachine::instance().logStates();
    Tcp::drivePackets( );
    log_debug() << "States after drivePackets";

    GuiStatemachine::instance().logStates();
}

