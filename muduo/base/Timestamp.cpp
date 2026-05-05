#include "muduo/base/Timestamp.hpp"
#include "Timestamp.hpp"

#include <format>

#include <time.h>
#include <sys/time.h>

namespace muduo::base
{

std::string Timestamp::toString() const
{
    int64_t seconds =  m_microSecondsSinceEpoch / kMicroSecondsPerSecond;
    int64_t mircroseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;

    return std::format("{}.{}", seconds, mircroseconds);
}

std::string Timestamp::toFormattedString(bool showMicroSeconds) const
{
    std::string result;
    time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
    tm tm_time;
    gmtime_r(&seconds ,&tm_time );

    if (showMicroSeconds)
    {
        int microseconds = static_cast<int>(m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
        result = std::format("{:04d}{:02d}{:02d} {:02d}:{:02d}:{:02d}.{:06d}", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    }
    else
    {
        result = std::format("{:04d}{:02d}{:02d} {:02d}:{:02d}:{:02d}", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return result;
}

Timestamp Timestamp::now()
{
    timeval tv;
    gettimeofday(&tv, nullptr);

    int64_t seconds = tv.tv_sec;

    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

}
