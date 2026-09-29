#include "status.h"
#include "strngify.h"

#include <string.h>
#include <stdlib.h>


std::string Status::serialize() const
{
    std::string result = "<o><type>Status</type>";
    result += "<attributes>";
    serialize_attributes(result);
    result += "</attributes></o>";
    return result;
}

void Status::serialize_attributes(std::string& result) const
{
    Message::serialize_attributes(result);
    // attribute:host_platform
    result += "<host_platform>";
    result += xml_escape(m_host_platform);
    result += "</host_platform>";
    // attribute:drives
    result += "<drives>";
    for(size_t i=0; i<m_drives.size(); i++)
    {
        result += "<item>" + xml_escape(m_drives[i]) + "</item>";
    }
    result += "</drives>";
    // attribute:free_space
    result += "<free_space>";
    for(size_t i=0; i<m_free_space.size(); i++)
    {
        result += "<item>" + stringify(m_free_space[i]) + "</item>";
    }
    result += "</free_space>";
    // attribute:working_directories
    result += "<working_directories>";
    for(size_t i=0; i<m_working_directories.size(); i++)
    {
        result += "<item>" + xml_escape(m_working_directories[i]) + "</item>";
    }
    result += "</working_directories>";
    // attribute:current__work_index
    result += "<current__work_index>";
    result += stringify(m_current__work_index);
    result += "</current__work_index>";
}

int Status::deserialize(const char* xml)
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

int Status::deserialize(ezxml_t x)
{
    if(!x) return 0;
    ezxml_t type_node = ezxml_child(x, "type");
    if(!type_node || strcmp(type_node->txt, "Status")) return 0;
    return deserialize_attributes(ezxml_child(x, "attributes"));
}

int Status::deserialize_attributes(ezxml_t attrs_node)
{
    if(!attrs_node) return 0;
    if(!Message::deserialize_attributes(attrs_node)) return 0;
    ezxml_t attr_node_host_platform = ezxml_child(attrs_node, "host_platform");
    if(attr_node_host_platform) m_host_platform = attr_node_host_platform->txt;
    ezxml_t attr_node_drives = ezxml_child(attrs_node, "drives");
    m_drives.clear();
    for(ezxml_t item = ezxml_child(attr_node_drives, "item"); item; item = item->next)
    {
        std::string l_item;
        l_item = item->txt;
        m_drives.push_back(l_item);
    }
    ezxml_t attr_node_free_space = ezxml_child(attrs_node, "free_space");
    m_free_space.clear();
    for(ezxml_t item = ezxml_child(attr_node_free_space, "item"); item; item = item->next)
    {
        long l_item;
        l_item = atol(item->txt);
        m_free_space.push_back(l_item);
    }
    ezxml_t attr_node_working_directories = ezxml_child(attrs_node, "working_directories");
    m_working_directories.clear();
    for(ezxml_t item = ezxml_child(attr_node_working_directories, "item"); item; item = item->next)
    {
        std::string l_item;
        l_item = item->txt;
        m_working_directories.push_back(l_item);
    }
    ezxml_t attr_node_current__work_index = ezxml_child(attrs_node, "current__work_index");
    if(attr_node_current__work_index) m_current__work_index = atoi(attr_node_current__work_index->txt);
    return 1;
}

bool Status::operator == (const Status& rhs) const
{
    if(!Message::operator ==(rhs)) return false;
    if(m_host_platform != rhs.m_host_platform) return false;
    if(m_drives.size() != rhs.m_drives.size() ) return false;
    for(size_t i_drives = 0; i_drives < m_drives.size(); i_drives++)
        if(!(m_drives[i_drives] == rhs.m_drives[i_drives])) return false;
    if(m_free_space.size() != rhs.m_free_space.size() ) return false;
    for(size_t i_free_space = 0; i_free_space < m_free_space.size(); i_free_space++)
        if(!(m_free_space[i_free_space] == rhs.m_free_space[i_free_space])) return false;
    if(m_working_directories.size() != rhs.m_working_directories.size() ) return false;
    for(size_t i_working_directories = 0; i_working_directories < m_working_directories.size(); i_working_directories++)
        if(!(m_working_directories[i_working_directories] == rhs.m_working_directories[i_working_directories])) return false;
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
void Status::set_free_space(const std::vector<long>& p_free_space)
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
