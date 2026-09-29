#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class Authenticate: public Message
{
public:
    Authenticate() : Message(), m_user_name_hash(), m_password_hash()
    {}

    Authenticate(const std::string& p_user_name_hash, const std::string& p_password_hash) : Message(), m_user_name_hash(p_user_name_hash), m_password_hash(p_password_hash)
    {}

    virtual ~Authenticate() {}

    virtual std::string name() const { return "Authenticate";}

    // setters
    void set_user_name_hash(const std::string& p_user_name_hash);
    void set_password_hash(const std::string& p_password_hash);

    // getters
    const std::string& get_user_name_hash() const
    {
        return  m_user_name_hash;
    }
    const std::string& get_password_hash() const
    {
        return  m_password_hash;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const Authenticate&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_user_name_hash;
    std::string m_password_hash;
};
#endif
