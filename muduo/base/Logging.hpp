#ifndef __MUDUO_BASE_LOGGING_H__
#define __MUDUO_BASE_LOGGING_H__

#include "muduo/base/TimeZone.hpp"
#include "muduo/base/Timestamp.hpp"
#include "muduo/base/LogStream.hpp"


#include <string_view>
#include <source_location>

namespace muduo::base
{


class Logger
{
public:
    enum class LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };


    class SourceFile
    {
    public:
        SourceFile(std::string_view path)
        : data(path.data()) , size(static_cast<int>(path.size()))
        {
            auto pos = path.find_last_of('/');
            if (pos != std::string_view::npos)
            {
                data = path.substr(pos +1).data();
                size = static_cast<int>(path.substr(pos +1).size());
            }
        }

        const char* data;
        int size;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);

    explicit Logger(std::source_location localtion = std::source_location::current());
    Logger(LogLevel level, std::source_location localtion = std::source_location::current());
    Logger(LogLevel level, const char* func, std::source_location localtion = std::source_location::current());
    Logger(bool toAbort, std::source_location localtion = std::source_location::current());

    ~Logger();

    LogStream& stream() { return m_impl.m_stream; }

    static void setLogLevel(LogLevel level);
    static LogLevel logLevel();

    using OutputFunc = void(*)(const char* msg, int line);
    using FlushFunc = void(*)();

    static void setOutput(OutputFunc func);
    static void setFlush(FlushFunc func);
    static void setTimeZone(const TimeZone& tz);

private:

    struct Impl
    {
        using LogLevel = Logger::LogLevel;
        Impl() = delete;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        Impl(LogLevel level, int old_errno, std::source_location localtion = std::source_location::current());

        static std::string_view externBaseName(const std::string_view path);

        void formatTime();
        void finish();

        Timestamp m_time;
        LogStream m_stream;
        LogLevel m_level;
        int m_line;
        SourceFile m_basename;
    };

    Impl m_impl;

};

#if defined(USE_OLD_LOG)

#define LOG_TRACE if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::TRACE)  \
    muduo::base::Logger(__FILE__, __LINE__, muduo::base::Logger::LogLevel::TRACE, __func__).stream()

#define LOG_DEBUG if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::DEBUG)  \
    muduo::base::Logger(__FILE__, __LINE__, muduo::base::Logger::LogLevel::DEBUG, __func__).stream()

#define LOG_INFO if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::INFO)  \
    muduo::base::Logger(__FILE__, __LINE__, muduo::base::Logger::LogLevel::INFO, __func__).stream()


#define LOG_WARN muduo::base::Logger(__FILE__, __LINE__, muduo::base::Logger::LogLevel::WARN).stream()
#define LOG_ERROR muduo::base::Logger(__FILE__, __LINE__, muduo::base::Logger::LogLevel::ERROR).stream()
#define LOG_FATAL muduo::base::Logger(__FILE__, __LINE__,  muduo::base::Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR muduo::base::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::base::Logger(__FILE__, __LINE__, true).stream()

#else

#define LOG_TRACE if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::TRACE)  \
    muduo::base::Logger(muduo::base::Logger::LogLevel::TRACE).stream()

#define LOG_DEBUG if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::DEBUG)  \
    muduo::base::Logger(muduo::base::Logger::LogLevel::DEBUG).stream()

#define LOG_INFO if (muduo::base::Logger::logLevel() <= muduo::base::Logger::LogLevel::INFO)  \
    muduo::base::Logger(muduo::base::Logger::LogLevel::INFO).stream()

#define LOG_WARN muduo::base::Logger(muduo::base::Logger::LogLevel::WARN).stream()
#define LOG_ERROR muduo::base::Logger(muduo::base::Logger::LogLevel::ERROR).stream()
#define LOG_FATAL muduo::base::Logger(muduo::base::Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR muduo::base::Logger(false).stream()
#define LOG_SYSFATAL muduo::base::Logger(true).stream()

#endif

const char *strerror_tl(int savedError);


template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* name, T* ptr)
{
    if (ptr == nullptr)
    {
        Logger(file, line, Logger::LogLevel::FATAL).stream() << name;
    }
    return ptr;
}

template <typename T>
T* CheckNotNull(const char* name, T* ptr, std::source_location localtion = std::source_location::current())
{
    if (ptr == nullptr)
    {
        Logger(Logger::LogLevel::FATAL, localtion).stream() << name;
    }
    return ptr;
}

#if defined(USE_OLD_LOG)

#define CHECK_NOTNULL(val)  \
    ::muduo::base::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

#else

#define CHECK_NOTNULL(val)  \
    ::muduo::base::CheckNotNull("'" #val "' Must be non NULL", (val))

#endif

}

#endif