#include "muduo/base/Date.hpp"

#include <format>

namespace muduo::base
{

namespace detail
{
    constexpr int getJulianDayNumber(int year, int month, int day)
    {
        static_assert(sizeof(int) >= sizeof(int32_t), "Error int size great int32_t");
        int a = (14 - month) / 12;
        int y = year + 4800 - a;
        int m = month + 12 * a - 3;
        return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
    }

    constexpr Date::YearMonthDay getYearMonthDay(int julianDayNumber)
    {
        int a = julianDayNumber + 32044;
        int b = (4 * a + 3) / 146097;
        int c = a - ((b * 146097) / 4);
        int d = (4 * c + 3) / 1461;
        int e = c - ((1461 * d) / 4);
        int m = (5 * e + 2) / 153;
        Date::YearMonthDay ymd;
        ymd.day = e - ((153 * m + 2) / 5) + 1;
        ymd.month = m + 3 - 12 * (m / 10);
        ymd.year = b * 100 + d - 4800 + (m / 10);
        return ymd;
    }

}

const int Date::kJulianDayOff_1970_01_01 = detail::getJulianDayNumber(1970, 1, 1);

Date::Date(int year, int month, int day)
    : m_julianDayNumber(detail::getJulianDayNumber(year, month, day))
{
}

Date::Date(const tm &t)
    : m_julianDayNumber(detail::getJulianDayNumber(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday))
{
}

std::string Date::toIsoString() const
{
    YearMonthDay ymd(detail::getYearMonthDay(m_julianDayNumber));
    return std::format("{:04}-{:02}-{:02}", ymd.year, ymd.month, ymd.day);
}

Date::YearMonthDay Date::yearMonthDay() const
{
    return detail::getYearMonthDay(m_julianDayNumber);
}

}
