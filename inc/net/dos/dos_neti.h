
#ifndef _DOS_MTCP_NETIFACE_H_
#define _DOS_MTCP_NETIFACE_H_

#include "types.h"
#include "net_ifce.h"

class TcpSocket;

class DosMTcpIpIface : public NetworkInterface
{

    enum InterfaceState
    {
        Created = 0,
        EnvironmentRead = 1,
        StackInited = 2,

        Unknown = 255
    };

public:

    DosMTcpIpIface();
    virtual ~DosMTcpIpIface();

    /**
     * Checks that mTCP can work: MTCPCFG is set (if not, the MTCP.CFG next to
     * the program is used), the configuration is complete and the packet driver
     * is loaded. Prints what is wrong to the console and returns false if
     * something is. Call it before switching to the GUI.
     */
    static bool checkEnvironment(const char* argv0);

    virtual bool setup();
    virtual void shutdown();
    virtual void* provide_socket();
    virtual bool connect(void* sock, const char* where, uint16_t port);
    virtual bool isConnected(void* sock);
    virtual void poll(void *sock, uint32_t timeout, void* data, FrameCallback callback);
    virtual bool send(void* sock, const char* data, uint16_t length);

private:

    // calls the callback for the complete frames in the receive buffer
    void dispatchFrames(void* data, FrameCallback callback);

private:
    TcpSocket* theSocket;
    InterfaceState state;

    // the received bytes which are not yet dispatched as frames
    char* recvBuf;
    uint16_t recvLen;
    // up to here the receive buffer was already searched for a '\0'
    uint16_t scannedLen;
    bool inPoll;
};

#endif
