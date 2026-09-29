#ifndef __MAKEDIRECTORYREQUEST_H__
#define __MAKEDIRECTORYREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class MakeDirectoryRequest: public Message
{
public:
    MakeDirectoryRequest() : Message(), m_parent_hash(), m_name()
    {}

    MakeDirectoryRequest(const std::string& p_parent_hash, const std::string& p_name) : Message(), m_parent_hash(p_parent_hash), m_name(p_name)
    {}

    virtual ~MakeDirectoryRequest() {}

    virtual std::string name() const { return "MakeDirectoryRequest";}

    // setters
    void set_parent_hash(const std::string& p_parent_hash);
    void set_name(const std::string& p_name);

    // getters
    const std::string& get_parent_hash() const
    {
        return  m_parent_hash;
    }
    const std::string& get_name() const
    {
        return  m_name;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const MakeDirectoryRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_parent_hash;
    std::string m_name;
};
#endif
