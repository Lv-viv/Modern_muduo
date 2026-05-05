#ifndef __MUDUO_BASE_COUNTDOWNLATCH_H__
#define __MUDUO_BASE_COUNTDOWNLATCH_H__

#include "muduo/base/noncopyable.hpp"
#include "muduo/base/Mutex.hpp"
#include "muduo/base/Condition.hpp"

namespace muduo::base
{

class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const { MutexLockGuard lock(m_mutex); return m_count; }


private:
    mutable MutexLock m_mutex;
    Condition m_condition GUARDED_BY(m_mutex);
    int m_count GUARDED_BY(m_mutex);
};


}


#endif