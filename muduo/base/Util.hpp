#ifndef __MUDUO_BASE_UTIL_H__
#define __MUDUO_BASE_UTIL_H__

#include <type_traits>

namespace muduo::base
{

template<typename T>
concept ArithmeticType = std::is_arithmetic_v<T>;

}


#endif