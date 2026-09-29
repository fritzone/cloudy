#include "fileread.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string FileReadRequest::serialize() const
{
    std::string result = "<o><type>FileReadRequest</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void FileReadRequest::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:file_hash
    result += "<file_hash>";
    result += xml_escape(m_file_hash);
    result += "</file_hash>";
    // attribute:offset
    result += "<offset>";
    result += stringify(m_offset);
    result += "</offset>";
    // attribute:length
    result += "<length>";
    result += stringify(m_length);
    result += "</length>";
}

int FileReadRequest::deserialize(const char* xml)
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

int FileReadRequest::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "FileReadRequest")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int FileReadRequest::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_file_hash = ezxml_child(attrs_node, "file_hash");
    if(attr_node_file_hash) m_file_hash = attr_node_file_hash->txt;
    ezxml_t attr_node_offset = ezxml_child(attrs_node, "offset");
    if(attr_node_offset) m_offset = atol(attr_node_offset->txt);
    ezxml_t attr_node_length = ezxml_child(attrs_node, "length");
    if(attr_node_length) m_length = atoi(attr_node_length->txt);
    return 1;
}

bool FileReadRequest::operator == (const FileReadRequest& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_file_hash != rhs.m_file_hash) return false;
    if(m_offset != rhs.m_offset) return false;
    if(m_length != rhs.m_length) return false;

    return true;
}
void FileReadRequest::set_file_hash(const std::string& p_file_hash)
{
    m_file_hash = p_file_hash;
}
void FileReadRequest::set_offset(long p_offset)
{
    m_offset = p_offset;
}
void FileReadRequest::set_length(int p_length)
{
    m_length = p_length;
}
