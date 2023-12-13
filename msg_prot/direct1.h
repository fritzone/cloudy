#ifndef __DIRECTORYLISTREQUEST_H__
#define __DIRECTORYLISTREQUEST_H__
#include "message.h"
#include <string>
class DirectoryListRequest: public Message
{
public:
    DirectoryListRequest(const std::string& p_directory_hash = std::string(), int p_message_id =  0) : Message(p_message_id), m_directory_hash(p_directory_hash)
    {}

    virtual ~DirectoryListRequest() {}

    virtual std::string name() const { return "DirectoryListRequest";}

    // setters
    void set_directory_hash(const std::string& p_directory_hash);

    // getters
    std::string get_directory_hash() const
    {
        return  m_directory_hash;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const DirectoryListRequest&) const;

private:
    std::string m_directory_hash;
};
#endif
