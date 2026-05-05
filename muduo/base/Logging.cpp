#include "muduo/base/Logging.hpp"
#include "Logging.hpp"

#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>

namespace muduo::base
{

thread_local time_t t_lastSecond;
thread_local char t_time[64];
thread_local char t_errorBuf[512];

const char *LogLevelName[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] =
{
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
};

Logger::LogLevel initLogLevel()
{
    if (::getenv("MUDUO_LOG_TRACE"))
    {
        return Logger::LogLevel::TRACE;
    }
    else if (::getenv("MUDUO_LOG_DEBUG"))
    {
        return Logger::LogLevel::DEBUG;
    }
    else
    {
        return Logger::LogLevel::INFO;
    }
}


Logger::LogLevel g_logLevel = initLogLevel();

void defaultOutput(const char* msg, int len)
{
    [[maybe_unused]] size_t n = fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
    fflush(stdout);
}


Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
TimeZone g_logTimeZone;

class T
{
public:
    T(const char *str, unsigned int len)
        : m_str(str), m_len(len)
    {
        assert(strlen(str) == len);
    }
    const char *m_str;
    const unsigned m_len;
};

const char *strerror_tl(int savedError)
{
    return strerror_r(savedError, t_errorBuf, sizeof(t_errorBuf));
}

inline LogStream &operator<<(LogStream &s, T v)
{
    s.append(v.m_str, v.m_len);
    return s;
}

inline LogStream &operator<<(LogStream &s, Logger::SourceFile &v)
{
    s.append(v.data, v.size);
    return s;
}

Logger::Impl::Impl(Logger::LogLevel level, int savedErrno, const Logger::SourceFile &file, int line)
    : m_time(Timestamp::now()), m_stream(), m_level(level), m_line(line), m_basename(file)
{
    formatTime();
    // 后续 线程相关

    m_stream << T(LogLevelName[static_cast<int>(level)], 6);
    if (savedErrno)
    {
        m_stream << strerror_tl(savedErrno) << " errno=(" << savedErrno << ")";
    }
}

Logger::Impl::Impl(LogLevel level, int savedErrno, std::source_location localtion)
    : m_time(Timestamp::now()), m_stream(), m_level(level), m_line(static_cast<int>(localtion.line())), m_basename(externBaseName(localtion.file_name()))
{
    formatTime();
    // 后续 线程相关

    m_stream << T(LogLevelName[static_cast<int>(level)], 6);
    if (savedErrno)
    {
        m_stream << strerror_tl(savedErrno) << " errno=(" << savedErrno << ")";
    }
}

std::string_view Logger::Impl::externBaseName(const std::string_view path)
{
    if (auto pos = path.find_last_of("/\\"); pos != std::string_view::npos)
    {
        return path.substr(0, pos);
    }

    return path;
}

void Logger::Impl::formatTime()
{
    int64_t microSecondSinceEpoch = m_time.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondSinceEpoch % Timestamp::kMicroSecondsPerSecond);

    if (seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct DateTime dt;
        if (g_logTimeZone.vaild())
        {
            dt = g_logTimeZone.toLocalTime(seconds);
        }
        else
        {
            dt = TimeZone::toUtcTime(seconds);
        }

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        assert(len == 17);
        (void)len;
    }

    if (g_logTimeZone.vaild())
    {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8);
        m_stream << T(t_time, 17) << T(us.data(), 8);
    }
    else
    {
        Fmt us(".%06dZ", microseconds);
        assert(us.length() == 9);
        m_stream << T(t_time, 17) << T(us.data(), 9);
    }
}

void Logger::Impl::finish()
{
    m_stream << " - " << m_basename << ':' << m_line << "\n";
}

Logger::Logger(SourceFile file, int line)
    : m_impl(LogLevel::INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : m_impl(level, 0, file, line)
{
}
Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : m_impl(level, 0, file, line)
{
    m_impl.m_stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : m_impl(toAbort ? LogLevel::FATAL : LogLevel::ERROR, errno, file, line)
{
}

Logger::Logger(std::source_location localtion)
    : m_impl(LogLevel::INFO, 0, localtion)
{
}

Logger::Logger(LogLevel level, std::source_location localtion)
    : m_impl(level, 0, localtion)
{
}

Logger::Logger(LogLevel level, const char *func, std::source_location localtion)
    : m_impl(level, 0, localtion)
{
    m_impl.m_stream << func << ' ';
}

Logger::Logger(bool toAbort, std::source_location localtion)
    : m_impl(toAbort ? LogLevel::FATAL : LogLevel::ERROR, errno, localtion)
{
}

Logger::~Logger()
{
    m_impl.finish();
    const LogStream::Buffer& buff(stream().buffer());
    g_output(buff.data(), buff.length());
    if (m_impl.m_level == LogLevel::FATAL)
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    g_logLevel = level;
}

Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

void Logger::setOutput(OutputFunc func)
{
    g_output = func;
}

void Logger::setFlush(FlushFunc func)
{
    g_flush = func;
}

void Logger::setTimeZone(const TimeZone &tz)
{
    g_logTimeZone = tz;
}
}
