#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <message.h>
#include <connectr.h>
#include <connecti.h>
#include <authenti.h>
#include <authen1.h>
#include <statusre.h>
#include <status.h>
#include <director.h>
#include <direct1.h>
#include <direc2.h>
// Protocol message handler types
typedef void(*Message_Handler)(const Message*);
typedef void(*ConnectRequest_Handler)(const ConnectRequest*);
typedef void(*ConnectionRequestReply_Handler)(const ConnectionRequestReply*);
typedef void(*Authenticate_Handler)(const Authenticate*);
typedef void(*AuthenticationStatus_Handler)(const AuthenticationStatus*);
typedef void(*StatusRequest_Handler)(const StatusRequest*);
typedef void(*Status_Handler)(const Status*);
typedef void(*DirectoryEntry_Handler)(const DirectoryEntry*);
typedef void(*DirectoryListRequest_Handler)(const DirectoryListRequest*);
typedef void(*DirectoryList_Handler)(const DirectoryList*);

class Protocol {
public:

    Protocol() : m_Message_handler(NULL), m_ConnectRequest_handler(NULL), m_ConnectionRequestReply_handler(NULL), m_Authenticate_handler(NULL), m_AuthenticationStatus_handler(NULL), m_StatusRequest_handler(NULL), m_Status_handler(NULL), m_DirectoryEntry_handler(NULL), m_DirectoryListRequest_handler(NULL), m_DirectoryList_handler(NULL)
    {}

    // message creators with parameters
    Message* create_Message();
    ConnectRequest* create_ConnectRequest(const std::string& p_platform, const std::string& p_unique_id);
    ConnectionRequestReply* create_ConnectionRequestReply(bool p_accepted, bool p_authentication_required);
    Authenticate* create_Authenticate(const std::string& p_user_name_hash, const std::string& p_password_hash);
    AuthenticationStatus* create_AuthenticationStatus(bool p_success, const std::string& p_rights);
    StatusRequest* create_StatusRequest();
    Status* create_Status(const std::string& p_host_platform, const std::vector<std::string>& p_drives, const std::vector<int>& p_free_space, const std::vector<std::string>& p_working_directories, int p_current__work_index);
    DirectoryEntry* create_DirectoryEntry(const std::string& p_name, long p_date, long p_time, long p_size, const std::string& p_attrs, const std::string& p_container, const std::string& p_hash, const std::string& p_host_id);
    DirectoryListRequest* create_DirectoryListRequest(const std::string& p_directory_hash);
    DirectoryList* create_DirectoryList(const std::string& p_directory_name, const std::vector<DirectoryEntry>& p_entries);
    // empty message creators
    ConnectRequest* create_ConnectRequest();
    ConnectionRequestReply* create_ConnectionRequestReply();
    Authenticate* create_Authenticate();
    AuthenticationStatus* create_AuthenticationStatus();
    Status* create_Status();
    DirectoryEntry* create_DirectoryEntry();
    DirectoryListRequest* create_DirectoryListRequest();
    DirectoryList* create_DirectoryList();

private:

    Message_Handler m_Message_handler;
    ConnectRequest_Handler m_ConnectRequest_handler;
    ConnectionRequestReply_Handler m_ConnectionRequestReply_handler;
    Authenticate_Handler m_Authenticate_handler;
    AuthenticationStatus_Handler m_AuthenticationStatus_handler;
    StatusRequest_Handler m_StatusRequest_handler;
    Status_Handler m_Status_handler;
    DirectoryEntry_Handler m_DirectoryEntry_handler;
    DirectoryListRequest_Handler m_DirectoryListRequest_handler;
    DirectoryList_Handler m_DirectoryList_handler;
    /**
     * Handling the Message message.
     */
    int receive_Message(const char*);

    /**
     * Handling the ConnectRequest message.
     */
    int receive_ConnectRequest(const char*);

    /**
     * Handling the ConnectionRequestReply message.
     */
    int receive_ConnectionRequestReply(const char*);

    /**
     * Handling the Authenticate message.
     */
    int receive_Authenticate(const char*);

    /**
     * Handling the AuthenticationStatus message.
     */
    int receive_AuthenticationStatus(const char*);

    /**
     * Handling the StatusRequest message.
     */
    int receive_StatusRequest(const char*);

    /**
     * Handling the Status message.
     */
    int receive_Status(const char*);

    /**
     * Handling the DirectoryEntry message.
     */
    int receive_DirectoryEntry(const char*);

    /**
     * Handling the DirectoryListRequest message.
     */
    int receive_DirectoryListRequest(const char*);

    /**
     * Handling the DirectoryList message.
     */
    int receive_DirectoryList(const char*);

public:
    /**
     * This method is called when the TCP/IP stack received something
     **/
    int receive(const char*, const char*);

};
#endif
