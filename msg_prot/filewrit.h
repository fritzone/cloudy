#ifndef __FILEWRITEREQUEST_H__
#define __FILEWRITEREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class FileWriteRequest: public Message
{
public:
    FileWriteRequest() : Message(), m_directory_hash(), m_name(), m_offset(), m_data(), m_last()
    {}

    FileWriteRequest(const std::string& p_directory_hash, const std::string& p_name, long p_offset, const std::string& p_data, bool p_last) : Message(), m_directory_hash(p_directory_hash), m_name(p_name), m_offset(p_offset), m_data(p_data), m_last(p_last)
    {}

    virtual ~FileWriteRequest() {}

    virtual std::string name() const { return "FileWriteRequest";}

    // setters
    void set_directory_hash(const std::string& p_directory_hash);
    void set_name(const std::string& p_name);
    void set_offset(long p_offset);
    void set_data(const std::string& p_data);
    void set_last(bool p_last);

    // getters
    const std::string& get_directory_hash() const
    {
        return  m_directory_hash;
    }
    const std::string& get_name() const
    {
        return  m_name;
    }
    long get_offset() const
    {
        return  m_offset;
    }
    const std::string& get_data() const
    {
        return  m_data;
    }
    bool get_last() const
    {
        return  m_last;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const FileWriteRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_directory_hash;
    std::string m_name;
    long m_offset;
    std::string m_data;
    bool m_last;
};
#endif
