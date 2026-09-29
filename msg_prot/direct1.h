#ifndef __DIRECTORYLISTREQUEST_H__
#define __DIRECTORYLISTREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class DirectoryListRequest: public Message
{
public:
    DirectoryListRequest() : Message(), m_directory_hash(), m_start()
    {}

    DirectoryListRequest(const std::string& p_directory_hash, int p_start) : Message(), m_directory_hash(p_directory_hash), m_start(p_start)
    {}

    virtual ~DirectoryListRequest() {}

    virtual std::string name() const { return "DirectoryListRequest";}

    // setters
    void set_directory_hash(const std::string& p_directory_hash);
    void set_start(int p_start);

    // getters
    const std::string& get_directory_hash() const
    {
        return  m_directory_hash;
    }
    int get_start() const
    {
        return  m_start;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const DirectoryListRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_directory_hash;
    int m_start;
};
#endif
