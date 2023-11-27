#ifndef _NET_STATE_H_
#define _NET_STATE_H_

#include "dos_neti.h"

#include "log.h"

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

    void* stateData;
    void *getStateData() const;
    void setStateData(void *newStateData);

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
};

class NetState_TryConnect : public NetState
{
public:
    NetState_TryConnect()
    {
        tried = false;
    }
    virtual int execute(void*)
    {
        if(tried)
        {
            return 1;
        }

        tried = true;

        // the network interface
        DosMTcpIpIface netIface;
        if(!netIface.setup())
        {
            GuiStatemachine::reportError("Network setup failed");
            return 1;
        }

        void* clientSocket = netIface.provide_socket();
        if(!netIface.connect(clientSocket, (char*)stateData))
        {
            netIface.shutdown();
            return 1;
        }

        return 0;
    }

private:

    bool tried;
};




class NetStatemachine
{
public:
    static NetStatemachine instance()
    {
        static NetStatemachine i;
        return i;
    }

    NetState *getCurrentState() const
    {
        return currentState;
    }

    void addState(NetState* s)
    {
        states.push_back(s);
    }

    NetState* advance(void* stdata)
    {
        if(currentState->nextState)
        {
            currentState = currentState->nextState;
            currentState->setStateData(stdata);
        }
        return currentState;
    }

public:

    NetState* currentState;
    std::vector<NetState*> states;

};


#endif // NET_STATE_H
