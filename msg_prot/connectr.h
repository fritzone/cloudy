#ifndef __CONNECTREQUEST_H__
#define __CONNECTREQUEST_H__
#include "message.h"
#include <string>
class ConnectRequest: public Message
{
public:
    ConnectRequest(const std::string& p_platform = std::string(), const std::string& p_unique_id = std::string(), int p_message_id =  0) : Message(p_message_id), m_platform(p_platform), m_unique_id(p_unique_id)
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
    std::string get_platform() const
    {
        return  m_platform;
    }
    std::string get_unique_id() const
    {
        return  m_unique_id;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const ConnectRequest&) const;

private:
    std::string m_platform;
    std::string m_unique_id;
};
#endif
