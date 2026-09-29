#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>
#include <map>
#include <vector>

#include "msg_prot/protocol.h"

class NetworkInterface;

/**
 * How many bytes of DOS memory are still free
 */
unsigned long freeDosMemory();

/**
  * The way this works is like:
  *
  *  - the network interface hands over the '\0' terminated frames it received
  *    to onFrameReceived
  *  - that parses the XML envelope and gets the message type out of it
  *  - Protocol::receive deserializes the message
  *  - and calls the corresponding function pointer which was registered
 */
class ProtocolImpl : public Protocol
{
public:
    ProtocolImpl();

    /**
     * Sets the network interface and the socket used for sending messages
     */
    void setNetworkInterface(NetworkInterface* iface, void* socket);

    NetworkInterface* networkInterface() const { return network; }
    void* socket() const { return sock; }

    /**
     * @brief envelope will pack the serialized message m into an XML envelop, ready to be sent
     * @param m the message to pack
     * @return the enveloped serialized message
     */
    std::string envelope(const Message *m);

    /**
     * Sends the message to the peer. Returns false if it could not.
     */
    bool send(const Message* m);

    /**
     * Polls the network for timeout milliseconds and dispatches what came
     */
    void poll(unsigned long timeout);

    /**
     * Handles one received frame
     */
    void onFrameReceived(char* frame);

private:

    NetworkInterface* network;
    void* sock;
};

#endif
