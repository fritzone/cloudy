#ifndef __FILEWRITEREPLY_H__
#define __FILEWRITEREPLY_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class FileWriteReply: public Message
{
public:
    FileWriteReply() : Message(), m_name(), m_offset(), m_ok(), m_error()
    {}

    FileWriteReply(const std::string& p_name, long p_offset, bool p_ok, const std::string& p_error) : Message(), m_name(p_name), m_offset(p_offset), m_ok(p_ok), m_error(p_error)
    {}

    virtual ~FileWriteReply() {}

    virtual std::string name() const { return "FileWriteReply";}

    // setters
    void set_name(const std::string& p_name);
    void set_offset(long p_offset);
    void set_ok(bool p_ok);
    void set_error(const std::string& p_error);

    // getters
    const std::string& get_name() const
    {
        return  m_name;
    }
    long get_offset() const
    {
        return  m_offset;
    }
    bool get_ok() const
    {
        return  m_ok;
    }
    const std::string& get_error() const
    {
        return  m_error;
    }

    // serializer
    virtual std::string serialize() const;
    virtual int deserialize(const char*);
    virtual int deserialize(ezxml_t);

    // comparison
    bool operator == (const FileWriteReply&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_name;
    long m_offset;
    bool m_ok;
    std::string m_error;
};
#endif
