#include "CountDownLatch.hpp"


namespace muduo::base
{

CountDownLatch::CountDownLatch(int count)
    :m_mutex()
    , m_condition(m_mutex)
    , m_count(count)
    
{

}

void CountDownLatch::wait()
{
    MutexLockGuard lock(m_mutex);
    while (m_count > 0)
    {
        m_condition.wait();
    }
}

void CountDownLatch::countDown()
{
    MutexLockGuard lock(m_mutex);
    --m_count;
    if (0 == m_count)
    {
        m_condition.notifyAll();
    }
}

}