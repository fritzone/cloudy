#include "connecti.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string ConnectionRequestReply::serialize() const
{
    std::string result = "<o><type>ConnectionRequestReply</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void ConnectionRequestReply::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:accepted
    result += "<accepted>";
    result += std::string(m_accepted ? "1" : "0");
    result += "</accepted>";
    // attribute:authentication_required
    result += "<authentication_required>";
    result += std::string(m_authentication_required ? "1" : "0");
    result += "</authentication_required>";
    // attribute:host_name
    result += "<host_name>";
    result += xml_escape(m_host_name);
    result += "</host_name>";
}

int ConnectionRequestReply::deserialize(const char* xml)
{
    size_t len = strlen(xml);
    char* copy = (char*)malloc(len + 1);
    if(!copy) return 0;
    memcpy(copy, xml, len + 1);
    ezxml_t x = ezxml_parse_str(copy, len);
    int result = deserialize(x);
    ezxml_free(x);
    free(copy);
    return result;
}

int ConnectionRequestReply::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "ConnectionRequestReply")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int ConnectionRequestReply::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_accepted = ezxml_child(attrs_node, "accepted");
    if(attr_node_accepted) m_accepted = xml_to_bool(attr_node_accepted->txt);
    ezxml_t attr_node_authentication_required = ezxml_child(attrs_node, "authentication_required");
    if(attr_node_authentication_required) m_authentication_required = xml_to_bool(attr_node_authentication_required->txt);
    ezxml_t attr_node_host_name = ezxml_child(attrs_node, "host_name");
    if(attr_node_host_name) m_host_name = attr_node_host_name->txt;
    return 1;
}

bool ConnectionRequestReply::operator == (const ConnectionRequestReply& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_accepted != rhs.m_accepted) return false;
    if(m_authentication_required != rhs.m_authentication_required) return false;
    if(m_host_name != rhs.m_host_name) return false;

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
void ConnectionRequestReply::set_host_name(const std::string& p_host_name)
{
    m_host_name = p_host_name;
}
