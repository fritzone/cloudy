#ifndef __AUTHENTICATIONSTATUS_H__
#define __AUTHENTICATIONSTATUS_H__
#include "message.h"
#include <string>
class AuthenticationStatus: public Message
{
public:
    AuthenticationStatus(bool p_success =  0, const std::string& p_rights = std::string(), int p_message_id =  0) : Message(p_message_id), success(p_success), rights(p_rights)
    {}

    virtual ~AuthenticationStatus() {}

    virtual std::string get_name() const { return "AuthenticationStatus";}

    // setters
    void set_success(bool p_success);
    void set_rights(const std::string& p_rights);

    // getters
    bool get_success() const
    {
        return success;
    }
    std::string get_rights() const
    {
        return rights;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const AuthenticationStatus&) const;

private:
    bool success;
    std::string rights;
};
#endif
