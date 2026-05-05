#include "muduo/base/TimeZone.hpp"

#include <print>

using namespace muduo::base;

void printUTCAndLocal(int64_t utc, TimeZone local)
{   
    std::println("Unix Time: {}", utc);
    std::println("UTC:       {}", TimeZone::toUtcTime(utc).toString());
    int utcOffset = 0;
    std::print("Local:     {}", local.toLocalTime(utc, &utcOffset).toString());
    std::println(" {:+03d}{:02d}", utcOffset / 3600, utcOffset % 3600 / 60);

}

int main(int argc, char* argv[])
{
    TimeZone local = TimeZone::loadZoneFile("/etc/localtime");
    if (argc <= 1)
    {
        auto now = time(nullptr);
        printUTCAndLocal(now, local);
        return 0;
    }

    for (int i = 0; i < argc; ++i)
    {
        char* end = nullptr;
        int64_t t = strtol(argv[i], &end, 10); // ./timezone_test 1698192000
        if (end > argv[i] && *end == '\0')
        {
            printUTCAndLocal(t, local);
        }
        else
        {
            struct tm tm{};
            end = strptime(argv[i], "%F %T", &tm); // ./timezone_test "2023-10-25 15:30:00"
            if (end != nullptr && *end == '\0')
            {
                DateTime dt(tm);
                t = local.fromLocalTime(dt);
                printUTCAndLocal(t, local);
            }
        }
    }


    return 0;
}