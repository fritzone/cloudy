#include "net_stts.h"
#include "messager.h"
#include "guistate.h"
#include "ezxml.h"
#include "log.h"

#include "net_ifce.h"
#include "dos_neti.h"
#include "prot.h"

#include <string.h>
#include <stdlib.h>

static ProtocolImpl p;

NetState_TryConnect::~NetState_TryConnect()
{
    delete netIface;
    delete nsd;
}

int NetState_TryConnect::execute(void * d)
{
    log_info() << "Trying to connect d=" << (d ? (char*)d  : "NULL" )<< " stateData:" << (char*)stateData;
    if(tried)
    {
        log_info() << "Already tried, not trying again";
        return 1;
    }
    log_debug() << "Creating interface";

    tried = true;

    if(netIface == NULL)
    {
        netIface = new DosMTcpIpIface;
    }
    log_debug() << "Created " << (void*)netIface;

    if(netIface == NULL)
    {
        log_info() << "Network setup failed";
        GuiStatemachine::reportError("Network setup failed");
        messager(MSG_CONNECTION_FAILED, NULL);
        tried = false;
        return 1;
    }

    log_debug() << "Setting it up for " << (void*)netIface;
    // the network interface
    if(netIface->setup() == false)
    {
        log_info() << "Network setup failed";
        GuiStatemachine::reportError("Network setup failed");
        messager(MSG_CONNECTION_FAILED, NULL);
        tried = false;
        return 1;
    }

    log_debug() << "Interface setup complete, getting socket";
    clientSocket = netIface->provide_socket();
    if(clientSocket == NULL)
    {
        log_info() << "Network setup failed, cannot get socket";
        GuiStatemachine::reportError("Network setup failed, cannot get socket.");
        messager(MSG_CONNECTION_FAILED, NULL);
        tried = false;
        return 1;
    }

    log_debug() << "Socket acquisitioned, got socket:" <<(void*)clientSocket;

    if(!netIface->connect(clientSocket, (char*)d, 8966))
    {
        log_info() << "Cannot connect";
        messager(MSG_CONNECTION_FAILED, NULL);
        tried = false;
        return 1;
    }

    log_info() << "Connected";

    nsd = new NetStateData;
    nsd->clientSocket = clientSocket;
    nsd->iface = netIface;

    log_info() << "Advancing with" << (void*)nsd;
    messager(MSG_CONNECTED, nsd);
    log_info() << "And done";
    return 0;
}

NetState *NetStatemachine::advance(void *stdata)
{
    log_info() << "Enter, this:" << (void*)this << "currentState:" <<(void*)currentState;

    if(currentState == NULL)
    {
        log_critical() << "NULL state here";
        if(states.size() == 0)
        {
            log_critical() << "No states at all";
            exit(1);
        }
        currentState = states.at(0);
        if(currentState == NULL)
        {
            log_critical() << "Still NULL state here";
        }
        return currentState;
    }
    else
    {
        for(int i=0; i<states.size(); i++)
        {
            log_debug() << "State:" << states[i]->name() << " at " << (void*)states[i];
        }
        if(currentState && currentState->nextState)
        {
            log_info() << "Advancing from" << currentState->name() << "with"<< (void*)stdata << "to" << currentState->nextState->name();
            currentState = currentState->nextState;

            currentState->setStateData(stdata);
            log_info() << "Advancing to" << currentState->name() << "done";
        }
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
        currentState = states.at(0);
        return currentState;
    }
    else
    {
        for(int i=0; i<states.size(); i++)
        {
            log_debug() << "State:" << states[i]->name() << " at " << (void*)states[i];
        }
        if(currentState && currentState->prevState)
        {
            log_info() << "Going back from" << currentState->name() << "with"<< (void*)stdata << "to" << currentState->prevState->name();
            currentState = currentState->prevState;

            currentState->setStateData(stdata);
            log_info() << "Going back from " << currentState->name() << "done";
        }
    }
    return currentState;

}


void onDataReceived(void* object, const char* data)
{
    fprintf(stderr, "%s\n", data);
    ezxml_t doc = ezxml_parse_str((char*)data, strlen(data));
    char* s;
    s = ezxml_toxml(doc);
    fprintf(stderr, "%s\n", s);
    free(s);

    ezxml_t cld = ezxml_child(doc, "cld");
    if(cld == NULL)
    {
        fprintf(stderr, "cannot get cloud tag");
        return;
    }
    const char* cloud_version = ezxml_attr(cld, "v");
    fprintf(stderr, "cloud version: %s\n", cloud_version);

    ezxml_t msg = ezxml_child(cld, "msg");
    const char* msg_t = ezxml_attr(msg, "t");
    fprintf(stderr, "message_t: %s\n", msg_t);

    char* msg_data = ezxml_toxml(msg);

    p.receive(msg_t, msg_data);

    free(msg_data);

    ezxml_free(doc);

}

int NetState_TryPoll::execute(void * d)
{
    log_info() << "running, d is" << d << "stateData is" << stateData;
    NetStateData* nsd = (NetStateData*)d;
    nsd->iface->poll(nsd->clientSocket, 300, NULL, onDataReceived);
    return 0;
}

void cb_ConnectRecuqest(void* o, const char* data)
{
    log_info() <<"Received:" << data;
    ((NetState_ConnectRequest*)o)->received = true;
}

int NetState_ConnectRequest::execute(void *d)
{
    log_info() << "running, d is" << d << "stateData is" << stateData << "sent:" << sent << "received:" << received;

    if(sent && received)
    {
        return 0;
    }

    NetStateData* nsd = (NetStateData*)d;

    if(!sent)
    {

        ConnectRequest* cr = p.create_ConnectRequest();

        std::string scr = p.envelope(cr);
        log_info() << "Sending" << scr;

        nsd->iface->send(nsd->clientSocket, scr.c_str(), scr.length());
        sent = true;

        delete cr;

        return 0;
    }

    if(!received)
    {
        log_info() << "Polling ..." ;
        nsd->iface->poll(nsd->clientSocket, 300, this, &cb_ConnectRecuqest);
        return 0;
    }

    return 1;
}
