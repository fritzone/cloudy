#ifndef _NET_STATE_H_
#define _NET_STATE_H_

#include "net_ifce.h"
#include "log.h"

#include <vector>

struct NetStateData
{
    NetworkInterface* iface;
    void* clientSocket;
};

class NetState
{
public:
    NetState(NetState* next = NULL, NetState* prev = NULL) : nextState(next), prevState(prev)
    {
    }

    /**
     * @brief execute
     * @return  0 in case of success, otherwise something else, that needs to be dealt with
     */
    virtual int execute(void*) = 0;

    virtual const char* name() const = 0;

    void* stateData;
    void *getStateData() const;
    void setStateData(void *newStateData);

    void setNext(NetState* s) {nextState = s;}
    void setPrev(NetState* s) {prevState = s;}

public:
    NetState* nextState;
    NetState* prevState;
};

inline void *NetState::getStateData() const
{
    return stateData;
}

inline void NetState::setStateData(void *newStateData)
{
    log_info() << "Setting state data for " << name() << "as" << newStateData;
    stateData = newStateData;
}

/**
 * @brief The NetState_NoOp class the no operation state, which does nothing
 */
class NetState_NoOp : public NetState
{
public:
    virtual int execute(void*)
    {
        log_info() << "execute";
        return 0;
    }

    virtual const char* name() const {return "NoOp"; }

};

/**
 * Will try to connect to the cloud server specified in the IP
 **/
class NetState_TryConnect : public NetState
{
public:
    NetState_TryConnect() : tried(false), netIface(NULL), clientSocket(NULL), nsd(NULL)
    {
        tried = false;
    }
    ~NetState_TryConnect();

    virtual int execute(void*);

    virtual const char* name() const {return "TryConnect"; }

private:

    bool tried;
    NetworkInterface *netIface;
    void* clientSocket;
    NetStateData* nsd;
};

/**
 * Polls the server to see if there is anything
 **/
class NetState_TryPoll : public NetState
{
public:
    NetState_TryPoll()
    {
    }

    virtual int execute(void*);

    virtual const char* name() const {return "TryPoll"; }


};

class NetState_ConnectRequest : public NetState
{
public:
    NetState_ConnectRequest() : sent(false), received(false)
    {
    }

    virtual int execute(void*);

    virtual const char* name() const {return "ConnectRequest"; }
    friend void cb_ConnectRecuqest(void* o, const char* data);

private:

    bool sent;
    bool received;
};

/**
 * The container of the network states
 **/
class NetStatemachine
{
public:

    NetStatemachine():currentState(NULL)
    {}

    static NetStatemachine& instance()
    {
        static NetStatemachine i;
        return i;
    }

    NetState *getCurrentState() const
    {
        return currentState;
    }

    void setCurrentState(NetState* s)
    {
        currentState = s;

        log_debug() << "CurrentState:" << s->name() << " at " << (void*)s;

        for(int i=0; i<states.size(); i++)
        {
            log_debug() << "State:" << states[i]->name() << " at " << (void*)states[i];
        }

    }

    void addState(NetState* s)
    {
        states.push_back(s);
    }

    NetState* advance(void* stdata);
    NetState* go_back(void* stdata);

    void onNext(NetState* s, int(*cb)(void*))
    {
        log_info() << "Set callback for" << s->name() << " as " << (void*)cb;
        stateNextOps[s] = cb;
    }

public:

    NetState* currentState;
    std::vector<NetState*> states;
    std::map<NetState*, int(*)(void*)> stateNextOps;

};

#endif // NET_STATE_H
