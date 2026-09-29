
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "ezxml.h"
#include "prot.h"
#include "net_ifce.h"
#include "log.h"


ProtocolImpl::ProtocolImpl() : network(NULL), sock(NULL)
{
}


std::string ProtocolImpl::envelope(const Message *m)
{
    std::string scr = m->serialize();
    static const std::string envelope_part1 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><protocol><cld v=\"1.0\" msg=\"";
    return envelope_part1 + m->name() + "\">" + scr + "</cld></protocol>";
}

void ProtocolImpl::setNetworkInterface(NetworkInterface* iface, void* socket)
{
    network = iface;
    sock = socket;
}

bool ProtocolImpl::send(const Message *m)
{
    if(network == NULL || sock == NULL)
    {
        log_error() << "Not connected, cannot send" << m->name();
        return false;
    }

    std::string data = envelope(m);
    log_debug() << "Sending" << data;

    // the '\0' at the end separates the messages
    return network->send(sock, data.c_str(), data.length() + 1);
}

static void frameReceived(void* object, char* frame)
{
    ((ProtocolImpl*)object)->onFrameReceived(frame);
}

void ProtocolImpl::poll(unsigned long timeout)
{
    if(network == NULL || sock == NULL)
    {
        return;
    }
    network->poll(sock, timeout, this, frameReceived);
}

unsigned long freeDosMemory()
{
    // asking for too much fails, and tells how much there is
    unsigned paragraphs = 0;
    _dos_allocmem(0xffff, &paragraphs);
    return (unsigned long)paragraphs * 16;
}

void ProtocolImpl::onFrameReceived(char* frame)
{
    log_debug() << "Received:" << frame;
    log_debug() << "Free DOS memory:" << freeDosMemory();

    ezxml_t doc = ezxml_parse_str(frame, strlen(frame));
    if(doc == NULL || *ezxml_error(doc))
    {
        log_error() << "Cannot parse the received frame:" << (doc ? ezxml_error(doc) : "");
        ezxml_free(doc);
        return;
    }

    ezxml_t cld = ezxml_child(doc, "cld");
    const char* msg_t = cld ? ezxml_attr(cld, "msg") : NULL;
    if(msg_t == NULL)
    {
        log_error() << "No cld tag or message type in the frame";
        ezxml_free(doc);
        return;
    }

    try
    {
        receive(msg_t, ezxml_child(cld, "o"));
    }
    catch(...)
    {
        // most likely out of memory
        log_error() << "Exception while handling" << msg_t << ", free memory:" << freeDosMemory();
    }

    ezxml_free(doc);
}
