#ifndef __STATUSREQUEST_H__
#define __STATUSREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class StatusRequest: public Message
{
public:
    StatusRequest() : Message()
    {}

    virtual ~StatusRequest() {}

    virtual std::string name() const { return "StatusRequest";}

    // setters

    // getters

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const StatusRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
};
#endif
