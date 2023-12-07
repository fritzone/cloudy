#include "connectr.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>

std::string ConnectRequest::serialize() const
{
    std::string result = "<o><type>ConnectRequest</type>";
    result += "<attributes>";
    // attribute:platform
    result += "<platform>";
    result += stringify(platform);
    result += "</platform>";
    // attribute:unique_id
    result += "<unique_id>";
    result += stringify(unique_id);
    result += "</unique_id>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int ConnectRequest::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "ConnectRequest")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_platform = ezxml_child(attrs_node, "platform");
    destringify(platform, attr_node_platform->txt);
    ezxml_t attr_node_unique_id = ezxml_child(attrs_node, "unique_id");
    destringify(unique_id, attr_node_unique_id->txt);

    return 1;
}
bool ConnectRequest::operator == (const ConnectRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(platform != rhs.platform) return false;
    if(unique_id != rhs.unique_id) return false;

    return true;
}
void ConnectRequest::set_platform(const std::string& p_platform)
{
    if (p_platform != "dos" && p_platform != "linux") { return; }
    platform = p_platform;
}
void ConnectRequest::set_unique_id(const std::string& p_unique_id)
{
    unique_id = p_unique_id;
}
