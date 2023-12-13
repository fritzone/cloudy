#ifndef __CONNECTIONREQUESTREPLY_H__
#define __CONNECTIONREQUESTREPLY_H__
#include "message.h"
#include <string>
class ConnectionRequestReply: public Message
{
public:
    ConnectionRequestReply(bool p_accepted =  0, bool p_authentication_required =  0, int p_message_id =  0) : Message(p_message_id), m_accepted(p_accepted), m_authentication_required(p_authentication_required)
    {}

    virtual ~ConnectionRequestReply() {}

    virtual std::string name() const { return "ConnectionRequestReply";}

    // setters
    void set_accepted(bool p_accepted);
    void set_authentication_required(bool p_authentication_required);

    // getters
    bool get_accepted() const
    {
        return  m_accepted;
    }
    bool get_authentication_required() const
    {
        return  m_authentication_required;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const ConnectionRequestReply&) const;

private:
    bool m_accepted;
    bool m_authentication_required;
};
#endif
