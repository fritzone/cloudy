#ifndef __DIRECTORYENTRY_H__
#define __DIRECTORYENTRY_H__
#include <string>
#include "ezxml.h"
class DirectoryEntry
{
public:
    DirectoryEntry() : m_name(), m_date(), m_time(), m_size(), m_attrs(), m_hash()
    {}

    DirectoryEntry(const std::string& p_name, long p_date, long p_time, long p_size, const std::string& p_attrs, const std::string& p_hash) : m_name(p_name), m_date(p_date), m_time(p_time), m_size(p_size), m_attrs(p_attrs), m_hash(p_hash)
    {}

    virtual ~DirectoryEntry() {}

    virtual std::string name() const { return "DirectoryEntry";}

    // setters
    void set_name(const std::string& p_name);
    void set_date(long p_date);
    void set_time(long p_time);
    void set_size(long p_size);
    void set_attrs(const std::string& p_attrs);
    void set_hash(const std::string& p_hash);

    // getters
    const std::string& get_name() const
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
    const std::string& get_attrs() const
    {
        return  m_attrs;
    }
    const std::string& get_hash() const
    {
        return  m_hash;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const DirectoryEntry&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_name;
    long m_date;
    long m_time;
    long m_size;
    std::string m_attrs;
    std::string m_hash;
};
#endif
