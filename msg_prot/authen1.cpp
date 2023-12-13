#include "authen1.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string AuthenticationStatus::serialize() const
{
    std::string result = "<o><type>AuthenticationStatus</type>";
    result += "<attributes>";
    // attribute:success
    result += "<success>";
    result += stringify(m_success);
    result += "</success>";
    // attribute:rights
    result += "<rights>";
    result += stringify(m_rights);
    result += "</rights>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int AuthenticationStatus::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "AuthenticationStatus")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_success = ezxml_child(attrs_node, "success");
    destringify(m_success, attr_node_success->txt);
    ezxml_t attr_node_rights = ezxml_child(attrs_node, "rights");
    destringify(m_rights, attr_node_rights->txt);

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
