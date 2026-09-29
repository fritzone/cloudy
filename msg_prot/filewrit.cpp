#include "filewrit.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string FileWriteRequest::serialize() const
{
    std::string result = "<o><type>FileWriteRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void FileWriteRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:directory_hash
    result += "<directory_hash>";
    result += xml_escape(m_directory_hash);
    result += "</directory_hash>";
    // attribute:name
    result += "<name>";
    result += xml_escape(m_name);
    result += "</name>";
    // attribute:offset
    result += "<offset>";
    result += stringify(m_offset);
    result += "</offset>";
    // attribute:data
    result += "<data>";
    result += xml_escape(m_data);
    result += "</data>";
    // attribute:last
    result += "<last>";
    result += std::string(m_last ? "1" : "0");
    result += "</last>";
}

int FileWriteRequest::deserialize(const char* xml)
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

int FileWriteRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "FileWriteRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int FileWriteRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_directory_hash = ezxml_child(attrs_node, "directory_hash");
    if(attr_node_directory_hash) m_directory_hash = attr_node_directory_hash->txt;
    ezxml_t attr_node_name = ezxml_child(attrs_node, "name");
    if(attr_node_name) m_name = attr_node_name->txt;
    ezxml_t attr_node_offset = ezxml_child(attrs_node, "offset");
    if(attr_node_offset) m_offset = atol(attr_node_offset->txt);
    ezxml_t attr_node_data = ezxml_child(attrs_node, "data");
    if(attr_node_data) m_data = attr_node_data->txt;
    ezxml_t attr_node_last = ezxml_child(attrs_node, "last");
    if(attr_node_last) m_last = xml_to_bool(attr_node_last->txt);
    return 1;
}

bool FileWriteRequest::operator == (const FileWriteRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_directory_hash != rhs.m_directory_hash) return false;
    if(m_name != rhs.m_name) return false;
    if(m_offset != rhs.m_offset) return false;
    if(m_data != rhs.m_data) return false;
    if(m_last != rhs.m_last) return false;

    return true;
}
void FileWriteRequest::set_directory_hash(const std::string& p_directory_hash)
{
    m_directory_hash = p_directory_hash;
}
void FileWriteRequest::set_name(const std::string& p_name)
{
    m_name = p_name;
}
void FileWriteRequest::set_offset(long p_offset)
{
    m_offset = p_offset;
}
void FileWriteRequest::set_data(const std::string& p_data)
{
    m_data = p_data;
}
void FileWriteRequest::set_last(bool p_last)
{
    m_last = p_last;
}
