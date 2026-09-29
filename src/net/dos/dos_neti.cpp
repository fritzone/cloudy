
#include "dos_neti.h"
#include "guistmch.h"
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
#include <stdio.h>
#include <io.h>
#include <dos.h>

// The biggest frame we can receive, the peer keeps its messages below this
#define RECV_BUF_SIZE 16384u

// The size of the mTCP receive buffer of the socket (max 16K)
#define SOCKET_RECV_BUF_SIZE 8192u

// How long we keep trying to get the data out before giving up
#define SEND_TIMEOUT_MS 15000ul

// Lets the TCP/IP stack do its job
static void driveStack()
{
    PACKET_PROCESS_SINGLE;
    Arp::driveArp( );
    Tcp::drivePackets( );
}

DosMTcpIpIface::DosMTcpIpIface() : NetworkInterface(), theSocket(NULL), state(Unknown),
    recvBuf(NULL), recvLen(0), scannedLen(0), inPoll(false)
{
    log_debug() << "Created interface:" << (void*)this;
    recvBuf = (char*)malloc(RECV_BUF_SIZE);
    state = Created;
}

DosMTcpIpIface::~DosMTcpIpIface()
{
    log_debug() << "Deleted interface:" << (void*)this;
    free(recvBuf);
}

// The PACKETINT of the mTCP configuration file, 0 if there is none
static unsigned configuredPacketInt(const char* cfgFile)
{
    FILE* f = fopen(cfgFile, "r");
    char line[128];
    unsigned packetInt = 0;

    if(f == NULL)
    {
        return 0;
    }
    while(fgets(line, sizeof(line), f))
    {
        char name[16];
        unsigned value;
        if(sscanf(line, "%15s %x", name, &value) == 2 && !stricmp(name, "PACKETINT"))
        {
            packetInt = value;
        }
    }
    fclose(f);
    return packetInt;
}

bool DosMTcpIpIface::checkEnvironment(const char* argv0)
{
    // stays in the environment, putenv keeps the pointer
    static char envVar[96];

    if(getenv("MTCPCFG") == NULL)
    {
        const char* slash = strrchr(argv0, '\\');
        if(slash != NULL && slash - argv0 < 70)
        {
            sprintf(envVar, "MTCPCFG=%.*sMTCP.CFG", (int)(slash - argv0 + 1), argv0);
            if(access(envVar + 8, 0) == 0)
            {
                putenv(envVar);
            }
        }
    }

    const char* cfg = getenv("MTCPCFG");
    if(cfg == NULL)
    {
        printf("cloudy: MTCPCFG is not set, so mTCP does not know the network settings.\n"
               "Run NETSTART.BAT from the cloudy directory first.\n");
        return false;
    }

    // mTCP prints what is wrong
    if(Utils::parseEnv() != 0)
    {
        printf("\ncloudy: the network settings in %s are not usable (see above).\n"
               "If IPADDR is missing, DHCP could not get an address: run NETSTART.BAT\n"
               "from the cloudy directory and read its messages.\n", cfg);
        return false;
    }

    unsigned packetInt = configuredPacketInt(cfg);
    char __far* handler = packetInt ? (char __far*)_dos_getvect(packetInt) : NULL;
    if(handler == NULL || _fmemcmp(handler + 3, "PKT DRVR", 8) != 0)
    {
        printf("cloudy: there is no packet driver at interrupt 0x%X.\n"
               "Run NETSTART.BAT from the cloudy directory, it loads the driver of the\n"
               "network card, and read its messages.\n", packetInt);
        return false;
    }

    return true;
}

bool DosMTcpIpIface::setup()
{
    if(recvBuf == NULL)
    {
        log_error() << "Cannot allocate the receive buffer";
        return false;
    }

    // mTCP environment
    if(state < EnvironmentRead)
    {
        log_debug() << (void*)this << " is parsing environment";

        if(Utils::parseEnv() != 0)
        {
            log_error() << "The mTCP configuration" << (getenv("MTCPCFG") ? getenv("MTCPCFG") : "(MTCPCFG not set)") << "is not usable";
            return false;
        }
        state = EnvironmentRead;
    }


    if(state < StackInited)
    {
        // start the TCP/IP stack with one socket and all the transmit buffers
        log_debug() << "Calling initStack";
        if(Utils::initStack(1, TCP_MAX_XMIT_BUFS, NULL, NULL))
        {
            log_error() << "Cannot set up mTCP stack, is the packet driver loaded?";
            return false;
        }
        state = StackInited;
    }

    return true;
}


void DosMTcpIpIface::shutdown()
{
    if(state < StackInited)
    {
        return;
    }

    if(theSocket)
    {
        log_debug() << "Closing socket";
        theSocket->close();
        TcpSocketMgr::freeSocket(theSocket);
        theSocket = NULL;
    }

    log_debug() << "calling endStack";
    Utils::endStack();
    state = EnvironmentRead;

    log_debug() << "done endStack";
}

