#include "muduo/base/Condition.hpp"

#include <errno.h>

namespace muduo::base
{

bool Condition::waitForSeconds(double seconds)
{
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    constexpr int64_t kNanoSecondsPerSecond = 1000000000;
    int64_t nonoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nonoseconds) / kNanoSecondsPerSecond);
    abstime.tv_nsec += static_cast<long>((abstime.tv_nsec + nonoseconds) % kNanoSecondsPerSecond);

    MutexLock::UnassignGuard ug(m_mutex);

    return ETIMEDOUT == pthread_cond_timedwait(&m_pcond, m_mutex.getPthreadMutex(), &abstime);
}

}
