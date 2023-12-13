#include "director.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string DirectoryEntry::serialize() const
{
    std::string result = "<o><type>DirectoryEntry</type>";
    result += "<attributes>";
    // attribute:name
    result += "<name>";
    result += stringify(m_name);
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
    result += stringify(m_attrs);
    result += "</attrs>";
    // attribute:container
    result += "<container>";
    result += stringify(m_container);
    result += "</container>";
    // attribute:hash
    result += "<hash>";
    result += stringify(m_hash);
    result += "</hash>";
    // attribute:host_id
    result += "<host_id>";
    result += stringify(m_host_id);
    result += "</host_id>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int DirectoryEntry::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "DirectoryEntry")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_name = ezxml_child(attrs_node, "name");
    destringify(m_name, attr_node_name->txt);
    ezxml_t attr_node_date = ezxml_child(attrs_node, "date");
    destringify(m_date, attr_node_date->txt);
    ezxml_t attr_node_time = ezxml_child(attrs_node, "time");
    destringify(m_time, attr_node_time->txt);
    ezxml_t attr_node_size = ezxml_child(attrs_node, "size");
    destringify(m_size, attr_node_size->txt);
    ezxml_t attr_node_attrs = ezxml_child(attrs_node, "attrs");
    destringify(m_attrs, attr_node_attrs->txt);
    ezxml_t attr_node_container = ezxml_child(attrs_node, "container");
    destringify(m_container, attr_node_container->txt);
    ezxml_t attr_node_hash = ezxml_child(attrs_node, "hash");
    destringify(m_hash, attr_node_hash->txt);
    ezxml_t attr_node_host_id = ezxml_child(attrs_node, "host_id");
    destringify(m_host_id, attr_node_host_id->txt);

    return 1;
}
bool DirectoryEntry::operator == (const DirectoryEntry& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_name != rhs.m_name) return false;
    if(m_date != rhs.m_date) return false;
    if(m_time != rhs.m_time) return false;
    if(m_size != rhs.m_size) return false;
    if(m_attrs != rhs.m_attrs) return false;
    if(m_container != rhs.m_container) return false;
    if(m_hash != rhs.m_hash) return false;
    if(m_host_id != rhs.m_host_id) return false;

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
void DirectoryEntry::set_container(const std::string& p_container)
{
    m_container = p_container;
}
void DirectoryEntry::set_hash(const std::string& p_hash)
{
    m_hash = p_hash;
}
void DirectoryEntry::set_host_id(const std::string& p_host_id)
{
    m_host_id = p_host_id;
}
