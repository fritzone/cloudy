#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__
#include "message.h"
#include <string>
class Authenticate: public Message
{
public:
    Authenticate(const std::string& p_user_name_hash = std::string(), const std::string& p_password_hash = std::string(), int p_message_id =  0) : Message(p_message_id), m_user_name_hash(p_user_name_hash), m_password_hash(p_password_hash)
    {}

    virtual ~Authenticate() {}

    virtual std::string name() const { return "Authenticate";}

    // setters
    void set_user_name_hash(const std::string& p_user_name_hash);
    void set_password_hash(const std::string& p_password_hash);

    // getters
    std::string get_user_name_hash() const
    {
        return  m_user_name_hash;
    }
    std::string get_password_hash() const
    {
        return  m_password_hash;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const Authenticate&) const;

private:
    std::string m_user_name_hash;
    std::string m_password_hash;
};
#endif
