#ifndef __MUDUO_BASE_TIMESTAMP_H__
#define __MUDUO_BASE_TIMESTAMP_H__

#include "muduo/base/copyable.hpp"
#include "muduo/base/Types.hpp"

//#include <boost/operators.hpp>

namespace muduo::base
{

class Timestamp : public muduo::base::copyable
{
public:
    Timestamp()
        : m_microSecondsSinceEpoch(0)
    {

    }

    explicit Timestamp(int64_t microSecondsSinceEpoch) 
        : m_microSecondsSinceEpoch(microSecondsSinceEpoch)
    {

    }

    bool operator==(const Timestamp& ) const = default;
    auto operator<=>(const Timestamp&) const = default;


    void swap(Timestamp& other)
    {
        std::swap(m_microSecondsSinceEpoch, other.m_microSecondsSinceEpoch);
    }

    std::string toString() const;
    std::string toFormattedString(bool showMicroSeconds = true) const;

    bool vaild() const { return m_microSecondsSinceEpoch > 0; }

    int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch; }

    time_t secondsSinceEpoch() const 
    { 
        return static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond); 
    }
  
    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }

    static Timestamp formUnixTime(time_t t)
    {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microsecods)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microsecods);
    }


    static const int kMicroSecondsPerSecond = 1000 * 1000; 

private:
    int64_t m_microSecondsSinceEpoch;
};


inline double timeDifference(Timestamp hig, Timestamp low)
{
    int64_t diff = hig.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delte = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delte);
}

    
} // namespace muduo::base




#endif 