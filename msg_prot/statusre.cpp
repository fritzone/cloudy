#include "statusre.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string StatusRequest::serialize() const
{
    std::string result = "<o><type>StatusRequest</type>";
    result += "</o>";
    return result;
}
int StatusRequest::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "StatusRequest")) return 0;
    return 1;
}
bool StatusRequest::operator == (const StatusRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;

    return true;
}
