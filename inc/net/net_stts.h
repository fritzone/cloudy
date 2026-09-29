#ifndef _NET_STATE_H_
#define _NET_STATE_H_

#include "net_ifce.h"
#include "log.h"

#include <vector>

class NetState
{
public:
    NetState(NetState* next = NULL, NetState* prev = NULL) : stateData(NULL), nextState(next), prevState(prev)
    {
    }

    virtual ~NetState() {}

    /**
     * @brief execute is called from the main loop while this is the current state
     * @return  0 in case of success, otherwise something else, that needs to be dealt with
     */
    virtual int execute(void*) = 0;

    /**
     * Called when the state machine enters this state
     */
    virtual void onEnter() {}

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
    log_debug() << "Setting state data for " << name() << "as" << newStateData;
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
        return 0;
    }

    virtual const char* name() const {return "NoOp"; }

};

/**
 * Will try to connect to the cloud server specified in the IP, and once
 * connected sends the ConnectRequest.
 **/
class NetState_TryConnect : public NetState
{
public:
    NetState_TryConnect() : tried(false), netIface(NULL)
    {
    }
    ~NetState_TryConnect();

    virtual int execute(void*);
    virtual void onEnter() { tried = false; }

    virtual const char* name() const {return "TryConnect"; }

    /**
     * Closes the connection and shuts down the network stack
     */
    void disconnect();

private:

    bool tried;
    NetworkInterface *netIface;
};

/**
 * We are connected, pumps the network and dispatches the received messages
 **/
class NetState_Connected : public NetState
{
public:
    virtual int execute(void*);

    virtual const char* name() const {return "Connected"; }
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
        log_debug() << "CurrentState:" << s->name();
        currentState->onEnter();
    }

    void addState(NetState* s)
    {
        states.push_back(s);
    }

    NetState* advance(void* stdata);
    NetState* go_back(void* stdata);

public:

    NetState* currentState;
    std::vector<NetState*> states;
};

#endif // NET_STATE_H
