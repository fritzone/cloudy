#ifndef _NETINTERFACE_H_
#define _NETINTERFACE_H_

#include <types.h>

/**
 * Called for each complete frame received. The frame is '\0' terminated and
 * the callee may modify it in place, but must not keep a pointer to it.
 */
typedef void (*FrameCallback)(void* data, char* frame);

/*
 * This class will handle the network setup, each system will have to derive from it
 * in order to properly initialize its network capabilities.
 *
 * On the wire the messages are separated by a '\0' byte.
 */
class NetworkInterface
{
public:

    NetworkInterface() {}
    virtual ~NetworkInterface() {}

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
    virtual bool connect(void* sock, const char* where, uint16_t port) = 0;

    /**
   * Returns false once the other side closed the connection
   */
    virtual bool isConnected(void* sock) = 0;

    /**
   * Processes the network for at most timeout milliseconds (0: just once) and
   * calls the callback for every complete frame that arrived.
   */
    virtual void poll(void *sock, uint32_t timeout, void* data, FrameCallback callback) = 0;

    /**
   * Sends all the data through the socket, returns false if it could not.
   */
    virtual bool send(void* sock, const char* data, uint16_t length) = 0;
};

#endif
