#include "status.h"
#include "strngify.h"

#include "ezxml.h"

#include <string.h>


std::string Status::serialize() const
{
    std::string result = "<o><type>Status</type>";
    result += "<attributes>";
    // attribute:host_platform
    result += "<host_platform>";
    result += stringify(m_host_platform);
    result += "</host_platform>";
    // attribute:drives
    result += "<drives>";
    for(int i=0; i<m_drives.size(); i++) result += "<item i=\"" + stringify(i) + "\">" + stringify(m_drives[i]) + "</item>";
    result += "</drives>";
    // attribute:free_space
    result += "<free_space>";
    for(int i=0; i<m_free_space.size(); i++) result += "<item i=\"" + stringify(i) + "\">" + stringify(m_free_space[i]) + "</item>";
    result += "</free_space>";
    // attribute:working_directories
    result += "<working_directories>";
    for(int i=0; i<m_working_directories.size(); i++) result += "<item i=\"" + stringify(i) + "\">" + stringify(m_working_directories[i]) + "</item>";
    result += "</working_directories>";
    // attribute:current__work_index
    result += "<current__work_index>";
    result += stringify(m_current__work_index);
    result += "</current__work_index>";
    result += "</attributes>";
    result += "</o>";
    return result;
}
int Status::deserialize(const char* xml)
{
    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));
    ezxml_t type_node = ezxml_child(x, "type");
    const char* str_type = type_node->txt;
    if(strcmp(str_type, "Status")) return 0;    ezxml_t attrs_node = ezxml_child(x, "attributes");
    if(!attrs_node) return 0;
    ezxml_t attr_node_host_platform = ezxml_child(attrs_node, "host_platform");
    destringify(m_host_platform, attr_node_host_platform->txt);
    ezxml_t attr_node_drives = ezxml_child(attrs_node, "drives");
    for (ezxml_t item = ezxml_child(attr_node_drives, "item"); item; item = item->next) {
        std::string l_string;
        ezxml_t items_o = ezxml_child(item, "o");
        char * l_attr_data = ezxml_toxml(items_o);
        destringify(l_string, l_attr_data);
        free(l_attr_data);
        m_drives.push_back(l_string);

    }    ezxml_t attr_node_free_space = ezxml_child(attrs_node, "free_space");
    for (ezxml_t item = ezxml_child(attr_node_free_space, "item"); item; item = item->next) {
        int l_int;
        ezxml_t items_o = ezxml_child(item, "o");
        char * l_attr_data = ezxml_toxml(items_o);
        destringify(l_int, l_attr_data);
        free(l_attr_data);
        m_free_space.push_back(l_int);

    }    ezxml_t attr_node_working_directories = ezxml_child(attrs_node, "working_directories");
    for (ezxml_t item = ezxml_child(attr_node_working_directories, "item"); item; item = item->next) {
        std::string l_string;
        ezxml_t items_o = ezxml_child(item, "o");
        char * l_attr_data = ezxml_toxml(items_o);
        destringify(l_string, l_attr_data);
        free(l_attr_data);
        m_working_directories.push_back(l_string);

    }    ezxml_t attr_node_current__work_index = ezxml_child(attrs_node, "current__work_index");
    destringify(m_current__work_index, attr_node_current__work_index->txt);

    return 1;
}
bool Status::operator == (const Status& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_host_platform != rhs.m_host_platform) return false;
    // Checking vector
    if(m_drives.size() != rhs.m_drives.size() ) return false;
    // Checking vector
    if(m_free_space.size() != rhs.m_free_space.size() ) return false;
    // Checking vector
    if(m_working_directories.size() != rhs.m_working_directories.size() ) return false;
    if(m_current__work_index != rhs.m_current__work_index) return false;

    return true;
}
void Status::set_host_platform(const std::string& p_host_platform)
{
    m_host_platform = p_host_platform;
}
void Status::set_drives(const std::vector<std::string>& p_drives)
{
    m_drives = p_drives;
}
void Status::set_free_space(const std::vector<int>& p_free_space)
{
    m_free_space = p_free_space;
}
void Status::set_working_directories(const std::vector<std::string>& p_working_directories)
{
    m_working_directories = p_working_directories;
}
void Status::set_current__work_index(int p_current__work_index)
{
    m_current__work_index = p_current__work_index;
}
