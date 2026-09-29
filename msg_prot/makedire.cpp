#include "makedire.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string MakeDirectoryRequest::serialize() const
{
    std::string result = "<o><type>MakeDirectoryRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void MakeDirectoryRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:parent_hash
    result += "<parent_hash>";
    result += xml_escape(m_parent_hash);
    result += "</parent_hash>";
    // attribute:name
    result += "<name>";
    result += xml_escape(m_name);
    result += "</name>";
}

int MakeDirectoryRequest::deserialize(const char* xml)
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

int MakeDirectoryRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "MakeDirectoryRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int MakeDirectoryRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_parent_hash = ezxml_child(attrs_node, "parent_hash");
    if(attr_node_parent_hash) m_parent_hash = attr_node_parent_hash->txt;
    ezxml_t attr_node_name = ezxml_child(attrs_node, "name");
    if(attr_node_name) m_name = attr_node_name->txt;
    return 1;
}

bool MakeDirectoryRequest::operator == (const MakeDirectoryRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_parent_hash != rhs.m_parent_hash) return false;
    if(m_name != rhs.m_name) return false;

    return true;
}
void MakeDirectoryRequest::set_parent_hash(const std::string& p_parent_hash)
{
    m_parent_hash = p_parent_hash;
}
void MakeDirectoryRequest::set_name(const std::string& p_name)
{
    m_name = p_name;
}
