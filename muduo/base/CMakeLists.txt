set(base_SRC
    Logging.cpp
    TimeZone.cpp
    Date.cpp
    Timestamp.cpp
    LogStream.cpp
    FileUtil.cpp
    LogFile.cpp
    CurrentThread.cpp
    Thread.cpp
    Condition.cpp
    ProcessInfo.cpp
    CountDownLatch.cpp
    )

add_library(muduo_base ${base_SRC})

target_link_libraries(muduo_base pthread rt)

file(GLOB HEADERS "*.hpp")

#if (MUDUO_BUILD_EXAMPLES)
    add_subdirectory(tests)
#endif()