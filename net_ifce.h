#ifndef _NETINTERFACE_H_
#define _NETINTERFACE_H_

#include <types.h>

/*
 * This class will handle the network setup, each system will have to derive from it
 * in order to properly initialize its network capabilities
 */
class NetworkInterface
{
public:

  /**
   * Sets up the network, returns true if success, false otherwise
   */
  virtual bool setup() = 0;

  /**
   * Shuts down the network interface
   */
  virtual void shutdown() = 0;

  /**
   * Will create a socket, and provide an opaque pointer to it.
   * Deriving interfaces are required to manage the sockets they create.
   */
  virtual void* provide_socket() = 0;

  /**
   * Will connect the given socket to the given address.
   */
  virtual bool connect(void* sock, const char* where) = 0;

  /**
   * Polls the socket, waiting for the specific timeout, upon data it calls the callback
   */
  virtual void poll(void *sock, uint32_t timeout, void(*callback)(const char*) ) = 0;

  /**
   * Will send the data through the socket
   */
  virtual void send(void* sock, const char* data, unsigned short length) = 0;

};

#endif

