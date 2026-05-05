#ifndef __MUDUO_BASE_TYPES_H__
#define __MUDUO_BASE_TYPES_H__

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <concepts>

namespace muduo::base
{

inline void memZero(void* p, size_t n)
{
    memset(p, 0, n);
}

template <typename To, typename From>
    requires std::convertible_to<From, To>
inline To implicit_cast(const From &f)
{
    return f;
}

template <typename To, typename From>
    requires std::derived_from<From, std::remove_pointer_t<To>>
inline To down_cast(From* f)
{
    return static_cast<To>(f);
}

}
#endif