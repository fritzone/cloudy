#ifndef __DIRECTORYLIST_H__
#define __DIRECTORYLIST_H__
#include "message.h"
#include "director.h"
#include <string>
#include <vector>
#include "ezxml.h"
class DirectoryList: public Message
{
public:
    DirectoryList() : Message(), m_directory_name(), m_directory_hash(), m_start(), m_total(), m_error(), m_entries()
    {}

    DirectoryList(const std::string& p_directory_name, const std::string& p_directory_hash, int p_start, int p_total, const std::string& p_error, const std::vector<DirectoryEntry>& p_entries) : Message(), m_directory_name(p_directory_name), m_directory_hash(p_directory_hash), m_start(p_start), m_total(p_total), m_error(p_error), m_entries(p_entries)
    {
    }

    virtual ~DirectoryList() {}

    virtual std::string name() const { return "DirectoryList";}

    // setters
    void set_directory_name(const std::string& p_directory_name);
    void set_directory_hash(const std::string& p_directory_hash);
    void set_start(int p_start);
    void set_total(int p_total);
    void set_error(const std::string& p_error);
    void set_entries(const std::vector<DirectoryEntry>& p_entries);

    // getters
    const std::string& get_directory_name() const
    {
        return  m_directory_name;
    }
    const std::string& get_directory_hash() const
    {
        return  m_directory_hash;
    }
    int get_start() const
    {
        return  m_start;
    }
    int get_total() const
    {
        return  m_total;
    }
    const std::string& get_error() const
    {
        return  m_error;
    }
    const std::vector<DirectoryEntry>& get_entries() const
    {
        return  m_entries;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const DirectoryList&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_directory_name;
    std::string m_directory_hash;
    int m_start;
    int m_total;
    std::string m_error;
    std::vector<DirectoryEntry> m_entries;
};
#endif
