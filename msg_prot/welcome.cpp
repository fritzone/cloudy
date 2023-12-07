#include "welcome.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>

std::string Welcome::serialize() const
{
    std::string result = "<o><type>Welcome</type>";
    result += "<attributes>";
    // attribute:accepted
    result += "<accepted>";
    result += stringify(accepted);
    result += "</accepted>";
    // attribute:authentication_required
    result += "<authentication_required>";
    result += stringify(authentication_required);
    result += "</authentication_required>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int Welcome::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "Welcome")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_accepted = ezxml_child(attrs_node, "accepted");
    destringify(accepted, attr_node_accepted->txt);
    ezxml_t attr_node_authentication_required = ezxml_child(attrs_node, "authentication_required");
    destringify(authentication_required, attr_node_authentication_required->txt);

    return 1;
}
bool Welcome::operator == (const Welcome& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(accepted != rhs.accepted) return false;
    if(authentication_required != rhs.authentication_required) return false;

    return true;
}
void Welcome::set_accepted(bool p_accepted)
{
    accepted = p_accepted;
}
void Welcome::set_authentication_required(bool p_authentication_required)
{
    authentication_required = p_authentication_required;
}
