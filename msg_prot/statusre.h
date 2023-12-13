#ifndef __STATUSREQUEST_H__
#define __STATUSREQUEST_H__
#include "message.h"
#include <string>
class StatusRequest: public Message
{
public:
    StatusRequest(int p_message_id =  0) : Message(p_message_id)
    {}

    virtual ~StatusRequest() {}

    virtual std::string name() const { return "StatusRequest";}

    // setters

    // getters

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);

    // comparison
    bool operator == (const StatusRequest&) const;

private:
};
#endif
