#ifndef __MAKEDIRECTORYREPLY_H__
#define __MAKEDIRECTORYREPLY_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class MakeDirectoryReply: public Message
{
public:
    MakeDirectoryReply() : Message(), m_name(), m_hash(), m_ok(), m_error()
    {}

    MakeDirectoryReply(const std::string& p_name, const std::string& p_hash, bool p_ok, const std::string& p_error) : Message(), m_name(p_name), m_hash(p_hash), m_ok(p_ok), m_error(p_error)
    {}

    virtual ~MakeDirectoryReply() {}

    virtual std::string name() const { return "MakeDirectoryReply";}

    // setters
    void set_name(const std::string& p_name);
    void set_hash(const std::string& p_hash);
    void set_ok(bool p_ok);
    void set_error(const std::string& p_error);

    // getters
    const std::string& get_name() const
    {
        return  m_name;
    }
    const std::string& get_hash() const
    {
        return  m_hash;
    }
    bool get_ok() const
    {
        return  m_ok;
    }
    const std::string& get_error() const
    {
        return  m_error;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const MakeDirectoryReply&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_name;
    std::string m_hash;
    bool m_ok;
    std::string m_error;
};
#endif
