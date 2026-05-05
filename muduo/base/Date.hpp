#ifndef __MUDUO_BASE_DATE_H__

#include "muduo/base/copyable.hpp"
#include "muduo/base/Types.hpp"

#include <time.h>

#include <compare>

namespace muduo::base
{
/// 公历日期。
///
/// 本类不可变。
/// 建议按值传递，因为在 x64 架构中会通过寄存器传递

class Date : public copyable
{
public:
    struct YearMonthDay
    {
        int year;
        int month;
        int day;
    };

    static const int kDaysPeraWeek = 7;
    static const int kJulianDayOff_1970_01_01;

    Date() : m_julianDayNumber(0)
    {

    }


    Date(int year, int month, int day);

    explicit Date(const struct tm& t);

    explicit Date(int julanDay) : m_julianDayNumber(julanDay)
    {

    }

    void swap(Date& that)
    {
        std::swap(m_julianDayNumber, that.m_julianDayNumber);
    }

    bool vaild() const { return m_julianDayNumber > 0; }

    int julianDayNumber() const { return m_julianDayNumber; }

    std::string toIsoString() const;

    struct YearMonthDay yearMonthDay() const;

    int year() const
    {
        return yearMonthDay().year;
    }

    int month() const
    {
        return yearMonthDay().month;
    }

    int day() const
    {
        return yearMonthDay().day;
    }

    // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
    int weekDay()
    {
        return (m_julianDayNumber + 1) % kDaysPeraWeek;
    }
    auto operator<=>(const Date& other) const
    {
        return julianDayNumber() <=> other.julianDayNumber();
    }

    bool operator==(const Date& other) const
    {
        return julianDayNumber() == other.julianDayNumber();
    }

private:
    int m_julianDayNumber;
};

}

#endif 