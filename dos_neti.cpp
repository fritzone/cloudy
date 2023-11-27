#include "dos_neti.h"
#include "guistate.h"

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

static const char CANNOT_SETUP_MTCP_1[] = "Cannot set up mTCP environment\n";
static const char CANNOT_SETUP_MTCP_2[] = "Cannot set up mTCP stack\n";


bool DosMTcpIpIface::setup()
{
    // mTCP environment
    if(Utils::parseEnv() != 0)
    {
        fprintf(stderr, CANNOT_SETUP_MTCP_1);
        return false;
    }
    // start the TCP/IP stack
    // 1 - one socket
    if(Utils::initStack(1, TCP_SOCKET_RING_SIZE, NULL, NULL))
    {
        fprintf(stderr, CANNOT_SETUP_MTCP_2);
        return false;
    }

    return true;
}


void DosMTcpIpIface::shutdown()
{
    fprintf(stderr, "Closing socket\n");
    ((TcpSocket*)theSocket)->close();
    // done
    fprintf(stderr, "calling endStack\n");
    Utils::endStack();

    fprintf(stderr, "done endStack\n");
}

void* DosMTcpIpIface::provide_socket()
{
    // create the client socket
    TcpSocket* clientSocket = TcpSocketMgr::getSocket();

    // and set the receive buffer size to be 8192
    int8_t rc = clientSocket->setRecvBuffer(1024);

    theSocket = clientSocket;
    return clientSocket;
}


bool DosMTcpIpIface::connect(void* sock, const char* where)
{
    TcpSocket *clientSocket = (TcpSocket *)sock;

    // connecting to somewhere
    IpAddr_t serverAddr;
    int8_t rc2 = Dns::resolve(where, serverAddr, 1);

    if(rc2 < 0)
    {
        fprintf(stderr, "Cannot resolve host\n");
        return false;
    }

    // which port we will receive stuff
    uint16_t clientPort = 1099 + rand() % 1024;

    int8_t rc = clientSocket->connect(clientPort, serverAddr, 8966, 1000);

    if(rc != 0)
    {
        GuiStatemachine::reportError("Connection failed, cloudy peer is running?");
        return false;
    }

    return true;

}

void DosMTcpIpIface::poll( void* sock, uint32_t timeout, void(*callback)(const char*) )
{
    TcpSocket *s = (TcpSocket*)sock;

    if(s->isRemoteClosed())
    {
        return;
    }

#define INBUFSIZE  8192
    uint8_t* inBuf = (uint8_t*)calloc(INBUFSIZE, 1);
    memset(inBuf, INBUFSIZE, 0);

    int inBufIndex = 0;

    clockTicks_t startTime = TIMER_GET_CURRENT( );

    while ( 1 )
    {

        PACKET_PROCESS_SINGLE;
        Arp::driveArp( );
        Tcp::drivePackets( );

        int rc = s->recv( inBuf + inBufIndex, (INBUFSIZE - inBufIndex) );
        if ( rc > 0 )
        {
            callback((const char*)inBuf);
            memset(inBuf, INBUFSIZE, 0);
            inBufIndex = 0;
        }
        else
            if(rc < 0)
            {
                break;
            }

        uint32_t t_ms = Timer_diff( startTime, TIMER_GET_CURRENT( ) ) * TIMER_TICK_LEN;

        // Timeout?
        if ( t_ms > timeout ) break;

    }
    free (inBuf);
}

void DosMTcpIpIface::send(void* sock, const char* data, unsigned short length)
{
    TcpSocket *s = (TcpSocket*)sock;
    s->send((unsigned char*)data, length);
}

