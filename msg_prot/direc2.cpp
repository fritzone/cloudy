#include "direc2.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string DirectoryList::serialize() const
{
    std::string result = "<o><type>DirectoryList</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void DirectoryList::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:directory_name
    result += "<directory_name>";
    result += xml_escape(m_directory_name);
    result += "</directory_name>";
    // attribute:directory_hash
    result += "<directory_hash>";
    result += xml_escape(m_directory_hash);
    result += "</directory_hash>";
    // attribute:start
    result += "<start>";
    result += stringify(m_start);
    result += "</start>";
    // attribute:total
    result += "<total>";
    result += stringify(m_total);
    result += "</total>";
    // attribute:error
    result += "<error>";
    result += xml_escape(m_error);
    result += "</error>";
    // attribute:entries
    result += "<entries>";
    for(size_t i=0; i<m_entries.size(); i++)
    {
        result += "<item>" + m_entries[i].serialize() + "</item>";
    }
    result += "</entries>";
}

int DirectoryList::deserialize(const char* xml)
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

int DirectoryList::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "DirectoryList")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int DirectoryList::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_directory_name = ezxml_child(attrs_node, "directory_name");
    if(attr_node_directory_name) m_directory_name = attr_node_directory_name->txt;
    ezxml_t attr_node_directory_hash = ezxml_child(attrs_node, "directory_hash");
    if(attr_node_directory_hash) m_directory_hash = attr_node_directory_hash->txt;
    ezxml_t attr_node_start = ezxml_child(attrs_node, "start");
    if(attr_node_start) m_start = atoi(attr_node_start->txt);
    ezxml_t attr_node_total = ezxml_child(attrs_node, "total");
    if(attr_node_total) m_total = atoi(attr_node_total->txt);
    ezxml_t attr_node_error = ezxml_child(attrs_node, "error");
    if(attr_node_error) m_error = attr_node_error->txt;
    ezxml_t attr_node_entries = ezxml_child(attrs_node, "entries");
    m_entries.clear();
    for(ezxml_t item = ezxml_child(attr_node_entries, "item"); item; item = item->next)
    {
        DirectoryEntry l_item;
        if(!l_item.deserialize(ezxml_child(item, "o"))) return 0;
        m_entries.push_back(l_item);
    }
    return 1;
}

bool DirectoryList::operator == (const DirectoryList& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_directory_name != rhs.m_directory_name) return false;
    if(m_directory_hash != rhs.m_directory_hash) return false;
    if(m_start != rhs.m_start) return false;
    if(m_total != rhs.m_total) return false;
    if(m_error != rhs.m_error) return false;
    if(m_entries.size() != rhs.m_entries.size() ) return false;
    for(size_t i_entries = 0; i_entries < m_entries.size(); i_entries++)
        if(!(m_entries[i_entries] == rhs.m_entries[i_entries])) return false;

    return true;
}
void DirectoryList::set_directory_name(const std::string& p_directory_name)
{
    m_directory_name = p_directory_name;
}
void DirectoryList::set_directory_hash(const std::string& p_directory_hash)
{
    m_directory_hash = p_directory_hash;
}
void DirectoryList::set_start(int p_start)
{
    m_start = p_start;
}
void DirectoryList::set_total(int p_total)
{
    m_total = p_total;
}
void DirectoryList::set_error(const std::string& p_error)
{
    m_error = p_error;
}
void DirectoryList::set_entries(const std::vector<DirectoryEntry>& p_entries)
{
    m_entries = p_entries;
}
