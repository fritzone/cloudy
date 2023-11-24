#ifndef _NET_STATE_H_
#define _NET_STATE_H_

#include "dos_neti.h"

class NetState
{
public:
    /**
     * @brief execute
     * @return  0 in case of success, otherwise something else, that needs to be dealt with
     */
    virtual int execute() = 0;

    void* stateData;
};

/**
 * @brief The NetState_NoOp class the no operation state, which does nothing
 */
class NetState_NoOp : public NetState
{
public:
    virtual int execute()
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
    virtual int execute()
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
            fprintf(stderr, "Network setup failed");
            exit(1);
            return 1;
        }

        void* clientSocket = netIface.provide_socket();
        if(!netIface.connect(clientSocket, (char*)stateData))
        {
            netIface.shutdown();
            exit(1);
            return 1;
        }

        return 0;
    }

private:

    bool tried;
};

#endif // NET_STATE_H
