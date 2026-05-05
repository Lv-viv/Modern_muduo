#ifndef __MUDUO_BASE_PROCESSINFO_H__
#define __MUDUO_BASE_PROCESSINFO_H__

#include "muduo/base/StringPiech.hpp"
#include "muduo/base/Types.hpp"
#include "muduo/base/Timestamp.hpp"
#include "muduo/base/Atomic.hpp"

#include <string_view>
#include <vector>
#include <sys/types.h>

namespace muduo::base::ProcessInfo
{

pid_t pid();
std::string pidString();

uid_t uid();
std::string uidString();

std::string username();

uid_t euid();

Timestamp startTime();

int clockTicksPerSecond();

int pageSize();

bool isDebugBuild();

std::string hostname();

std::string procStatus();
std::string procStat();

StringPiece procname(std::string_view stat);

std::string procname();

std::string threadStat();

std::string exePath();

int openedFiles();
int maxOpenFiles();


struct CpuTime
{
    double userSeconds = 0.0;
    double systemSeconds = 0.0;

    double total() const { return userSeconds + systemSeconds; };
};


CpuTime cpuTime();

int numThreads();

std::vector<pid_t> threads();

} // namespace muduo::base::ProcessInfo



#endif