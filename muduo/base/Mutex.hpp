#ifndef __MUDUO_BASE_MUTEX_H__
#define __MUDUO_BASE_MUTEX_H__

#include "muduo/base/CurrentThread.hpp"
#include "muduo/base/noncopyable.hpp"


#include <assert.h>
#include <pthread.h>

#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x) // no-np
#endif

#define CAPABILITY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(capablity(x))


#define SCOPED_CAPABILITY\
    THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRES(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRES_SHARED(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...)\
    THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x)\
    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)



#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum, 
                        const char* file,
                        unsigned int line,
                        const char* function)
    noexcept __attribute__((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else //CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace muduo::base
{


class CAPABILITY("mutex") MutexLock : noncopyable
{
public:
    MutexLock()
        : m_holder(0)
    {
        MCHECK(pthread_mutex_init(&m_mutex, nullptr));
    }

    ~MutexLock()
    {
        assert(m_holder == 0);
        MCHECK(pthread_mutex_destroy(&m_mutex));
    }

    bool isLockedByThisThread() const
    {
        return m_holder == CurrentThread::tid();
    }

    void assertLocked() const ASSERT_CAPABILITY(this)
    {
        assert(isLockedByThisThread());
    }

    void lock() ACQUIRES()
    {
        MCHECK(pthread_mutex_lock(&m_mutex));
    }

    void unlock() ACQUIRES()
    {
        MCHECK(pthread_mutex_unlock(&m_mutex));
    }

    pthread_mutex_t* getPthreadMutex()
    {
        return &m_mutex;
    }

private:
    friend class Condition;

    class UnassignGuard : noncopyable
    {
    public:
        explicit UnassignGuard(MutexLock& owner)
            : m_owner(owner)
        {
            m_owner.unassignHolder();
        }

        ~UnassignGuard()
        {
            m_owner.assignHolder();
        }

    private:
        MutexLock& m_owner;
    };

    void unassignHolder()
    {
        m_holder = 0;
    }

    void assignHolder()
    {
        m_holder = CurrentThread::tid();
    }




private:
    pthread_mutex_t m_mutex;
    pid_t m_holder;
};

class SCOPED_CAPABILITY MutexLockGuard : public noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex)
      :  m_mutex(mutex)
    {
        m_mutex.lock();
    }

    ~MutexLockGuard() RELEASE()
    {
        m_mutex.unlock();
    }

private:
    MutexLock& m_mutex;
};


} // namespace muduo::base


#define MutexLockGuard(x) error "Missing guard object name"


#endif