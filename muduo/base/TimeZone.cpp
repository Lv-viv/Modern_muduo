#include "muduo/base/TimeZone.hpp"

#include "muduo/base/noncopyable.hpp"

#include <vector>
#include <stdexcept>

#include <stdio.h>
#include <assert.h>
#include <endian.h>


namespace muduo::base
{
constexpr int kSecondsPerDay = 24 * 60 * 60;

struct TimeZone::Data
{
    struct Transition
    {
        int64_t utctimel;
        int64_t localtime;
        int localtimeIdx;

        Transition(int64_t t, int64_t l, int localIdx)
            : utctimel(t), localtime(l), localtimeIdx(localIdx)
        {
        
        }
    };

    struct LocalTime
    {
        int utcOffset;
        bool isDst;
        int desigIdx;

        LocalTime(int offset, bool dst, int idx)
            : utcOffset(offset), isDst(dst), desigIdx(idx)
        {

        }

    };
    

    struct CompareUtcTime
    {
        bool operator()(const Transition& lhs, const Transition& rhs) const
        {
            return lhs.utctimel < rhs.utctimel;
        }
    };

    struct CompareLocalTime
    {
        bool operator()(const Transition& lhs, const Transition& rhs) const
        {
            return lhs.localtime < rhs.localtime;
        }
    };

    void addLocalTime(int32_t uctOffset, bool isDst, int desigIdx)
    {
        localtimes.emplace_back(uctOffset, isDst, desigIdx);
    }

    void addTransition(int64_t utctimel, int localtimeIdx)
    {
        LocalTime lt = localtimes.at(localtimeIdx);
        transitions.emplace_back(utctimel, utctimel + lt.utcOffset, localtimeIdx);
    }

    const LocalTime* findLocalTime(int64_t utcTime) const;

    const LocalTime* findLocalTime(const DateTime& local, bool postTransition) const;

    std::vector<Transition> transitions;
    std::vector<LocalTime> localtimes;
    std::string abberviation;
    std::string tzstring;
};



namespace detail
{

class File : public noncopyable
{
public:
    explicit File(std::string_view file)
        :m_fp(::fopen(file.data(), "rb"))
    {

    }


    bool valid() const { return m_fp; }

    std::string readBytes(int n)
    {
        std::unique_ptr<char[]> buf(new char[n]);
        ssize_t nr = ::fread(buf.get(), 1, n, m_fp);
        if (nr != static_cast<ssize_t>(n))
        {
            throw std::logic_error("no enough data");
        }
        return std::string(buf.get(), n);
    }

    std::string readToEnd()
    {
        ssize_t nr;
        char buf[4096];
        std::string result;
        while((nr = ::fread(buf, 1, sizeof(buf), m_fp)) > 0)
        {
            result.append(buf, nr);
        }
        return result;
    }

    int64_t readInt64()
    {
        int64_t x;
        ssize_t nr = ::fread(&x, 1, sizeof(int64_t), m_fp);
        if (nr != sizeof(int64_t))
        {
            throw std::logic_error("bad int64_t data");
        }
        return be64toh(x);
    }

    int32_t readInt32()
    {
        int32_t x;
        ssize_t nr = ::fread(&x, 1, sizeof(int32_t), m_fp);
        if (nr != sizeof(int32_t))
        {
            throw std::logic_error("bad int64_t data");
        }
        return be32toh(x);
    }

    int8_t readInt8()
    {
        int8_t x;
        ssize_t nr = ::fread(&x, 1, sizeof(int8_t), m_fp);
        if (nr != sizeof(int8_t))
        {
            throw std::logic_error("bad int64_t data");
        }
        return x;
    }

