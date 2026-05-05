#ifndef __MUDUO_BASE_TIMEZEON_H__
#define __MUDUO_BASE_TIMEZEON_H__

#include "muduo/base/Types.hpp"
#include "muduo/base/copyable.hpp"
#include "muduo/base/Date.hpp"

#include <time.h>
#include <memory>
#include <format>
#include <string_view>

namespace muduo::base
{

struct DateTime
{

    DateTime() = default;

    explicit DateTime(const tm& t);
    DateTime(int _year, int _month ,int _day ,int _hour ,int _minute, int _second)
        : year(_year), month(_month), day(_day), hour(_hour), minute(_minute), second(_second)
    {

    }

    std::string toString() const;

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
};

class TimeZone : public copyable
{
public:
    TimeZone() = default;
    ~TimeZone() = default;

    TimeZone(int eastOfUtc, const char* tzname);

    static TimeZone UTC();
    static TimeZone China();
    static TimeZone loadZoneFile(const std::string_view zoneFile);
    

    bool vaild() const
    {
        return static_cast<bool>(m_data);
    }


    DateTime toLocalTime(int64_t secondsSinceEpoch, int* utcOffset = nullptr) const;
    int64_t fromLocalTime(const DateTime& dataTime, bool postTransition = false) const;


    static DateTime toUtcTime(int64_t secondsSinceEpoch);
    static int64_t fromUtcTime(const DateTime& dataTime);

    struct Data;

private:

    explicit TimeZone(std::unique_ptr<Data> data);

    std::shared_ptr<Data> m_data;

};


}
#endif