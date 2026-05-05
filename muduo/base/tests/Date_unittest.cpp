#include "muduo/base/Date.hpp"

#include <time.h>
#include <assert.h>
#include <iostream>
#include <print>


using namespace muduo::base;

constexpr int kMonthsOfYear = 12;

int isLeapYear(int year)
{
    if (0 == year % 400)
    {
        return 1;
    }
    else if (0 == year % 100)
    {
        return 1;
    }
    else if (0 == year % 4)
    {
        return 1;
    }
    return 0;
} 

int daysOfMonth(int year, int month)
{
    static int days[2][kMonthsOfYear + 1] = {
        { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
        { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    };
    return days[isLeapYear(year)][month];
}

void passByValue(Date day)
{
    std::println("{}", day.toIsoString());
}

void passByConstReference(const Date& day)
{
    std::println("{}", day.toIsoString());
}

int main()
{

    time_t now =  time(nullptr);
    auto t1 = *gmtime(&now);
    auto t2 = *localtime(&now);
    Date someDay(2008, 9, 10);
    std::println("{}", someDay.toIsoString());
    passByValue(someDay);
    passByConstReference(someDay);

    Date toDayUtc(t1);
    std::println("{}", toDayUtc.toIsoString());
    Date toLocal(t2);
    std::println("{}", toLocal.toIsoString());

    int julanDayNumber = 2415021;
    int weekDay = 1; // Monday

    for (int year = 1900; year < 2500; ++year)
    {
        assert(Date(year, 3, 1).julianDayNumber() - Date(year, 2, 29).julianDayNumber() == isLeapYear(year));
        for (int month = 1; month <= kMonthsOfYear; ++month)
        {
            for (int day = 1; day <= daysOfMonth(year, month); ++day)
            {
                Date d(year, month, day);
                assert(day == d.day());
                assert(month == d.month());
                assert(year == d.year());
                assert(weekDay == d.weekDay());
                assert(julanDayNumber == d.julianDayNumber());

                Date d2(julanDayNumber);
                assert(day == d2.day());
                assert(month == d2.month());
                assert(year == d2.year());
                assert(weekDay == d2.weekDay());
                assert(julanDayNumber == d2.julianDayNumber());

                ++julanDayNumber;
                weekDay = (weekDay + 1) % 7;
            }
        }
    }
    std::println("All pass");
    return 0;
}