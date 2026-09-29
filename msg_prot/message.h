#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <string>
#include "ezxml.h"
class Message
{
public:
    Message() : m_message_id(++ seq_message_id)
    {}

    virtual ~Message() {}

    virtual std::string name() const { return "Message";}

    // setters

    // getters
    int get_message_id() const
    {
        return  m_message_id;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const Message&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    int m_message_id;

private:
    static int seq_message_id;
};
#endif
