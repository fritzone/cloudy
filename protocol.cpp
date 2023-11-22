#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "protocol.h"

unsigned long message::sequence_nr_counter = 0;


/**
 * This is a welcome message handler. Contains just an ID, so that the host knows we exist.
 *
 * The following is the format of the XML we expect:
 *
 *  <?xml version="1.0">
 *  <protocol>
 *   <cld v="1.0">
 *    <msg t="welcome">
 *     <id>12345</id>
 *    <msg>
 *   </cld>
 *  </protocol>
 */
static void welcomeHandler(const char* welcomeMessage, void*)
{
  ezxml_t doc = ezxml_parse_str((char*)welcomeMessage, strlen(welcomeMessage));
  char* s = ezxml_toxml(doc);

  ezxml_t id_node = ezxml_child(doc, "id");
  const char* id = id_node->txt;
  fprintf(stderr, "Welcome: %s as %s \n", s, id);

  free(s);
  ezxml_free(doc);
}


protocol::protocol()
{
  registerMessageHandler("welcome", welcomeHandler, NULL);
}

void protocol::message(const char* type, const char* data)
{
   std::string key(type);
   if(handlers.count(key))
   {
     std::vector<MessageHandlerStruct>& mh = handlers[key];
     for(int i=0; i<mh.size(); i++)
     {
       mh[i].messageHandler(data, mh[i].userParameter);
     }
   }
}

void protocol::registerMessageHandler(const char* message_type, MessageHandler mh, void* up)
{
  struct MessageHandlerStruct mhs;
  mhs.userParameter = up;
  mhs.messageHandler = mh;
  std::string key = message_type;
  if(handlers.count(key))
  {
    handlers[key].push_back(mhs);
  }
  else
  {
    std::vector<MessageHandlerStruct> tvmhs;
    tvmhs.push_back(mhs);
    handlers[key] = tvmhs;
  }

}

void protocol::setNetworkInterface(NetworkInterface* iface)
{
  network = iface;
}

