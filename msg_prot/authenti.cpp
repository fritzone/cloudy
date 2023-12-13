#include "authenti.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string Authenticate::serialize() const
{
    std::string result = "<o><type>Authenticate</type>";
    result += "<attributes>";
    // attribute:user_name_hash
    result += "<user_name_hash>";
    result += stringify(m_user_name_hash);
    result += "</user_name_hash>";
    // attribute:password_hash
    result += "<password_hash>";
    result += stringify(m_password_hash);
    result += "</password_hash>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int Authenticate::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "Authenticate")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_user_name_hash = ezxml_child(attrs_node, "user_name_hash");
    destringify(m_user_name_hash, attr_node_user_name_hash->txt);
    ezxml_t attr_node_password_hash = ezxml_child(attrs_node, "password_hash");
    destringify(m_password_hash, attr_node_password_hash->txt);

    return 1;
}
bool Authenticate::operator == (const Authenticate& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_user_name_hash != rhs.m_user_name_hash) return false;
    if(m_password_hash != rhs.m_password_hash) return false;

    return true;
}
void Authenticate::set_user_name_hash(const std::string& p_user_name_hash)
{
    m_user_name_hash = p_user_name_hash;
}
void Authenticate::set_password_hash(const std::string& p_password_hash)
{
    m_password_hash = p_password_hash;
}
