#include "protocol.h"
#include <ezxml.h>
#include <string.h>


int Protocol::receive_Message(const char* p_xml)
{
    if(!m_Message_handler) return 0;
    Message obj;
    obj.deserialize(p_xml);
    m_Message_handler(&obj);
    return 1;
}

int Protocol::receive_ConnectRequest(const char* p_xml)
{
    if(!m_ConnectRequest_handler) return 0;
    ConnectRequest obj;
    obj.deserialize(p_xml);
    m_ConnectRequest_handler(&obj);
    return 1;
}

int Protocol::receive_ConnectionRequestReply(const char* p_xml)
{
    if(!m_ConnectionRequestReply_handler) return 0;
    ConnectionRequestReply obj;
    obj.deserialize(p_xml);
    m_ConnectionRequestReply_handler(&obj);
    return 1;
}

int Protocol::receive_Authenticate(const char* p_xml)
{
    if(!m_Authenticate_handler) return 0;
    Authenticate obj;
    obj.deserialize(p_xml);
    m_Authenticate_handler(&obj);
    return 1;
}

int Protocol::receive_AuthenticationStatus(const char* p_xml)
{
    if(!m_AuthenticationStatus_handler) return 0;
    AuthenticationStatus obj;
    obj.deserialize(p_xml);
    m_AuthenticationStatus_handler(&obj);
    return 1;
}

int Protocol::receive_StatusRequest(const char* p_xml)
{
    if(!m_StatusRequest_handler) return 0;
    StatusRequest obj;
    obj.deserialize(p_xml);
    m_StatusRequest_handler(&obj);
    return 1;
}

int Protocol::receive_Status(const char* p_xml)
{
    if(!m_Status_handler) return 0;
    Status obj;
    obj.deserialize(p_xml);
    m_Status_handler(&obj);
    return 1;
}

int Protocol::receive_DirectoryEntry(const char* p_xml)
{
    if(!m_DirectoryEntry_handler) return 0;
    DirectoryEntry obj;
    obj.deserialize(p_xml);
    m_DirectoryEntry_handler(&obj);
    return 1;
}

int Protocol::receive_DirectoryListRequest(const char* p_xml)
{
    if(!m_DirectoryListRequest_handler) return 0;
    DirectoryListRequest obj;
    obj.deserialize(p_xml);
    m_DirectoryListRequest_handler(&obj);
    return 1;
}

int Protocol::receive_DirectoryList(const char* p_xml)
{
    if(!m_DirectoryList_handler) return 0;
    DirectoryList obj;
    obj.deserialize(p_xml);
    m_DirectoryList_handler(&obj);
    return 1;
}

int Protocol::receive(const char* p_message_type, const char* p_serialized_xml)
{
    if(!strcmp(p_message_type, "Message")) { return receive_Message(p_serialized_xml); }
    if(!strcmp(p_message_type, "ConnectRequest")) { return receive_ConnectRequest(p_serialized_xml); }
    if(!strcmp(p_message_type, "ConnectionRequestReply")) { return receive_ConnectionRequestReply(p_serialized_xml); }
    if(!strcmp(p_message_type, "Authenticate")) { return receive_Authenticate(p_serialized_xml); }
    if(!strcmp(p_message_type, "AuthenticationStatus")) { return receive_AuthenticationStatus(p_serialized_xml); }
    if(!strcmp(p_message_type, "StatusRequest")) { return receive_StatusRequest(p_serialized_xml); }
    if(!strcmp(p_message_type, "Status")) { return receive_Status(p_serialized_xml); }
    if(!strcmp(p_message_type, "DirectoryEntry")) { return receive_DirectoryEntry(p_serialized_xml); }
    if(!strcmp(p_message_type, "DirectoryListRequest")) { return receive_DirectoryListRequest(p_serialized_xml); }
    if(!strcmp(p_message_type, "DirectoryList")) { return receive_DirectoryList(p_serialized_xml); }
    return 1;
}

// Default Creators
ConnectRequest* Protocol::create_ConnectRequest()
{
    return new ConnectRequest();
}

ConnectionRequestReply* Protocol::create_ConnectionRequestReply()
{
    return new ConnectionRequestReply();
}

Authenticate* Protocol::create_Authenticate()
{
    return new Authenticate();
}

AuthenticationStatus* Protocol::create_AuthenticationStatus()
{
    return new AuthenticationStatus();
}

Status* Protocol::create_Status()
{
    return new Status();
}

DirectoryEntry* Protocol::create_DirectoryEntry()
{
    return new DirectoryEntry();
}

DirectoryListRequest* Protocol::create_DirectoryListRequest()
{
    return new DirectoryListRequest();
}

DirectoryList* Protocol::create_DirectoryList()
{
    return new DirectoryList();
}

// Creators with specific parameters
Message*  Protocol::create_Message()
{
    Message* l_Message = new Message();
    return  l_Message;

}
ConnectRequest*  Protocol::create_ConnectRequest(const std::string& p_platform, const std::string& p_unique_id)
{
    ConnectRequest* l_ConnectRequest = new ConnectRequest();
    return  l_ConnectRequest;

}
ConnectionRequestReply*  Protocol::create_ConnectionRequestReply(bool p_accepted, bool p_authentication_required)
{
    ConnectionRequestReply* l_ConnectionRequestReply = new ConnectionRequestReply();
    return  l_ConnectionRequestReply;

}
Authenticate*  Protocol::create_Authenticate(const std::string& p_user_name_hash, const std::string& p_password_hash)
{
    Authenticate* l_Authenticate = new Authenticate();
    return  l_Authenticate;

}
AuthenticationStatus*  Protocol::create_AuthenticationStatus(bool p_success, const std::string& p_rights)
{
    AuthenticationStatus* l_AuthenticationStatus = new AuthenticationStatus();
    return  l_AuthenticationStatus;

}
StatusRequest*  Protocol::create_StatusRequest()
{
    return new StatusRequest();
}

Status*  Protocol::create_Status(const std::string& p_host_platform, const std::vector<std::string>& p_drives, const std::vector<int>& p_free_space, const std::vector<std::string>& p_working_directories, int p_current__work_index)
{
    Status* l_Status = new Status();
    return  l_Status;

}
DirectoryEntry*  Protocol::create_DirectoryEntry(const std::string& p_name, long p_date, long p_time, long p_size, const std::string& p_attrs, const std::string& p_container, const std::string& p_hash, const std::string& p_host_id)
{
    DirectoryEntry* l_DirectoryEntry = new DirectoryEntry();
    return  l_DirectoryEntry;

}
DirectoryListRequest*  Protocol::create_DirectoryListRequest(const std::string& p_directory_hash)
{
    DirectoryListRequest* l_DirectoryListRequest = new DirectoryListRequest();
    return  l_DirectoryListRequest;

}
DirectoryList*  Protocol::create_DirectoryList(const std::string& p_directory_name, const std::vector<DirectoryEntry>& p_entries)
{
    DirectoryList* l_DirectoryList = new DirectoryList();
    return  l_DirectoryList;

}
