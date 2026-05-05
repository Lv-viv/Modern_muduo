#include "muduo/base/TimeZone.hpp"
#include "muduo/base/Types.hpp"

#include <assert.h>
#include <string.h>

#include <print>


using namespace muduo::base;

struct tm getTm(int year, int month, int day, int hour, int minute, int second)
{
    struct tm gmt;
    memZero(&gmt, sizeof(gmt));
    gmt.tm_year = year - 1900;
    gmt.tm_mon = month - 1;
    gmt.tm_mday = day;
    gmt.tm_hour = hour;
    gmt.tm_min = minute;
    gmt.tm_sec = second;
    return gmt;
}

struct tm getTm(std::string_view str)
{
    struct tm gmt;
    memZero(&gmt, sizeof(gmt));
    strptime(str.data(), "%F %T", &gmt);
    return gmt;
}

time_t getGmt(int year, int month, int day, int hour, int minute, int second)
{
    auto gmt = getTm(year,month, day, hour, minute, second);
    return timegm(&gmt);
}

time_t getGmt(std::string_view str)
{
    auto gmt = getTm(str);
    return timegm(&gmt);
}

struct TestCase
{
    const char* gmt;
    const char* local;
    int postTransition;
};

int failure = 0;

void test(const TimeZone& tz, TestCase tc)
{
    const auto gmt = getGmt(tc.gmt);
    {
        int utcOffset = 0;
        std::string local = tz.toLocalTime(gmt, &utcOffset).toString();
        local += std::format(" {:+03d}{:02d}", utcOffset / 3600, utcOffset % 3600 / 60);
        
        if (local != tc.local)
        {
            std::print("WRONG ");
            std::println("'{}' -> '{}' got {}", tc.gmt, tc.local, local);
            failure++;
        }
        else
        {
            std::println("'{}' -> '{}'", tc.gmt, tc.local);
        }
    
    }

    {
        const auto local = getTm(tc.local);
        DateTime localtime(local.tm_year + 1900, local.tm_mon + 1,
                           local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

        const int64_t result = tz.fromLocalTime(localtime, tc.postTransition);
        if (result != gmt)
        {
            failure++;
            std::println("WRONG fromLocalTime: inport{} expect{} got{}", tc.local, tc.gmt, tz.toUtcTime(result).toString());
        }
    }
    
}

void testLondon()
{
  // UTC time             isdst  offset  Local time (London)
  // 2010-03-28 01:00:00Z   1     1.0    2010-03-28 02:00:00
  // 2010-10-31 01:00:00Z   0     0.0    2010-10-31 01:00:00
  // 2011-03-27 01:00:00Z   1     1.0    2011-03-27 02:00:00
  // 2011-10-30 01:00:00Z   0     0.0    2011-10-30 01:00:00
  // 2012-03-25 01:00:00Z   1     1.0    2012-03-25 02:00:00
  // 2012-10-28 01:00:00Z   0     0.0    2012-10-28 01:00:00
  // 2013-03-31 01:00:00Z   1     1.0    2013-03-31 02:00:00
  // 2013-10-27 01:00:00Z   0     0.0    2013-10-27 01:00:00
  // 2014-03-30 01:00:00Z   1     1.0    2014-03-30 02:00:00
  // 2014-10-26 01:00:00Z   0     0.0    2014-10-26 01:00:00

  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Europe/London");
  TestCase cases[] =
  {

    { "2011-03-26 00:00:00", "2011-03-26 00:00:00 +0000", false },
    { "2011-03-27 00:59:59", "2011-03-27 00:59:59 +0000", false },
    { "2011-03-27 01:00:00", "2011-03-27 02:00:00 +0100", false },
    { "2011-10-30 00:59:59", "2011-10-30 01:59:59 +0100", false },
    { "2011-10-30 01:00:00", "2011-10-30 01:00:00 +0000", true },
    { "2011-10-30 01:59:59", "2011-10-30 01:59:59 +0000", true },
    { "2011-12-31 22:00:00", "2011-12-31 22:00:00 +0000", false },
    { "2012-01-01 00:00:00", "2012-01-01 00:00:00 +0000", false },

    { "2012-03-24 00:00:00", "2012-03-24 00:00:00 +0000", false },
    { "2012-03-25 00:59:59", "2012-03-25 00:59:59 +0000", false },
    { "2012-03-25 01:00:00", "2012-03-25 02:00:00 +0100", false },
    { "2012-10-28 00:59:59", "2012-10-28 01:59:59 +0100", false },
    { "2012-10-28 01:00:00", "2012-10-28 01:00:00 +0000", true },
    { "2012-10-28 01:59:59", "2012-10-28 01:59:59 +0000", true },
    { "2012-12-31 22:00:00", "2012-12-31 22:00:00 +0000", false },
    { "2013-01-01 00:00:00", "2013-01-01 00:00:00 +0000", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testNewYork()
{
  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/America/New_York");
  TestCase cases[] =
  {

    // Unix Epoch is 1969-12-31 local time.
    { "1970-01-01 00:00:00", "1969-12-31 19:00:00 -0500", false },

    { "2006-03-07 00:00:00", "2006-03-06 19:00:00 -0500", false },
    { "2006-04-02 06:59:59", "2006-04-02 01:59:59 -0500", false },
    { "2006-04-02 07:00:00", "2006-04-02 03:00:00 -0400", false },
    { "2006-05-01 00:00:00", "2006-04-30 20:00:00 -0400", false },
    { "2006-05-02 01:00:00", "2006-05-01 21:00:00 -0400", false },
    { "2006-10-21 05:00:00", "2006-10-21 01:00:00 -0400", false },
    { "2006-10-29 05:59:59", "2006-10-29 01:59:59 -0400", false },
    { "2006-10-29 06:00:00", "2006-10-29 01:00:00 -0500", true },
    { "2006-10-29 06:30:00", "2006-10-29 01:30:00 -0500", true },
    { "2006-12-31 06:00:00", "2006-12-31 01:00:00 -0500", false },
    { "2007-01-01 00:00:00", "2006-12-31 19:00:00 -0500", false },

    { "2007-03-07 00:00:00", "2007-03-06 19:00:00 -0500", false },
    { "2007-03-11 06:59:59", "2007-03-11 01:59:59 -0500", false },
    { "2007-03-11 07:00:00", "2007-03-11 03:00:00 -0400", false },
    { "2007-05-01 00:00:00", "2007-04-30 20:00:00 -0400", false },
    { "2007-05-02 01:00:00", "2007-05-01 21:00:00 -0400", false },
    { "2007-10-31 05:00:00", "2007-10-31 01:00:00 -0400", false },
    { "2007-11-04 05:59:59", "2007-11-04 01:59:59 -0400", false },
    { "2007-11-04 06:00:00", "2007-11-04 01:00:00 -0500", true },
    { "2007-11-04 06:59:59", "2007-11-04 01:59:59 -0500", true },
    { "2007-12-31 06:00:00", "2007-12-31 01:00:00 -0500", false },
    { "2008-01-01 00:00:00", "2007-12-31 19:00:00 -0500", false },

    { "2009-03-07 00:00:00", "2009-03-06 19:00:00 -0500", false },
    { "2009-03-08 06:59:59", "2009-03-08 01:59:59 -0500", false },
    { "2009-03-08 07:00:00", "2009-03-08 03:00:00 -0400", false },
    { "2009-05-01 00:00:00", "2009-04-30 20:00:00 -0400", false },
    { "2009-05-02 01:00:00", "2009-05-01 21:00:00 -0400", false },
    { "2009-10-31 05:00:00", "2009-10-31 01:00:00 -0400", false },
    { "2009-11-01 05:59:59", "2009-11-01 01:59:59 -0400", false },
    { "2009-11-01 06:00:00", "2009-11-01 01:00:00 -0500", true },
    { "2009-11-01 06:59:59", "2009-11-01 01:59:59 -0500", true },
    { "2009-12-31 06:00:00", "2009-12-31 01:00:00 -0500", false },
    { "2010-01-01 00:00:00", "2009-12-31 19:00:00 -0500", false },

    { "2010-03-13 00:00:00", "2010-03-12 19:00:00 -0500", false },
    { "2010-03-14 06:59:59", "2010-03-14 01:59:59 -0500", false },
    { "2010-03-14 07:00:00", "2010-03-14 03:00:00 -0400", false },
    { "2010-05-01 00:00:00", "2010-04-30 20:00:00 -0400", false },
    { "2010-05-02 01:00:00", "2010-05-01 21:00:00 -0400", false },
    { "2010-11-06 05:00:00", "2010-11-06 01:00:00 -0400", false },
    { "2010-11-07 05:59:59", "2010-11-07 01:59:59 -0400", false },
    { "2010-11-07 06:00:00", "2010-11-07 01:00:00 -0500", true },
    { "2010-11-07 06:59:59", "2010-11-07 01:59:59 -0500", true },
    { "2010-12-31 06:00:00", "2010-12-31 01:00:00 -0500", false },
    { "2011-01-01 00:00:00", "2010-12-31 19:00:00 -0500", false },

    { "2011-03-01 00:00:00", "2011-02-28 19:00:00 -0500", false },
    { "2011-03-13 06:59:59", "2011-03-13 01:59:59 -0500", false },
    { "2011-03-13 07:00:00", "2011-03-13 03:00:00 -0400", false },
    { "2011-05-01 00:00:00", "2011-04-30 20:00:00 -0400", false },
    { "2011-05-02 01:00:00", "2011-05-01 21:00:00 -0400", false },
    { "2011-11-06 05:59:59", "2011-11-06 01:59:59 -0400", false },
    { "2011-11-06 06:00:00", "2011-11-06 01:00:00 -0500", true },
    { "2011-11-06 06:59:59", "2011-11-06 01:59:59 -0500", true },
    { "2011-12-31 06:00:00", "2011-12-31 01:00:00 -0500", false },
    { "2012-01-01 00:00:00", "2011-12-31 19:00:00 -0500", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testHongKong()
{
    TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Asia/Hong_Kong");
    TestCase cases[] =
    {

        { "2011-04-03 00:00:00", "2011-04-03 08:00:00 +0800", false},

    };

    for (const auto& c : cases)
    {
        test(tz, c);
    }
}

void testSyndney()
{
    // DST starts in winter
    // UTC time             isdst  offset  Local time (London)
    // 2010-04-03 16:00:00Z isdst 0 offset  10.0  2010-04-04 02:00:00
    // 2010-10-02 16:00:00Z isdst 1 offset  11.0  2010-10-03 03:00:00
    // 2011-04-02 16:00:00Z isdst 0 offset  10.0  2011-04-03 02:00:00
    // 2011-10-01 16:00:00Z isdst 1 offset  11.0  2011-10-02 03:00:00
    // 2012-03-31 16:00:00Z isdst 0 offset  10.0  2012-04-01 02:00:00
    // 2012-10-06 16:00:00Z isdst 1 offset  11.0  2012-10-07 03:00:00

    TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Australia/Sydney");
    TestCase cases[] =
    {

        { "2011-01-01 00:00:00", "2011-01-01 11:00:00 +1100", false },
        { "2011-04-02 15:59:59", "2011-04-03 02:59:59 +1100", false },
        { "2011-04-02 16:00:00", "2011-04-03 02:00:00 +1000", true },
        { "2011-04-02 16:59:59", "2011-04-03 02:59:59 +1000", true },
        { "2011-05-02 01:00:00", "2011-05-02 11:00:00 +1000", false },
        { "2011-10-01 15:59:59", "2011-10-02 01:59:59 +1000", false },
        { "2011-10-01 16:00:00", "2011-10-02 03:00:00 +1100", false },
        { "2011-12-31 22:00:00", "2012-01-01 09:00:00 +1100", false },

    };
    for (const auto& c : cases)
    {
        test(tz, c);
    }
}

void testUTC()
{
    TimeZone utc = TimeZone::loadZoneFile("/usr/share/zoneinfo/UTC");
    const int kRang = 100 * 1000 * 1000;

    for (time_t t = -kRang; t <= kRang; t += 11)
    {
        struct tm* t1 = gmtime(&t);
        char buf[80];
        strftime(buf, sizeof(buf), "%F %T", t1);
        DateTime t2 = TimeZone::toUtcTime(t);
        std::string t2str = t2.toString();
        if (t2str != buf)
        {
            std::println("'{}' != '{}'", t2str, buf);
            failure++;
            assert(0);
        }

        DateTime t3 = utc.toLocalTime(t);
        std::string t3str = t3.toString();
        if (t3str != buf)
        {
            std::println("'{}' != '{}'", t3str, buf);
            failure++;
            assert(0);
        }

        int64_t u1 = TimeZone::fromUtcTime(t2);
        if (t != u1)
        {
            std::println("'{}' != '{}'", t, u1);
            failure++;
            assert(0);
        }
    }
}

void testFixedTimeZone()
{
    TimeZone tz(8 * 3600, "CST");
    TestCase cases[] =
    {
        { "2014-04-03 00:00:00", "2014-04-03 08:00:00 +0800", false},
    };

    for (const auto& c : cases)
    {
        test(tz, c);
    }

    tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Etc/GMT-8");
    for (const auto& c : cases)
    {
        test(tz, c);
    }
}

int main()
{

    //testUTC();
    //testFixedTimeZone();
    //testSyndney();
    //testHongKong();
    //testLondon();


    testNewYork();


    
    return 0;
}