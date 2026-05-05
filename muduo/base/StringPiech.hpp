#ifndef __MUDUO_BASE_STRINGPIECE_H__
#define __MUDUO_BASE_STRINGPIECE_H__

#include <string.h>
#include <iosfwd>


#include "muduo/base/Types.hpp"


namespace muduo::base
{

    // std::string_view
class StringArg
{
public:
    StringArg(const char* str)
        : m_str(str)
    {

    }

    StringArg(const std::string& str)
        : m_str(str.c_str())
    {

    }


    const char* c_str() const { return m_str; }


private:

    const char* m_str;

};


class StringPiece
{
private:
    const char* m_ptr;
    int m_length;

public:
    StringPiece()
        : m_ptr(nullptr)
        , m_length(0)
    {

    }

    StringPiece(const char* str)
        : m_ptr(str), m_length(static_cast<int>(strlen(m_ptr)))
    {

    }

    
    StringPiece(const unsigned char* str)
        : m_ptr(reinterpret_cast<const char*>(str)) , m_length(static_cast<int>(strlen(m_ptr)))
    {

    }

    StringPiece(const std::string& str)
        : m_ptr(str.c_str()), m_length(static_cast<int>(str.size()))
    {

    }

    StringPiece(const std::string_view str)
        : m_ptr(str.data()), m_length(static_cast<int>(str.size()))
    {

    }

    StringPiece(const char* offset, int len)
        : m_ptr(offset), m_length(len)
    {

    }

    const char* data() const { return m_ptr; }
    int size() const { return m_length; }
    bool empty() const { return m_length == 0; }
    const char* begin() const { return m_ptr; }
    const char* end() const { return m_ptr + m_length; }

    void clear()  { m_ptr = nullptr; m_length = 0; }
    void set(const char* buffer, int len) { m_ptr = buffer; m_length = len; }
    void set(const char* buffer)
    {
        m_ptr = buffer;
        m_length = static_cast<int>(strlen(buffer));
    }

    void set(const void* buff, int len)
    {
        m_ptr = reinterpret_cast<const char*>(buff);
        m_length = len;
    }

    // 注意范围
    char operator[](int i) const
    {
        return m_ptr[i];
    }

    void remove_prefix(int n)
    {
        m_ptr += n;
        m_length -= n;
    }

    void remove_suffix(int n)
    {
        m_length -= n;
    }

    bool operator==(const StringPiece& x) const
    {
        return ((m_length == x.m_length) && (memcmp(m_ptr, x.m_ptr, m_length) == 0));
    }

    bool operator!=(const StringPiece& x) const
    {
        return !(*this == x);
    }

#define STRINGPIECH_BINARY_PREDICATE(cmp, auxcmp)                                       \
    bool operator cmp(const StringPiece& x)    const {                                  \
        int r = memcmp(m_ptr, x.m_ptr, m_length < x.m_length ? m_length : x.m_length);  \
        return ((r auxcmp 0) ||((r == 0) && (m_length cmp x.m_length)));                   \
    }

    STRINGPIECH_BINARY_PREDICATE(<, <);
    STRINGPIECH_BINARY_PREDICATE(<=, <);
    STRINGPIECH_BINARY_PREDICATE(>=, >);
    STRINGPIECH_BINARY_PREDICATE(>, >);

#undef STRINGPIECH_BINARY_PREDICATE


    int compare(const StringPiece& x) const
    {
        int r = memcmp(m_ptr, x.m_ptr, m_length < x.m_length ? m_length : x.m_length);
        if (r == 0)
        {
            if (m_length < x.m_length)
            {
                r = -1;
            }
            else if (m_length > x.m_length)
            {
                r = +1;
            }
        }
        return r;
    }


    std::string as_string() const
    {
        return std::string(m_ptr, m_length);
    }

    std::string_view as_string_view() const
    {
        return std::string_view(m_ptr, m_length);
    }

    void CopyToString(std::string* target) const
    {
        target->assign(m_ptr, m_length);
    }

    bool starts_with(const StringPiece& x) const
    {
        return ((m_length >= x.m_length) && (memcmp(m_ptr, x.m_ptr, x.m_length) == 0));
    }

};

} // namespace muduo::base

#ifdef HAVE_TYPE_TRAITS
// 没有用
template<> struct std::__type_traits<muduo::base::StringPiece> {
  typedef __true_type    has_trivial_default_constructor;
  typedef __true_type    has_trivial_copy_constructor;
  typedef __true_type    has_trivial_assignment_operator;
  typedef __true_type    has_trivial_destructor;
  typedef __true_type    is_POD_type;
};
#endif


#endif