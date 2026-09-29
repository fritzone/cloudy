#include "authen1.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string AuthenticationStatus::serialize() const
{
    std::string result = "<o><type>AuthenticationStatus</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void AuthenticationStatus::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:success
    result += "<success>";
    result += std::string(m_success ? "1" : "0");
    result += "</success>";
    // attribute:rights
    result += "<rights>";
    result += xml_escape(m_rights);
    result += "</rights>";
}

int AuthenticationStatus::deserialize(const char* xml)
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

int AuthenticationStatus::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "AuthenticationStatus")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int AuthenticationStatus::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_success = ezxml_child(attrs_node, "success");
    if(attr_node_success) m_success = xml_to_bool(attr_node_success->txt);
    ezxml_t attr_node_rights = ezxml_child(attrs_node, "rights");
    if(attr_node_rights) m_rights = attr_node_rights->txt;
    return 1;
}

bool AuthenticationStatus::operator == (const AuthenticationStatus& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_success != rhs.m_success) return false;
    if(m_rights != rhs.m_rights) return false;

    return true;
}
void AuthenticationStatus::set_success(bool p_success)
{
    m_success = p_success;
}
void AuthenticationStatus::set_rights(const std::string& p_rights)
{
    m_rights = p_rights;
}
