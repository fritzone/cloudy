#ifndef __DIRECTORYLIST_H__
#define __DIRECTORYLIST_H__
#include "message.h"
#include "director.h"
#include <string>
#include <vector>
class DirectoryList: public Message
{
public:
    DirectoryList(const std::string& p_directory_name = std::string(), const std::vector<DirectoryEntry>& p_entries = std::vector<DirectoryEntry>(), int p_message_id =  0) : Message(p_message_id), m_directory_name(p_directory_name), m_entries(p_entries)
    {
    }

    virtual ~DirectoryList() {}

    virtual std::string name() const { return "DirectoryList";}

    // setters
    void set_directory_name(const std::string& p_directory_name);
    void set_entries(const std::vector<DirectoryEntry>& p_entries);

    // getters
    std::string get_directory_name() const
    {
        return  m_directory_name;
    }
    std::vector<DirectoryEntry> get_entries() const
    {
        return  m_entries;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const DirectoryList&) const;

private:
    std::string m_directory_name;
    std::vector<DirectoryEntry> m_entries;
};
#endif
