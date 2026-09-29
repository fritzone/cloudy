#ifndef __FILEDATA_H__
#define __FILEDATA_H__
#include "message.h"
#include <string>
#include "ezxml.h"
class FileData: public Message
{
public:
    FileData() : Message(), m_file_hash(), m_offset(), m_data(), m_eof(), m_error()
    {}

    FileData(const std::string& p_file_hash, long p_offset, const std::string& p_data, bool p_eof, const std::string& p_error) : Message(), m_file_hash(p_file_hash), m_offset(p_offset), m_data(p_data), m_eof(p_eof), m_error(p_error)
    {}

    virtual ~FileData() {}

    virtual std::string name() const { return "FileData";}

    // setters
    void set_file_hash(const std::string& p_file_hash);
    void set_offset(long p_offset);
    void set_data(const std::string& p_data);
    void set_eof(bool p_eof);
    void set_error(const std::string& p_error);

    // getters
    const std::string& get_file_hash() const
    {
        return  m_file_hash;
    }
    long get_offset() const
    {
        return  m_offset;
    }
    const std::string& get_data() const
    {
        return  m_data;
    }
    bool get_eof() const
    {
        return  m_eof;
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
    bool operator == (const FileData&) const;

protected:
    void serialize_attributes(std::string&) const;
    int deserialize_attributes(ezxml_t);

private:
    std::string m_file_hash;
    long m_offset;
    std::string m_data;
    bool m_eof;
    std::string m_error;
};
#endif
