#include "net_stts.h"
#include "messager.h"
#include "log.h"
#include "guistmch.h"
#include "net_ifce.h"
#include "dos_neti.h"
#include "prot.h"
#include "cldutils.h"

#include <string.h>
#include <stdlib.h>

extern ProtocolImpl p;

// The TCP port the cloudy peer listens on
#define CLOUDY_PORT 8966

NetState_TryConnect::~NetState_TryConnect()
{
    disconnect();
    delete netIface;
}

void NetState_TryConnect::disconnect()
{
    p.setNetworkInterface(NULL, NULL);
    if(netIface)
    {
        netIface->shutdown();
    }
}

int NetState_TryConnect::execute(void * d)
{
    if(tried)
    {
        return 1;
    }
    tried = true;

    log_info() << "Trying to connect to" << (d ? (char*)d  : "NULL");

    if(netIface == NULL)
    {
        netIface = new DosMTcpIpIface;
    }

    // the network interface
    if(netIface == NULL || netIface->setup() == false)
    {
        log_error() << "Network setup failed";
        GuiStatemachine::instance().reportError("Network setup failed, see CLOUDER.LOG");
        messager(MSG_CONNECTION_FAILED, NULL);
        return 1;
    }

    void* clientSocket = netIface->provide_socket();
    if(clientSocket == NULL)
    {
        log_error() << "Network setup failed, cannot get socket";
        GuiStatemachine::instance().reportError("Network setup failed, cannot get socket.");
        messager(MSG_CONNECTION_FAILED, NULL);
        return 1;
    }

    if(!netIface->connect(clientSocket, (char*)d, CLOUDY_PORT))
    {
        log_error() << "Cannot connect";
        messager(MSG_CONNECTION_FAILED, NULL);
        return 1;
    }

    log_info() << "Connected";
    GuiStatemachine::instance().reportError("");
    p.setNetworkInterface(netIface, clientSocket);

    ConnectRequest* cr = p.create_ConnectRequest("dos", rand_string(9));
    bool sent = p.send(cr);
    delete cr;

    if(!sent)
    {
        GuiStatemachine::instance().reportError("Cannot talk to the cloudy peer");
        disconnect();
        messager(MSG_CONNECTION_FAILED, NULL);
        return 1;
    }

    messager(MSG_CONNECTED, NULL);
    return 0;
}

int NetState_Connected::execute(void *)
{
    NetworkInterface* iface = p.networkInterface();
    if(iface == NULL || !iface->isConnected(p.socket()))
    {
        log_warning() << "The peer closed the connection";
        messager(MSG_DISCONNECTED, NULL);
        return 1;
    }

    p.poll(0);
    return 0;
}

NetState *NetStatemachine::advance(void *stdata)
{
    if(currentState == NULL)
    {
        log_critical() << "NULL state here";
        if(states.size() == 0)
        {
            log_critical() << "No states at all";
            exit(1);
        }
        setCurrentState(states.at(0));
        return currentState;
    }

    if(currentState->nextState)
    {
        log_info() << "Advancing from" << currentState->name() << "to" << currentState->nextState->name();
        currentState->nextState->setStateData(stdata);
        setCurrentState(currentState->nextState);
    }
    return currentState;
}

NetState *NetStatemachine::go_back(void *stdata)
{
    if(currentState == NULL)
    {
        if(states.size() == 0)
        {
            log_critical() << "No states at all";
            exit(1);
        }
        setCurrentState(states.at(0));
        return currentState;
    }

    if(currentState->prevState)
    {
        log_info() << "Going back from" << currentState->name() << "to" << currentState->prevState->name();
        currentState->prevState->setStateData(stdata);
        setCurrentState(currentState->prevState);
    }
    return currentState;
}
