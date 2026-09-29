#include "filewr1.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string FileWriteReply::serialize() const
{
    std::string result = "<o><type>FileWriteReply</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void FileWriteReply::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:name
    result += "<name>";
    result += xml_escape(m_name);
    result += "</name>";
    // attribute:offset
    result += "<offset>";
    result += stringify(m_offset);
    result += "</offset>";
    // attribute:ok
    result += "<ok>";
    result += std::string(m_ok ? "1" : "0");
    result += "</ok>";
    // attribute:error
    result += "<error>";
    result += xml_escape(m_error);
    result += "</error>";
}

int FileWriteReply::deserialize(const char* xml)
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

int FileWriteReply::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "FileWriteReply")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int FileWriteReply::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_name = ezxml_child(attrs_node, "name");
    if(attr_node_name) m_name = attr_node_name->txt;
    ezxml_t attr_node_offset = ezxml_child(attrs_node, "offset");
    if(attr_node_offset) m_offset = atol(attr_node_offset->txt);
    ezxml_t attr_node_ok = ezxml_child(attrs_node, "ok");
    if(attr_node_ok) m_ok = xml_to_bool(attr_node_ok->txt);
    ezxml_t attr_node_error = ezxml_child(attrs_node, "error");
    if(attr_node_error) m_error = attr_node_error->txt;
    return 1;
}

bool FileWriteReply::operator == (const FileWriteReply& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_name != rhs.m_name) return false;
    if(m_offset != rhs.m_offset) return false;
    if(m_ok != rhs.m_ok) return false;
    if(m_error != rhs.m_error) return false;

    return true;
}
void FileWriteReply::set_name(const std::string& p_name)
{
    m_name = p_name;
}
void FileWriteReply::set_offset(long p_offset)
{
    m_offset = p_offset;
}
void FileWriteReply::set_ok(bool p_ok)
{
    m_ok = p_ok;
}
void FileWriteReply::set_error(const std::string& p_error)
{
    m_error = p_error;
}
