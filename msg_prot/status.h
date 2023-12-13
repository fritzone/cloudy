#ifndef __STATUS_H__
#define __STATUS_H__
#include "message.h"
#include <string>
#include <vector>
class Status: public Message
{
public:
    Status(const std::string& p_host_platform = std::string(), const std::vector<std::string>& p_drives = std::vector<std::string>(), const std::vector<int>& p_free_space = std::vector<int>(), const std::vector<std::string>& p_working_directories = std::vector<std::string>(), int p_current__work_index =  0, int p_message_id =  0) : Message(p_message_id), m_host_platform(p_host_platform), m_drives(p_drives), m_free_space(p_free_space), m_working_directories(p_working_directories), m_current__work_index(p_current__work_index)
    {
    }

    virtual ~Status() {}

    virtual std::string name() const { return "Status";}

    // setters
    void set_host_platform(const std::string& p_host_platform);
    void set_drives(const std::vector<std::string>& p_drives);
    void set_free_space(const std::vector<int>& p_free_space);
    void set_working_directories(const std::vector<std::string>& p_working_directories);
    void set_current__work_index(int p_current__work_index);

    // getters
    std::string get_host_platform() const
    {
        return  m_host_platform;
    }
    std::vector<std::string> get_drives() const
    {
        return  m_drives;
    }
    std::vector<int> get_free_space() const
    {
        return  m_free_space;
    }
    std::vector<std::string> get_working_directories() const
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

    // comparison
    bool operator == (const Status&) const;

private:
    std::string m_host_platform;
    std::vector<std::string> m_drives;
    std::vector<int> m_free_space;
    std::vector<std::string> m_working_directories;
    int m_current__work_index;
};
#endif
