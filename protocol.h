#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>
#include <map>
#include <vector>

#include "msg_prot/connectr.h"

class NetworkInterface;

void onDataReceived(void *object, const char* data);

/**
  * The way this works is like:
  *
  *  - in the main receiver function we get XML data
  *  - we parse out a message from it
  *  - we send it to the Protocol.message
  *  - that identifies the message type
  *  - and calls the corresponding function pointer which was registered
  *  - the corresponding message handler handles the message, and it might call
  *    a callback function with the message data
 */
class Protocol
{
    static int messageId; // the zero based messageID used for the messages

public:
    Protocol();

    /**
     * Sets the network interface used in this Protocol
     */
    void setNetworkInterface(NetworkInterface* iface);

    /**
     * @brief envelope will pack the serialized message m into an XML envelop, ready to be sent
     * @param m the message to pack
     * @return the enveloped serialized message
     */
    std::string envelope(const Message *m);

    /**
     * @brief createConnectRequest Will create a connection request message
     * @return
     */
    ConnectRequest *createConnectRequest();

private:

    int nextMessageId();

    static char *rand_string(size_t size);

private:

    NetworkInterface* network;
};

#endif

