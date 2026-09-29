#include "connectr.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string ConnectRequest::serialize() const
{
    std::string result = "<o><type>ConnectRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void ConnectRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:platform
    result += "<platform>";
    result += xml_escape(m_platform);
    result += "</platform>";
    // attribute:unique_id
    result += "<unique_id>";
    result += xml_escape(m_unique_id);
    result += "</unique_id>";
}

int ConnectRequest::deserialize(const char* xml)
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

int ConnectRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "ConnectRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int ConnectRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_platform = ezxml_child(attrs_node, "platform");
    if(attr_node_platform) m_platform = attr_node_platform->txt;
    ezxml_t attr_node_unique_id = ezxml_child(attrs_node, "unique_id");
    if(attr_node_unique_id) m_unique_id = attr_node_unique_id->txt;
    return 1;
}

bool ConnectRequest::operator == (const ConnectRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_platform != rhs.m_platform) return false;
    if(m_unique_id != rhs.m_unique_id) return false;

    return true;
}
void ConnectRequest::set_platform(const std::string& p_platform)
{
    if (p_platform != "dos" && p_platform != "linux") { return; }
    m_platform = p_platform;
}
void ConnectRequest::set_unique_id(const std::string& p_unique_id)
{
    m_unique_id = p_unique_id;
}
