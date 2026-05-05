#ifndef __MUDUO_BASE_LOGFILE_H__
#define __MUDUO_BASE_LOGFILE_H__

#include "muduo/base/noncopyable.hpp"
#include "muduo/base/Types.hpp"
#include "muduo/base/Mutex.hpp"

#include <memory>

namespace muduo::base
{

namespace FileUtil
{

    class AppendFile;

}

class LogFile : public noncopyable
{
public:
    LogFile(std::string_view basename,
        off_t rollSize,
        bool threadSelf = true,
        int flushInterval = 3,
        int checkEveryN = 100);


    bool rollFile();
    void flush();
    void append(std::string_view logline);
    void append(const char* logline, int len);

private:

    static std::string getLogFileName(const std::string_view basename, time_t* now);

    void append_unliked(std::string_view logline);

    const std::string m_basename;
    const off_t m_rollSize;
    const int m_flushInterval;
    const int m_checkEveryN;
    
    int m_count;

    std::unique_ptr<MutexLock> m_mutex;
    time_t m_startOfPeriod;
    time_t m_lastRoll;
    time_t m_lastFlush;

    std::unique_ptr<FileUtil::AppendFile> m_file;
    static constexpr auto kRollPerSeconds = 60 * 60 * 24;
};


}

#endif