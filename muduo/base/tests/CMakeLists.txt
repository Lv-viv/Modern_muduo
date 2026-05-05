add_executable(test test.cpp)
target_link_libraries(test muduo_base)

add_executable(date_unittest Date_unittest.cpp)
target_link_libraries(date_unittest muduo_base)

add_executable(timezone_util TimeZone_util.cpp)
target_link_libraries(timezone_util muduo_base)

add_executable(timezone_untiltest TimeZone_untiltest.cpp)
target_link_libraries(timezone_untiltest muduo_base)

add_executable(timestamp_unittest Timestamp_unittest.cpp)
target_link_libraries(timestamp_unittest muduo_base)

if (BOOSTTEST_LIBRARY)
add_executable(logstream_test LogStream_test.cpp)
target_link_libraries(logstream_test muduo_base boost_unit_test_framework)
endif()