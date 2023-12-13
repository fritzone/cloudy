#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <string>
class Message
{
public:
    Message(int p_message_id =  0) : m_message_id(++ seq_message_id)
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

    // comparison
    bool operator == (const Message&) const;

private:
    int m_message_id;

private:
    static int seq_message_id;
};
#endif
