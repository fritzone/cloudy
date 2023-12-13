#include "connecti.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string ConnectionRequestReply::serialize() const
{
    std::string result = "<o><type>ConnectionRequestReply</type>";
    result += "<attributes>";
    // attribute:accepted
    result += "<accepted>";
    result += stringify(m_accepted);
    result += "</accepted>";
    // attribute:authentication_required
    result += "<authentication_required>";
    result += stringify(m_authentication_required);
    result += "</authentication_required>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int ConnectionRequestReply::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "ConnectionRequestReply")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_accepted = ezxml_child(attrs_node, "accepted");
    destringify(m_accepted, attr_node_accepted->txt);
    ezxml_t attr_node_authentication_required = ezxml_child(attrs_node, "authentication_required");
    destringify(m_authentication_required, attr_node_authentication_required->txt);

    return 1;
}
bool ConnectionRequestReply::operator == (const ConnectionRequestReply& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_accepted != rhs.m_accepted) return false;
    if(m_authentication_required != rhs.m_authentication_required) return false;

    return true;
}
void ConnectionRequestReply::set_accepted(bool p_accepted)
{
    m_accepted = p_accepted;
}
void ConnectionRequestReply::set_authentication_required(bool p_authentication_required)
{
    m_authentication_required = p_authentication_required;
}
