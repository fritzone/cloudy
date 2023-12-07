#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "protocol.h"

int Protocol::messageId = 0;


Protocol::Protocol()
{
}


std::string Protocol::envelope(const Message *m)
{
    std::string scr = m->serialize();
    static const std::string envelope_part1 = "<?xml version=\"1.0\"><protocol><cld v=\"1.0\" msg=\"";
    return envelope_part1 + m->get_name() + "\">" + scr + "</cld></protocol>";
}

ConnectRequest* Protocol::createConnectRequest()
{
    ConnectRequest *cr = new ConnectRequest;
    cr->set_message_id(nextMessageId());
    cr->set_platform("dos");
    cr->set_unique_id(rand_string(12));

    return cr;
}

int Protocol::nextMessageId()
{
    return ++messageId;
}

char *Protocol::rand_string(size_t size)
{
    static char str[128];
    memset(str, 0, 128);

    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK123456789";
    if (size)
    {
        --size;
        for (size_t n = 0; n < size; n++)
        {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

void Protocol::setNetworkInterface(NetworkInterface* iface)
{
    network = iface;
}

