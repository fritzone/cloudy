#include "direct1.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string DirectoryListRequest::serialize() const
{
    std::string result = "<o><type>DirectoryListRequest</type>";
    result += "<attributes>";
    // attribute:directory_hash
    result += "<directory_hash>";
    result += stringify(m_directory_hash);
    result += "</directory_hash>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int DirectoryListRequest::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "DirectoryListRequest")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_directory_hash = ezxml_child(attrs_node, "directory_hash");
    destringify(m_directory_hash, attr_node_directory_hash->txt);

    return 1;
}
bool DirectoryListRequest::operator == (const DirectoryListRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_directory_hash != rhs.m_directory_hash) return false;

    return true;
}
void DirectoryListRequest::set_directory_hash(const std::string& p_directory_hash)
{
    m_directory_hash = p_directory_hash;
}
