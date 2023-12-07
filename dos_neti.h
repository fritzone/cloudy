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

    virtual bool setup();
    virtual void shutdown();
    virtual void* provide_socket();
    virtual bool connect(void* sock, const char* where, uint16_t port);
    virtual void poll(void *sock, uint32_t timeout, void* data, void(*callback)(void*, const char*) );
    virtual void send(void* sock, const char* data, unsigned short length);

private:
    TcpSocket* theSocket;
    InterfaceState state;
};

#endif

