#include "ProcessInfo.hpp"
#include "muduo/base/FileUtil.hpp"
#include "muduo/base/CurrentThread.hpp"

#include <algorithm>
#include <format>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/times.h>

namespace muduo::base
{


namespace detail
{

thread_local int t_numOpenFiles = 0;
int fdDirFilter(const struct dirent* d)
{
    if (::isdigit(d->d_name[0]))
    {
        ++t_numOpenFiles;
    }
    return 0;
}

thread_local std::vector<pid_t>* t_pids = NULL;
int taskDirFilter(const struct dirent* d)
{
    if (::isdigit(d->d_name[0]))
    {
        t_pids->push_back(atoi(d->d_name));
    }
    return 0;
}

int scanDir(std::string_view dirpath, int (*fileter)(const struct dirent*))
{
    struct dirent** namelist = nullptr;
    int result = ::scandir(dirpath.data(), &namelist, fileter, alphasort);
    assert(namelist == nullptr);
    return result;
}

Timestamp g_startTime = Timestamp::now();
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));

}

namespace ProcessInfo
{

using namespace detail;

pid_t pid()
{
    return ::getpid();
}

std::string pidString()
{
    return std::format("{:d}", pid());
}

uid_t uid()
{
    return ::getuid();
}
std::string uidString()
{
    return std::format("{:d}", uid());
}

std::string username()
{
    struct passwd pwd {};
    struct passwd* result{nullptr};
    char buf[8196];
    const char* name = "unknowuser";

    ::getpwuid_r(uid(), &pwd, buf, sizeof(buf), &result);
    if (result)
    {
        name = pwd.pw_name;
    }

    return name;
}

uid_t euid()
{
    return ::geteuid();
}

Timestamp startTime()
{
    return g_startTime;
}

int clockTicksPerSecond()
{
    return g_clockTicks;
}

int pageSize()
{
    return g_pageSize;
}

bool isDebugBuild()
{

#if NDEBUG
    return false;
#else
    return true;
#endif

}

std::string hostname()
{
    char buf[64];
    if (::gethostname(buf, sizeof(buf)))
    {
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    }

    return "unknownhost";
}

std::string procStatus()
{
    std::string result;

    FileUtil::readFile("/proc/self/status", 65535, &result);
    return result;
}

std::string procStat()
{
    std::string result;

    FileUtil::readFile("/proc/self/stat", 65535, &result);
    return result;
}

StringPiece procname(std::string_view stat)
{
    StringPiece name;
    auto lp = stat.find('(');
    auto rp = stat.find(')');
    if (lp != std::string_view::npos && rp != std::string_view::npos && lp < rp)
    {
        name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
    }

    return name;
}

std::string procname()
{
    return procname(procStat()).as_string();
}

std::string threadStat()
{
    std::string path = std::format("/proc/self/task/{}/stat", CurrentThread::tid());
    std::string result;
    FileUtil::readFile(path, 65536, &result);
    return result;
}

std::string exePath()
{
    std::string result;
    char buf[1024];

    auto n = ::readlink("/proc/self/exe", buf, sizeof(buf));
    if (n > 0)
    {
        result.assign(buf, n);
    }

    return result;
}

int openedFiles()
{
    t_numOpenFiles = 0;
    scanDir("/proc/self/fd", fdDirFilter);
    return t_numOpenFiles;
}

int maxOpenFiles()
{
    struct rlimit rl;
    if (::getrlimit(RLIMIT_NOFILE, &rl))
    {
        return openedFiles();
    }

    return static_cast<int>(rl.rlim_cur);
}

CpuTime cpuTime()
{
    CpuTime t;
    struct tms tms;
    if (::times(&tms))
    {
        const double hz = static_cast<double>(clockTicksPerSecond());
        t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
        t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
    }

    return t;
}

int numThreads()
{
    int result = 0;
    std::string status = procStatus();
    auto pos = status.find("Threads:");
    if (pos != std::string::npos)
    {
        result = ::atoi(status.c_str() + pos + 8);
    }

    return result;
}

std::vector<pid_t> threads()
{
    std::vector<pid_t> result;
    t_pids = &result;
    scanDir("/proc/self/task", taskDirFilter);
    t_pids = nullptr;

    std::ranges::sort(result);

    return result;
}


}

} // namespace muduo::base::ProcessInfo
