#ifndef __MUDUO_BASE_ATOMIC_H__
#define __MUDUO_BASE_ATOMIC_H__

#include "muduo/base/noncopyable.hpp"
#include "muduo/base/Util.hpp"

#include <stdint.h>
#include <pthread.h>

namespace muduo::base
{


namespace detail
{

template <ArithmeticType T>
class AtomicIntergerT : noncopyable
{

public:
    AtomicIntergerT()
        : m_value(0)
    {

    }

    T get()
    {
        return __sync_val_compare_and_swap(&m_value, 0, 0);
    }

    T getAndAdd(T x)
    {
        return __sync_fetch_and_add(&m_value, x);
    }

    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
    }

    T incrementAndGet()
    {
        return addAndGet(1);
    }

    T decrementAndGet()
    {
        return addAndGet(-1);
    }

    void add(T x)
    {
        getAndAdd(x);
    }

    void increment()
    {
        incrementAndGet();
    }

    void decrement()
    {
        decrementAndGet();
    }

    T getAndSet(T newValue)
    {
        return __sync_lock_test_and_set(&m_value, newValue);
    }

private:

    volatile T m_value;
};


}

using AtomicInt32 = detail::AtomicIntergerT<int32_t>;
using AtimicInt64 = detail::AtomicIntergerT<int64_t>;


}



#endif