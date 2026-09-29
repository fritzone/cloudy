#include "filedata.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string FileData::serialize() const
{
    std::string result = "<o><type>FileData</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void FileData::serialize_attributes(std::string& result) const
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
    // attribute:data
    result += "<data>";
    result += xml_escape(m_data);
    result += "</data>";
    // attribute:eof
    result += "<eof>";
    result += std::string(m_eof ? "1" : "0");
    result += "</eof>";
    // attribute:error
    result += "<error>";
    result += xml_escape(m_error);
    result += "</error>";
}

int FileData::deserialize(const char* xml)
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

int FileData::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "FileData")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int FileData::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_file_hash = ezxml_child(attrs_node, "file_hash");
    if(attr_node_file_hash) m_file_hash = attr_node_file_hash->txt;
    ezxml_t attr_node_offset = ezxml_child(attrs_node, "offset");
    if(attr_node_offset) m_offset = atol(attr_node_offset->txt);
    ezxml_t attr_node_data = ezxml_child(attrs_node, "data");
    if(attr_node_data) m_data = attr_node_data->txt;
    ezxml_t attr_node_eof = ezxml_child(attrs_node, "eof");
    if(attr_node_eof) m_eof = xml_to_bool(attr_node_eof->txt);
    ezxml_t attr_node_error = ezxml_child(attrs_node, "error");
    if(attr_node_error) m_error = attr_node_error->txt;
    return 1;
}

bool FileData::operator == (const FileData& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_file_hash != rhs.m_file_hash) return false;
    if(m_offset != rhs.m_offset) return false;
    if(m_data != rhs.m_data) return false;
    if(m_eof != rhs.m_eof) return false;
    if(m_error != rhs.m_error) return false;

    return true;
}
void FileData::set_file_hash(const std::string& p_file_hash)
{
    m_file_hash = p_file_hash;
}
void FileData::set_offset(long p_offset)
{
    m_offset = p_offset;
}
void FileData::set_data(const std::string& p_data)
{
    m_data = p_data;
}
void FileData::set_eof(bool p_eof)
{
    m_eof = p_eof;
}
void FileData::set_error(const std::string& p_error)
{
    m_error = p_error;
}
