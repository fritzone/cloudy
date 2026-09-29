#ifndef __FILEREADREQUEST_H__
#define __FILEREADREQUEST_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class FileReadRequest: public Message
{
public:
    FileReadRequest() : Message(), m_file_hash(), m_offset(), m_length()
    {}

    FileReadRequest(const std::string& p_file_hash, long p_offset, int p_length) : Message(), m_file_hash(p_file_hash), m_offset(p_offset), m_length(p_length)
    {}

    virtual ~FileReadRequest() {}

    virtual std::string name() const { return "FileReadRequest";}

    // setters
    void set_file_hash(const std::string& p_file_hash);
    void set_offset(long p_offset);
    void set_length(int p_length);

    // getters
    const std::string& get_file_hash() const
    {
        return  m_file_hash;
    }
    long get_offset() const
    {
        return  m_offset;
    }
    int get_length() const
    {
        return  m_length;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const FileReadRequest&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_file_hash;
    long m_offset;
    int m_length;
};
#endif
