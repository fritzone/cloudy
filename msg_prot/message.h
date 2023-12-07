#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <string>
class Message
{
public:
    Message(int p_message_id =  0) : message_id(p_message_id)
    {}

    virtual ~Message() {}

    virtual std::string get_name() const { return "Message";}

    // setters
    void set_message_id(int p_message_id);

    // getters
    int get_message_id() const
    {
        return message_id;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const Message&) const;

private:
    int message_id;
};
#endif
