#include "statusre.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string StatusRequest::serialize() const
{
    std::string result = "<o><type>StatusRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void StatusRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
}

int StatusRequest::deserialize(const char* xml)
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

int StatusRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "StatusRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int StatusRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    return 1;
}

bool StatusRequest::operator == (const StatusRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;

    return true;
}