    off_t skip(ssize_t bytes)
    {
        return ::fseek(m_fp, bytes, SEEK_CUR);
    }

private:
    FILE* m_fp;
};


inline void fileHMS(unsigned seconds, DateTime& dt)
{
    dt.second = seconds % 60;
    unsigned minutes = seconds / 60;
    dt.minute = minutes % 60;
    dt.hour = minutes / 60;
}

DateTime BreakTime(int64_t t)
{
    DateTime dt;

    int seconds = static_cast<int>(t % kSecondsPerDay);
    int days = static_cast<int>(t / kSecondsPerDay);

    if (seconds < 0)
    {
        seconds += kSecondsPerDay;
        --days;
    }

    detail::fileHMS(seconds, dt);
    Date date(days + Date::kJulianDayOff_1970_01_01);
    Date::YearMonthDay ymd = date.yearMonthDay();
    dt.year = ymd.year;
    dt.month = ymd.month;
    dt.day = ymd.day;
    return dt;
}

// RFC 8536: https://www.rfc-editor.org/rfc/rfc8536.html
bool readDataBlock(File& f, TimeZone::Data* data, bool v1)
{
    const int time_size = v1 ? sizeof(int32_t) : sizeof(int64_t);

    const int32_t isutccnt = f.readInt32();
    const int32_t isstdcnt = f.readInt32();
    const int32_t leapcnt = f.readInt32();
    const int32_t timecnt = f.readInt32();
    const int32_t typecnt = f.readInt32();
    const int32_t charcnt = f.readInt32();

    if (leapcnt != 0)
    {
        return false;
    }
    if (isutccnt != 0 && typecnt != isutccnt)
    {
        return false;
    }

    if (isstdcnt != 0 && typecnt != isstdcnt)
    {
        return false;
    }

    std::vector<int64_t> trans;
    trans.reserve(timecnt);
    for (int i = 0; i < timecnt; ++i)
    {
        if (v1)
        {
            trans.push_back(f.readInt32());
        }
        else
        {
            trans.push_back(f.readInt64());
        }
    }

    std::vector<int64_t> localtimes;
    localtimes.reserve(timecnt);
    for (int i = 0; i < timecnt; ++i)
    {
        int8_t local = f.readInt8();
        localtimes.push_back(local);
    }

    data->localtimes.reserve(typecnt);
    for (int i = 0; i < typecnt; ++i)
    {
        int32_t gmtoff = f.readInt32();
        int8_t isdst = f.readInt8();
        int8_t abbrind = f.readInt8();

        data->addLocalTime(gmtoff, isdst, abbrind);
    }

    for (int i = 0; i < timecnt; ++i)
    {
        int localIdx = static_cast<int>(localtimes[i]);
        data->addTransition(trans[i], localIdx);
    }

    data->abberviation = f.readBytes(charcnt);
    f.skip(leapcnt + (time_size * 4));
    f.skip(isstdcnt);
    f.skip(isutccnt);
    if (!v1)
    {
        data->tzstring = f.readToEnd();
    }
    return true;
}

bool readTimeZoneFile(std::string_view zonefile, TimeZone::Data* data)
{
    if (nullptr == data)
    {
        throw std::logic_error("data is nullptr");
    }

    File f(zonefile);
    if (f.valid())
    {
        try
        {
            std::string head = f.readBytes(4);
            if (head != "TZif")
            {
                throw std::logic_error("bad head");
            }

            std::string version = f.readBytes(1);
            f.readBytes(15);
            
            const int32_t isgmtcnt = f.readInt32();
            const int32_t isstdcnt = f.readInt32();
            const int32_t leapcnt = f.readInt32();
            const int32_t timecnt = f.readInt32();
            const int32_t typecnt = f.readInt32();
            const int32_t charcnt = f.readInt32();

            if ("2" == version)
            {
                size_t skip = sizeof(int32_t) * timecnt + timecnt + 6 * typecnt + charcnt + 8 * leapcnt + isstdcnt + isgmtcnt;

                f.skip(skip);

                head = f.readBytes(4);
                if (head != "TZif")
                {
                    throw std::logic_error("bad head");
                }
                f.skip(16);
                return readDataBlock(f, data, false);
            }
            else
            {
                f.skip(-4 * 6);
                return readDataBlock(f, data, true);
            }
        }
        catch(const std::logic_error& e)
        {
            fprintf(stderr, "%s\n", e.what());
        }

    }
    return false;
}


}




const TimeZone::Data::LocalTime *TimeZone::Data::findLocalTime(int64_t utcTime) const
{
    const LocalTime* local = nullptr;
    // 行  协调世界时             时区状态  偏移量  中国标准时间

    //  1  1989-09-16 17:00:00Z   0      8.0   1989-09-17 01:00:00
    //  2  1990-04-14 18:00:00Z   1      9.0   1990-04-15 03:00:00
    //  3  1990-09-15 17:00:00Z   0      8.0   1990-09-16 01:00:00
    //  4  1991-04-13 18:00:00Z   1      9.0   1991-04-14 03:00:00
    //  5  1991-09-14 17:00:00Z   0      8.0   1991-09-15 01:00:00

    assert(!localtimes.empty());

    if(transitions.empty() || utcTime < transitions.front().utctimel)
    {
        local = &localtimes.front();
    }
    else
    {
        Transition sentry(utcTime, 0, 0);
        auto transI = std::upper_bound(transitions.begin(), transitions.end(), sentry, CompareUtcTime());

        assert(transI != transitions.begin());
        if (transI != transitions.end())
        {
            --transI;
            local = &localtimes[transI->localtimeIdx];
        }
        else
        {
            // 待处理：使用 TZ-env
            local = &localtimes[transitions.back().localtimeIdx];
        }
    }

    return local;
}

const TimeZone::Data::LocalTime* TimeZone::Data::findLocalTime(const DateTime &local, bool postTransition) const
{
    const int64_t localtime = fromUtcTime(local);
    
    if (transitions.empty() || localtime < transitions.front().localtime)
    {
        return &localtimes.front();
    }

    Transition sentry(0, localtime, 0);

    auto transI = std::upper_bound(transitions.begin(), transitions.end(), sentry, CompareLocalTime());
    assert(transI != transitions.begin());

    if (transI == transitions.end())
    {
        return &localtimes[transitions.back().localtimeIdx];
    }

    Transition prior_trans = *(transI - 1);
    int64_t prior_second = transI->utctimel - 1 + localtimes[prior_trans.localtimeIdx].utcOffset;

    if (prior_second < localtime)
    {
        if (postTransition)
        {
            return &localtimes[transI->localtimeIdx];
        }
        else
        {
            return &localtimes[prior_trans.localtimeIdx];
        }
    }

    --transI;
    if (transI != transitions.begin())
    {
        prior_trans = *(transI - 1);
        prior_second = transI->utctimel - 1 + localtimes[prior_trans.localtimeIdx].utcOffset;
    }

    if ( localtime <= prior_second)
    {
        if (postTransition)
        {
            return &localtimes[transI->localtimeIdx];
        }
        else
        {
            return &localtimes[prior_trans.localtimeIdx];
        }
    }

    return &localtimes[transI->localtimeIdx];
}

DateTime::DateTime(const tm& t)
    :year(t.tm_year + 1900) , month(t.tm_mon + 1), day(t.tm_mday), hour(t.tm_hour), minute(t.tm_min), second(t.tm_sec)
{
}

std::string DateTime::toString() const
{
    return std::format("{:04d}-{:02d}-{:02} {:02d}:{:02d}:{:02d}", year, month, day, hour, minute, second);
}

TimeZone::TimeZone(int eastOfUtc, const char *tzname)
    : m_data(std::make_shared<TimeZone::Data>())
{
    m_data->addLocalTime(eastOfUtc, false, 0);
    m_data->abberviation = tzname;
}

TimeZone TimeZone::UTC()
{
    return TimeZone(0, "UTC");
}

TimeZone TimeZone::China()
{   
    // 等待实现
    return TimeZone();
}

TimeZone TimeZone::loadZoneFile(const std::string_view zoneFile)
{
    std::unique_ptr<Data> data = std::make_unique<Data>();
    if (!detail::readTimeZoneFile(zoneFile, data.get()))
    {
        data.reset();
    }

    return TimeZone(std::move(data));
}

DateTime TimeZone::toLocalTime(int64_t secondsSinceEpoch, int *utcOffset) const
{
    DateTime localTime;

    assert(m_data != nullptr);

    const Data::LocalTime *local = m_data->findLocalTime(secondsSinceEpoch);

    if (local)
    {
        localTime = detail::BreakTime(secondsSinceEpoch + local->utcOffset);
        if (utcOffset)
        {
            *utcOffset = local->utcOffset;
        }
    }

    return localTime;
}

int64_t TimeZone::fromLocalTime(const DateTime &dataTime, bool postTransition) const
{
    assert(m_data != nullptr);
    const Data::LocalTime* local = m_data->findLocalTime(dataTime, postTransition);

    const int64_t localSecond = fromUtcTime(dataTime);
    if (local)
    {
        return localSecond - local->utcOffset;
    }

    return localSecond;
}

DateTime TimeZone::toUtcTime(int64_t secondsSinceEpoch)
{
    return detail::BreakTime(secondsSinceEpoch);
}

int64_t TimeZone::fromUtcTime(const DateTime &dataTime)
{
    Date date(dataTime.year, dataTime.month, dataTime.day);
    int secondInDay = dataTime.hour * 3600 + dataTime.minute * 60 + dataTime.second;
    int64_t days = date.julianDayNumber() - Date::kJulianDayOff_1970_01_01;

    return days * kSecondsPerDay + secondInDay;
}

TimeZone::TimeZone(std::unique_ptr<Data> data)
    : m_data(std::move(data))
{
}



}