#include "direct1.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string DirectoryListRequest::serialize() const
{
    std::string result = "<o><type>DirectoryListRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void DirectoryListRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:directory_hash
    result += "<directory_hash>";
    result += xml_escape(m_directory_hash);
    result += "</directory_hash>";
    // attribute:start
    result += "<start>";
    result += stringify(m_start);
    result += "</start>";
}

int DirectoryListRequest::deserialize(const char* xml)
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

int DirectoryListRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "DirectoryListRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int DirectoryListRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_directory_hash = ezxml_child(attrs_node, "directory_hash");
    if(attr_node_directory_hash) m_directory_hash = attr_node_directory_hash->txt;
    ezxml_t attr_node_start = ezxml_child(attrs_node, "start");
    if(attr_node_start) m_start = atoi(attr_node_start->txt);
    return 1;
}

bool DirectoryListRequest::operator == (const DirectoryListRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_directory_hash != rhs.m_directory_hash) return false;
    if(m_start != rhs.m_start) return false;

    return true;
}
void DirectoryListRequest::set_directory_hash(const std::string& p_directory_hash)
{
    m_directory_hash = p_directory_hash;
}
void DirectoryListRequest::set_start(int p_start)
{
    m_start = p_start;
}
