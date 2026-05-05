#ifndef __MUDUO_BASE_LOGSTREAM_H__
#define __MUDUO_BASE_LOGSTREAM_H__

#include "muduo/base/noncopyable.hpp"
#include "muduo/base/Types.hpp"
#include "muduo/base/StringPiech.hpp"
#include "muduo/base/Util.hpp"

#include <string.h>
#include <assert.h>

#include <string>
#include <stdexcept>


namespace muduo::base
{

namespace detail
{
constexpr int kSmallBuffer = 4000;
constexpr int kLargeBuffer = kSmallBuffer * 1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer()
        : m_cur(m_data)
    {
        setCookie(cookieStart);
    }

    ~FixedBuffer()
    {
        setCookie(cookieEnd);
    }

    void append(const char* buf, size_t len)
    {
        if (implicit_cast<size_t>(avail()) > len)
        {
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }

    const char* data() const  { return m_data; }
    int length() const { return static_cast<int>(m_cur - m_data); }


    char* current() const { return m_cur; }
    int avail() const { return static_cast<int>(end() - m_cur);}
    void add(size_t len) 
    { 
        if (static_cast<size_t>(avail()) > len)
        {
            m_cur += len; /* maybe dump */
        }
        else
        {
            throw std::logic_error("len great");
        }
    }

    void reset() { m_cur = m_data; }
    void bzero() { memZero(m_data, sizeof(m_data)); }

    const char* debugString();

    void setCookie(void(*cookie)()) { m_cookie = cookie; }

    std::string toString() const { return std::string(m_data, length()); }
    StringPiece toStringPiech() const { return StringPiece(m_data, length()); }

private:

    const char* end() const { return m_data + sizeof(m_data); }

    static void cookieStart();
    static void cookieEnd();

    void (*m_cookie)();
    char m_data[SIZE];
    char* m_cur;
};

template <int SIZE>
inline const char *FixedBuffer<SIZE>::debugString()
{
    *m_cur = '\0';
    return m_data;
}

template <int SIZE>
inline void FixedBuffer<SIZE>::cookieStart()
{
}

template <int SIZE>
inline void FixedBuffer<SIZE>::cookieEnd()
{
}

template<std::integral T>
size_t convert(char buf[], T value);

size_t convertHex(char buf[], uintptr_t value);

}

class LogStream : noncopyable
{
    using self = LogStream;
public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

    LogStream& operator<<(bool v)
    {
        m_buffer.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(int v);
    LogStream& operator<<(unsigned int v);

    LogStream& operator<< (short v);
    LogStream& operator<< (unsigned short v);

    LogStream& operator<< (long v);
    LogStream& operator<< (unsigned long v);
    LogStream& operator<< (unsigned long long v);

    LogStream& operator<< (const void* p);


    LogStream& operator<< (float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }

    LogStream& operator<< (double v);

    LogStream& operator<<(char v)
    {
        m_buffer.append(&v, 1);
        return *this;
    }

    LogStream& operator << (const char* str)
    {
        if (str)
        {
            m_buffer.append(str, strlen(str));
        }
        else
        {
            m_buffer.append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<< (const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream&  operator<<(const std::string& v)
    {
        m_buffer.append(v.c_str(), v.size());
        return *this;
    }

    LogStream& operator<<(std::string_view v)
    {
        m_buffer.append(v.data(), v.size());
        return *this;
    }


    LogStream& operator<<(const StringPiece& v)
    {
        m_buffer.append(v.data(), v.size());
        return *this;
    }

    LogStream& operator<<(const Buffer& v)
    {
        *this << v.toStringPiech();

        return *this;
    }
    

    void append(const char* data, int length)
    {
        m_buffer.append(data, length);
    }

    const Buffer& buffer() const { return m_buffer; }

    void resetBuffer() { m_buffer.reset(); }

private:

    void staticCheck();

    template<std::integral T>
    void formatInteger(T value);

    Buffer m_buffer;
    static constexpr int kMaxNumercSize = 48;
};

template <std::integral T>
inline void LogStream::formatInteger(T value)
{
    if (m_buffer.avail() >= kMaxNumercSize)
    {
        size_t len = detail::convert(m_buffer.current(), value);
        m_buffer.add(len);
    }

}




class Fmt
{
public:
    template<ArithmeticType T>
    Fmt(const char* fmt, T val);

    const char* data() const { return m_buf; }
    int length() const { return m_length; }

private:

    char m_buf[32];
    int m_length;
};

template <ArithmeticType T>
inline Fmt::Fmt(const char *fmt, T val)
{
    m_length = snprintf(m_buf, sizeof(m_buf), fmt, val);
    assert(static_cast<size_t>(m_length) < sizeof(m_buf));
}


inline LogStream& operator<< (LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}




} // namespace muduo::base

#endif






















