#include "authenti.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string Authenticate::serialize() const
{
    std::string result = "<o><type>Authenticate</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void Authenticate::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:user_name_hash
    result += "<user_name_hash>";
    result += xml_escape(m_user_name_hash);
    result += "</user_name_hash>";
    // attribute:password_hash
    result += "<password_hash>";
    result += xml_escape(m_password_hash);
    result += "</password_hash>";
}

int Authenticate::deserialize(const char* xml)
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

int Authenticate::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "Authenticate")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int Authenticate::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_user_name_hash = ezxml_child(attrs_node, "user_name_hash");
    if(attr_node_user_name_hash) m_user_name_hash = attr_node_user_name_hash->txt;
    ezxml_t attr_node_password_hash = ezxml_child(attrs_node, "password_hash");
    if(attr_node_password_hash) m_password_hash = attr_node_password_hash->txt;
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
