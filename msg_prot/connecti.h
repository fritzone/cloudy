#ifndef __CONNECTIONREQUESTREPLY_H__
#define __CONNECTIONREQUESTREPLY_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class ConnectionRequestReply: public Message
{
public:
    ConnectionRequestReply() : Message(), m_accepted(), m_authentication_required(), m_host_name()
    {}

    ConnectionRequestReply(bool p_accepted, bool p_authentication_required, const std::string& p_host_name) : Message(), m_accepted(p_accepted), m_authentication_required(p_authentication_required), m_host_name(p_host_name)
    {}

    virtual ~ConnectionRequestReply() {}

    virtual std::string name() const { return "ConnectionRequestReply";}

    // setters
    void set_accepted(bool p_accepted);
    void set_authentication_required(bool p_authentication_required);
    void set_host_name(const std::string& p_host_name);

    // getters
    bool get_accepted() const
    {
        return  m_accepted;
    }
    bool get_authentication_required() const
    {
        return  m_authentication_required;
    }
    const std::string& get_host_name() const
    {
        return  m_host_name;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const ConnectionRequestReply&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    bool m_accepted;
    bool m_authentication_required;
    std::string m_host_name;
};
#endif