void* DosMTcpIpIface::provide_socket()
{
    if(theSocket)
    {
        return theSocket;
    }

    TcpSocket* clientSocket = TcpSocketMgr::getSocket();
    log_debug() << "got socket:" << (void*)clientSocket;
    if(clientSocket == NULL)
    {
        return NULL;
    }

    if(clientSocket->setRecvBuffer(SOCKET_RECV_BUF_SIZE) != TCP_RC_GOOD)
    {
        log_error() << "Cannot set the receive buffer";
        TcpSocketMgr::freeSocket(clientSocket);
        return NULL;
    }

    theSocket = clientSocket;
    return clientSocket;
}


bool DosMTcpIpIface::connect(void* sock, const char* where, uint16_t port)
{
    TcpSocket *clientSocket = (TcpSocket *)sock;

    if(clientSocket != theSocket)
    {
        log_critical() << "Invalid socket received:" << (void*)sock;
        return false;
    }

    // connecting to somewhere
    IpAddr_t serverAddr;
    int8_t rc2 = Dns::resolve(where, serverAddr, 1);

    if(rc2 < 0)
    {
        GuiStatemachine::instance().reportError("Connection failed, cannot resolve host?");
        return false;
    }

    log_debug() << "DNS resolved";

    // which port we will receive stuff
    uint16_t clientPort = 1099 + rand() % 1024;

    int8_t rc = clientSocket->connect(clientPort, serverAddr, port, 5000);

    if(rc != 0)
    {
        log_error() << "Cannot connect to " << (int)serverAddr[0] << "."
                    << (int)serverAddr[1] << "."
                    << (int)serverAddr[2] << "."
                    << (int)serverAddr[3]
                    << ":" << port  << "/" << where << ", rc =" << rc;
        GuiStatemachine::instance().reportError("Connection failed, is cloudy peer running?");

        // start with a fresh socket next time
        theSocket->close();
        TcpSocketMgr::freeSocket(theSocket);
        theSocket = NULL;
        return false;
    }

    log_debug() << "Connected with rc:" << rc;
    recvLen = 0;
    scannedLen = 0;

    return true;
}

bool DosMTcpIpIface::isConnected(void* sock)
{
    TcpSocket *s = (TcpSocket*)sock;
    return s != NULL && !s->isRemoteClosed() && s->isConnectComplete();
}

void DosMTcpIpIface::dispatchFrames(void* data, FrameCallback callback)
{
    uint16_t frameStart = 0;

    while(scannedLen < recvLen)
    {
        char* end = (char*)memchr(recvBuf + scannedLen, 0, recvLen - scannedLen);
        if(end == NULL)
        {
            scannedLen = recvLen;
            break;
        }

        uint16_t frameEnd = (uint16_t)(end - recvBuf);
        if(frameEnd > frameStart && callback)
        {
            callback(data, recvBuf + frameStart);
        }

        frameStart = frameEnd + 1;
        scannedLen = frameStart;
    }

    // move the incomplete frame to the beginning of the buffer
    if(frameStart > 0)
    {
        memmove(recvBuf, recvBuf + frameStart, recvLen - frameStart);
        recvLen -= frameStart;
        scannedLen -= frameStart;
    }
}

void DosMTcpIpIface::poll(void* sock, uint32_t timeout, void *data, FrameCallback callback)
{
    TcpSocket *s = (TcpSocket*)sock;

    if(inPoll)
    {
        log_error() << "poll called from a frame callback";
        return;
    }
    inPoll = true;

    clockTicks_t startTime = TIMER_GET_CURRENT( );

    while ( 1 )
    {
        driveStack();

        if(recvLen < RECV_BUF_SIZE)
        {
            int16_t rc = s->recv( (uint8_t*)recvBuf + recvLen, RECV_BUF_SIZE - recvLen );
            if(rc > 0)
            {
                recvLen += rc;
                dispatchFrames(data, callback);
            }
        }

        if(recvLen == RECV_BUF_SIZE)
        {
            log_error() << "Frame bigger than" << RECV_BUF_SIZE << "bytes, dropping it";
            recvLen = 0;
            scannedLen = 0;
        }

        uint32_t t_ms = Timer_diff( startTime, TIMER_GET_CURRENT( ) ) * TIMER_TICK_LEN;
        if ( t_ms >= timeout )
        {
            break;
        }
    }

    inPoll = false;
}

bool DosMTcpIpIface::send(void* sock, const char* data, uint16_t length)
{
    TcpSocket *s = (TcpSocket*)sock;
    uint16_t sent = 0;
    clockTicks_t startTime = TIMER_GET_CURRENT( );

    while(sent < length)
    {
        int16_t rc = s->send((uint8_t*)data + sent, length - sent);
        if(rc < 0)
        {
            log_error() << "Send failed with rc:" << rc;
            return false;
        }
        sent += rc;

        driveStack();

        if(rc == 0 && Timer_diff( startTime, TIMER_GET_CURRENT( ) ) * TIMER_TICK_LEN > SEND_TIMEOUT_MS)
        {
            log_error() << "Send timed out, sent" << sent << "of" << length;
            return false;
        }
    }

    return true;
}
