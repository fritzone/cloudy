#include "message.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>

int Message::seq_message_id = 0;

std::string Message::serialize() const
{
    std::string result = "<o><type>Message</type>";
    result += "<attributes>";
    // attribute:message_id
    result += "<message_id>";
    result += stringify(m_message_id);
    result += "</message_id>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int Message::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "Message")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_message_id = ezxml_child(attrs_node, "message_id");
    destringify(m_message_id, attr_node_message_id->txt);

    return 1;
}
bool Message::operator == (const Message& rhs) const
{
    if(m_message_id != rhs.m_message_id) return false;

    return true;
}
