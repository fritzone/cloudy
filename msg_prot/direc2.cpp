#include "direc2.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string DirectoryList::serialize() const
{
    std::string result = "<o><type>DirectoryList</type>";
    result += "<attributes>";
    // attribute:directory_name
    result += "<directory_name>";
    result += stringify(m_directory_name);
    result += "</directory_name>";
    // attribute:entries
    result += "<entries>";
    for(int i=0; i<m_entries.size(); i++) result += "<item i=\"" + stringify(i) + "\">" + m_entries[i].serialize() + "</item>";
    result += "</entries>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int DirectoryList::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "DirectoryList")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_directory_name = ezxml_child(attrs_node, "directory_name");
    destringify(m_directory_name, attr_node_directory_name->txt);
    ezxml_t attr_node_entries = ezxml_child(attrs_node, "entries");
    for (ezxml_t item = ezxml_child(attr_node_entries, "item"); item; item = item->next) {
        DirectoryEntry l_DirectoryEntry;
        ezxml_t items_o = ezxml_child(item, "o");
        char * l_attr_data = ezxml_toxml(items_o);
        if(!l_DirectoryEntry.deserialize(l_attr_data))  {free(l_attr_data); return 0;}
        free(l_attr_data);
        m_entries.push_back(l_DirectoryEntry);

    }
    return 1;
}
bool DirectoryList::operator == (const DirectoryList& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_directory_name != rhs.m_directory_name) return false;
    // Checking vector
    if(m_entries.size() != rhs.m_entries.size() ) return false;

    return true;
}
void DirectoryList::set_directory_name(const std::string& p_directory_name)
{
    m_directory_name = p_directory_name;
}
void DirectoryList::set_entries(const std::vector<DirectoryEntry>& p_entries)
{
    m_entries = p_entries;
}
