#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezxml.h"
#include "prot.h"
#include "cldutils.h"


ProtocolImpl::ProtocolImpl()
{
}


std::string ProtocolImpl::envelope(const Message *m)
{
    std::string scr = m->serialize();
    static const std::string envelope_part1 = "<?xml version=\"1.0\"><protocol><cld v=\"1.0\" msg=\"";
    return envelope_part1 + m->name() + "\">" + scr + "</cld></protocol>";
}

void ProtocolImpl::setNetworkInterface(NetworkInterface* iface)
{
    network = iface;
}

