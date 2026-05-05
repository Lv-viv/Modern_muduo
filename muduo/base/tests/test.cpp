#include <iostream>
#include <print>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "muduo/base/ProcessInfo.hpp"

std::string username()
{
    struct passwd pwd {};
    struct passwd* result{nullptr};
    char buf[8196];
    const char* name = "unknowuser";

    ::getpwuid_r(::getuid(), &pwd, buf, sizeof(buf), &result);
    if (result)
    {
        name = pwd.pw_name;
    }

    return name;
}

int main()
{
    std::println("{}", username());

    // for (auto id : muduo::base::ProcessInfo::threads())
    // {
    //     std::println("{}", id);
    // }

}