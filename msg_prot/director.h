#ifndef __DIRECTORYENTRY_H__
#define __DIRECTORYENTRY_H__
#include "message.h"
#include <string>
class DirectoryEntry: public Message
{
public:
    DirectoryEntry(const std::string& p_name = std::string(), long p_date =  0, long p_time =  0, long p_size =  0, const std::string& p_attrs = std::string(), const std::string& p_container = std::string(), const std::string& p_hash = std::string(), const std::string& p_host_id = std::string(), int p_message_id =  0) : Message(p_message_id), m_name(p_name), m_date(p_date), m_time(p_time), m_size(p_size), m_attrs(p_attrs), m_container(p_container), m_hash(p_hash), m_host_id(p_host_id)
    {}

    virtual ~DirectoryEntry() {}

    virtual std::string name() const { return "DirectoryEntry";}

    // setters
    void set_name(const std::string& p_name);
    void set_date(long p_date);
    void set_time(long p_time);
    void set_size(long p_size);
    void set_attrs(const std::string& p_attrs);
    void set_container(const std::string& p_container);
    void set_hash(const std::string& p_hash);
    void set_host_id(const std::string& p_host_id);

    // getters
    std::string get_name() const
    {
        return  m_name;
    }
    long get_date() const
    {
        return  m_date;
    }
    long get_time() const
    {
        return  m_time;
    }
    long get_size() const
    {
        return  m_size;
    }
    std::string get_attrs() const
    {
        return  m_attrs;
    }
    std::string get_container() const
    {
        return  m_container;
    }
    std::string get_hash() const
    {
        return  m_hash;
    }
    std::string get_host_id() const
    {
        return  m_host_id;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const DirectoryEntry&) const;

private:
    std::string m_name;
    long m_date;
    long m_time;
    long m_size;
    std::string m_attrs;
    std::string m_container;
    std::string m_hash;
    std::string m_host_id;
};
#endif
