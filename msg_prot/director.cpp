#include "director.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string DirectoryEntry::serialize() const
{
    std::string result = "<o><type>DirectoryEntry</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void DirectoryEntry::serialize_attributes(std::string& result) const
{
    // attribute:name
    result += "<name>";
    result += xml_escape(m_name);
    result += "</name>";
    // attribute:date
    result += "<date>";
    result += stringify(m_date);
    result += "</date>";
    // attribute:time
    result += "<time>";
    result += stringify(m_time);
    result += "</time>";
    // attribute:size
    result += "<size>";
    result += stringify(m_size);
    result += "</size>";
    // attribute:attrs
    result += "<attrs>";
    result += xml_escape(m_attrs);
    result += "</attrs>";
    // attribute:hash
    result += "<hash>";
    result += xml_escape(m_hash);
    result += "</hash>";
}

int DirectoryEntry::deserialize(const char* xml)
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

int DirectoryEntry::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "DirectoryEntry")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int DirectoryEntry::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    ezxml_t attr_node_name = ezxml_child(attrs_node, "name");
    if(attr_node_name) m_name = attr_node_name->txt;
    ezxml_t attr_node_date = ezxml_child(attrs_node, "date");
    if(attr_node_date) m_date = atol(attr_node_date->txt);
    ezxml_t attr_node_time = ezxml_child(attrs_node, "time");
    if(attr_node_time) m_time = atol(attr_node_time->txt);
    ezxml_t attr_node_size = ezxml_child(attrs_node, "size");
    if(attr_node_size) m_size = atol(attr_node_size->txt);
    ezxml_t attr_node_attrs = ezxml_child(attrs_node, "attrs");
    if(attr_node_attrs) m_attrs = attr_node_attrs->txt;
    ezxml_t attr_node_hash = ezxml_child(attrs_node, "hash");
    if(attr_node_hash) m_hash = attr_node_hash->txt;
    return 1;
}

bool DirectoryEntry::operator == (const DirectoryEntry& rhs) const
{
    if(m_name != rhs.m_name) return false;
    if(m_date != rhs.m_date) return false;
    if(m_time != rhs.m_time) return false;
    if(m_size != rhs.m_size) return false;
    if(m_attrs != rhs.m_attrs) return false;
    if(m_hash != rhs.m_hash) return false;

    return true;
}
void DirectoryEntry::set_name(const std::string& p_name)
{
    m_name = p_name;
}
void DirectoryEntry::set_date(long p_date)
{
    m_date = p_date;
}
void DirectoryEntry::set_time(long p_time)
{
    m_time = p_time;
}
void DirectoryEntry::set_size(long p_size)
{
    m_size = p_size;
}
void DirectoryEntry::set_attrs(const std::string& p_attrs)
{
    m_attrs = p_attrs;
}
void DirectoryEntry::set_hash(const std::string& p_hash)
{
    m_hash = p_hash;
}
