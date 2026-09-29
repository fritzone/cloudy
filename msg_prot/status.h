#ifndef __STATUS_H__
#define __STATUS_H__
#include "message.h"
#include <string>
#include <vector>
#include "ezxml.h"
class Status: public Message
{
public:
    Status() : Message(), m_host_platform(), m_drives(), m_free_space(), m_working_directories(), m_current__work_index()
    {}

    Status(const std::string& p_host_platform, const std::vector<std::string>& p_drives, const std::vector<long>& p_free_space, const std::vector<std::string>& p_working_directories, int p_current__work_index) : Message(), m_host_platform(p_host_platform), m_drives(p_drives), m_free_space(p_free_space), m_working_directories(p_working_directories), m_current__work_index(p_current__work_index)
    {
    }

    virtual ~Status() {}

    virtual std::string name() const { return "Status";}

    // setters
    void set_host_platform(const std::string& p_host_platform);
    void set_drives(const std::vector<std::string>& p_drives);
    void set_free_space(const std::vector<long>& p_free_space);
    void set_working_directories(const std::vector<std::string>& p_working_directories);
    void set_current__work_index(int p_current__work_index);

    // getters
    const std::string& get_host_platform() const
    {
        return  m_host_platform;
    }
    const std::vector<std::string>& get_drives() const
    {
        return  m_drives;
    }
    const std::vector<long>& get_free_space() const
    {
        return  m_free_space;
    }
    const std::vector<std::string>& get_working_directories() const
    {
        return  m_working_directories;
    }
    int get_current__work_index() const
    {
        return  m_current__work_index;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const Status&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_host_platform;
    std::vector<std::string> m_drives;
    std::vector<long> m_free_space;
    std::vector<std::string> m_working_directories;
    int m_current__work_index;
};
#endif
