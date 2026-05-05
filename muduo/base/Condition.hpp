#ifndef __MUDUO_BASE_CONDITION_H__
#define __MUDUO_BASE_CONDITION_H__

#include "muduo/base/Mutex.hpp"
#include "muduo/base/noncopyable.hpp"

#include <pthread.h>

namespace muduo::base
{

class Condition : noncopyable
{
public:

    explicit Condition(MutexLock& mutex)
        : m_mutex(mutex)
    {
        MCHECK(pthread_cond_init(&m_pcond, nullptr));
    }


    ~Condition()
    {
        MCHECK(pthread_cond_destroy(&m_pcond));
    }

    void wait()
    {
        MutexLock::UnassignGuard ug(m_mutex);
        MCHECK(pthread_cond_wait(&m_pcond, m_mutex.getPthreadMutex()));
    }

    bool waitForSeconds(double seconds);

    void notity()
    {
        MCHECK(pthread_cond_signal(&m_pcond));
    }

    void notifyAll()
    {
        MCHECK(pthread_cond_broadcast(&m_pcond));
    }

private:
    MutexLock& m_mutex;
    pthread_cond_t m_pcond;
};


}
#endif