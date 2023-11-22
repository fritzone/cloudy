#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>
#include <map>
#include <vector>

class NetworkInterface;

void onDataReceived(const char* data);

/**
 * The base class for all messages.
 */
struct message
{
  message() : sequence_nr(sequence_nr_counter ++) {}

  // the unique identifier of the message in the system is a sequence number
  unsigned long sequence_nr;

  // this is incremented with each creation of any message
  static unsigned long sequence_nr_counter;
};

struct welcome : public message
{
};

/**
  * The way this works is like:
  *
  *  - in the main receiver function we get XML data
  *  - we parse out a message from it
  *  - we send it to the protocol.message
  *  - that identifies the message type
  *  - and calls the corresponding function pointer which was registered
  *  - the corresponding message handler handles the message, and it might call
  *    a callback function with the message data
 *
 */
class protocol
{
public:
  protocol();

  /**
   * This gets the parsed out message from the layer above, with the message type specified
   */
  void message(const char* message_type, const char* message_data);

  /**
   * Sets the network interface used in this protocol
   */
  void setNetworkInterface(NetworkInterface* iface);

  /**
   * The typedef for the error handler and callback functions.
   * The first parameter is the error message.
   * The second parameter is the message object that generated the error.
   * The third parameter is a user defined pointer.
   */
  typedef void(*ErrorHandler)(const char*, struct message*, void*);
  typedef void(*Callback)(const char*, struct message*, void*);
  /**
   * The typedef for the message handler function.
   *
   * @param data - the data of the message, in XML format for now
   * @param eh - the error handler function
   * @param cb - the callback function
   */
  typedef void(*MessageHandler)(const char* data, void*);

  /**
   * Registers a message handler function for the given message type
   */
  void registerMessageHandler(const char* message_type, MessageHandler mh, void* up);

private:

  struct MessageHandlerStruct
  {
    void* userParameter;
    MessageHandler messageHandler;
  };

  // the map of message handlers. A message can be handled by more than one
  // handler, hence we store them in a vector
  std::map<std::string, std::vector<MessageHandlerStruct> > handlers;

  NetworkInterface* network;
};

#endif

