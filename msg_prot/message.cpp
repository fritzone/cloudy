#include "message.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>

int Message::seq_message_id = 0;

std::string Message::serialize() const
{
    std::string result = "<o><type>Message</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void Message::serialize_attributes(std::string& result) const
{
    // attribute:message_id
    result += "<message_id>";
    result += stringify(m_message_id);
    result += "</message_id>";
}

int Message::deserialize(const char* xml)
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

int Message::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "Message")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int Message::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    ezxml_t attr_node_message_id = ezxml_child(attrs_node, "message_id");
    if(attr_node_message_id) m_message_id = atoi(attr_node_message_id->txt);
    return 1;
}

bool Message::operator == (const Message& rhs) const
{
    if(m_message_id != rhs.m_message_id) return false;

    return true;
}
