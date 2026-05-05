#include "muduo/base/LogFile.hpp"

#include "muduo/base/FileUtil.hpp"


#include <assert.h>
#include <time.h>
#include "LogFile.hpp"


namespace muduo::base
{

LogFile::LogFile(std::string_view basename, off_t rollSize, [[maybe_unused]] bool threadSelf, int flushInterval, int checkEveryN)
    : m_basename(basename)
    , m_rollSize(rollSize)
    , m_flushInterval(flushInterval)
    , m_checkEveryN(checkEveryN)
    , m_count(0)
    , m_mutex(threadSelf ? std::make_unique<MutexLock>() : nullptr)
    , m_startOfPeriod(0)
    , m_lastRoll(0)
    , m_lastFlush(0)
{ 
    assert(basename.find('/') != std::string_view::npos);

}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(m_basename, &now);
    time_t start = now / kRollPerSeconds * kRollPerSeconds;

    if (now > m_lastRoll)
    {
        m_lastRoll = now;
        m_lastFlush = now;
        m_startOfPeriod = start;
        m_file.reset(new FileUtil::AppendFile(filename));
        return true;
    }

    return false;
}

void LogFile::flush()
{
    if (m_mutex)
    {
        MutexLockGuard lock(*m_mutex);
        m_file->flush();
    }
    else
    {
        m_file->flush();
    }
}

void LogFile::append(std::string_view logline)
{
    if (m_mutex)
    {
        MutexLockGuard lock(*m_mutex);
        append_unliked(logline);
    }
    else
    {
        append_unliked(logline);
    }
}

void LogFile::append(const char *logline, int len)
{
    append(std::string_view(logline, len));
}

std::string LogFile::getLogFileName(const std::string_view basename, time_t *now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(nullptr);
    gmtime_r(now, &tm);
    strptime(timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    //filename += 

    filename += ".log";

    return filename;
}

void LogFile::append_unliked(std::string_view logline)
{
    m_file->append(logline.data(), logline.size());

    if (m_file->writtenBytes() > m_rollSize)
    {
        rollFile();
    }
    else
    {
        ++m_count;
        if (m_count >= m_checkEveryN)
        {
            m_count = 0;
            time_t now = ::time(nullptr);
            time_t thisPeriod = now / kRollPerSeconds * kRollPerSeconds;
            if (thisPeriod != m_startOfPeriod)
            {
                rollFile();
            } 
            else if (now - m_lastFlush > m_flushInterval)
            {
                m_lastFlush = now;
                m_file->flush();
            }
        }
    }
}


}
