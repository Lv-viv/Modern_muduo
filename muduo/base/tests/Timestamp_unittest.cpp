#include "muduo/base/Timestamp.hpp"
#include <vector>

#include <print>

using muduo::base::Timestamp;

void passByValue(Timestamp now)
{
    std::println("{}", now.toString());
}

void passByConstReference(const Timestamp& now)
{
    std::println("{}", now.toString());
}

void benchmark()
{
    const int kNumber = 1000 * 1000;
    std::vector<Timestamp> timestamps;
    timestamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i)
    {
        timestamps.push_back(Timestamp::now());
    }

    std::println("{}", timestamps.front().toString());
    std::println("{}", timestamps.back().toString());
    std::println("{}", muduo::base::timeDifference(timestamps.back(), timestamps.front()));

    int increments[100] = {0};
    int64_t start = timestamps.front().microSecondsSinceEpoch();
    for (int i = 0; i < kNumber; ++i)
    {
        int64_t next = timestamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;

        if (inc < 0)
        {
            std::println("reverse!");
        }
        else if (inc < 100)
        {
            ++increments[inc];
        }
        else
        {
            std::println("big gap {}", inc);
        }
    }

    for (int i = 0; i < 100; ++i)
    {
        std::println("{:2d}: {}", i, increments[i]);
    }
}

int main()
{
    Timestamp now(Timestamp::now());
    std::println("{}", now.toString());

    passByValue(now);
    passByConstReference(now);
    benchmark();

    return 0;
}