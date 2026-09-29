#ifndef __AUTHENTICATIONSTATUS_H__
#define __AUTHENTICATIONSTATUS_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class AuthenticationStatus: public Message
{
public:
    AuthenticationStatus() : Message(), m_success(), m_rights()
    {}

    AuthenticationStatus(bool p_success, const std::string& p_rights) : Message(), m_success(p_success), m_rights(p_rights)
    {}

    virtual ~AuthenticationStatus() {}

    virtual std::string name() const { return "AuthenticationStatus";}

    // setters
    void set_success(bool p_success);
    void set_rights(const std::string& p_rights);

    // getters
    bool get_success() const
    {
        return  m_success;
    }
    const std::string& get_rights() const
    {
        return  m_rights;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const AuthenticationStatus&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    bool m_success;
    std::string m_rights;
};
#endif
