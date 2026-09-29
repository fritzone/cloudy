#ifndef __CONNECTREQUEST_H__
#define __CONNECTREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class ConnectRequest: public Message
{
public:
    ConnectRequest() : Message(), m_platform(), m_unique_id()
    {}

    ConnectRequest(const std::string& p_platform, const std::string& p_unique_id) : Message(), m_platform(p_platform), m_unique_id(p_unique_id)
    {
        if(m_platform != "dos" && m_platform != "linux")
        {
            m_platform = std::string();
        }
    }

    virtual ~ConnectRequest() {}

    virtual std::string name() const { return "ConnectRequest";}

    // setters
    void set_platform(const std::string& p_platform);
    void set_unique_id(const std::string& p_unique_id);

    // getters
    const std::string& get_platform() const
    {
        return  m_platform;
    }
    const std::string& get_unique_id() const
    {
        return  m_unique_id;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const ConnectRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_platform;
    std::string m_unique_id;
};
#endif
