#ifndef __MUDUO_BASE_CURRENT_THREAD_H__
#define __MUDUO_BASE_CURRENT_THREAD_H__

#include "muduo/base/Types.hpp"

namespace muduo::base::CurrentThread
{


extern thread_local int t_cachedTid;
extern thread_local char t_tidString[32];
extern thread_local int t_tidStringLength;
extern thread_local const char* t_threadName;

void cacheTid();

inline int tid()
{
    if (t_cachedTid == 0) [[unlikely]]
    {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString()
{
    return t_tidString;
}

inline int tidStringLingth()
{
    return t_tidStringLength;
}

inline const char* name()
{
    return t_threadName;
}

bool isMainThread();

void sleepUsec(int64_t usec); ///

std::string stackTrace(bool demangle);


}


#endif