#ifndef __WELCOME_H__
#define __WELCOME_H__
#include "message.h"
#include <string>
class Welcome: public Message
{
public:
    Welcome(bool p_accepted =  0, bool p_authentication_required =  0, int p_message_id =  0) : Message(p_message_id), accepted(p_accepted), authentication_required(p_authentication_required)
    {}

    virtual ~Welcome() {}

    virtual std::string get_name() const { return "Welcome";}

    // setters
    void set_accepted(bool p_accepted);
    void set_authentication_required(bool p_authentication_required);

    // getters
    bool get_accepted() const
    {
        return accepted;
    }
    bool get_authentication_required() const
    {
        return authentication_required;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const Welcome&) const;

private:
    bool accepted;
    bool authentication_required;
};
#endif
