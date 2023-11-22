#ifndef _DOS_MTCP_NETIFACE_H_
#define _DOS_MTCP_NETIFACE_H_

#include "types.h"
#include "net_ifce.h"

class DosMTcpIpIface : public NetworkInterface
{
public:
  virtual bool setup();
  virtual void shutdown();
  virtual void* provide_socket();
  virtual bool connect(void* sock, const char* where);
  virtual void poll(void *sock, uint32_t timeout, void(*callback)(const char*) );
  virtual void send(void* sock, const char* data, unsigned short length);

private:
  void* theSocket;
};

#endif

