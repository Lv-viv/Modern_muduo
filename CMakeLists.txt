cmake_minimum_required(VERSION 3.5...4.2)

project(muduo-cpp20 C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE "Release")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if (CMAKE_PROJECT_NAME STREQUAL "muduo-cpp20")
    option(MUDUO_BUILD_EXAMPLES "build mudo examples" ON)
endif()

set(CXX_FLAGS
    -g
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -Wall
    -Wextra
    -Werror
    -Wconversion # 隐式类型转换
    -Wno-unused-parameter #未使用的参数
    -Wold-style-cast
    -Woverloaded-virtual # 当派生类中的函数声明隐藏了基类中的虚函数时，发出警告。
    -Wpointer-arith #警告那些对“大小未知”的指针进行的算术运算。
    -Wshadow  #你在内层作用域声明了一个变量，而它的名字与外层作用域（如全局变量、类成员变量、父类变量或上层局部变量）的变量名相同时，发出警告。
    # -Wwrite-strings # 让编译器将字符串字面量（如 "hello"）视为 const char[] 类型，而不是 char[] 类型。
    -march=native # 针对我当前正在编译的这台电脑的 CPU 架构，生成最高效、最快的代码
    -rdynamic # 将程序中的所有符号（函数名、全局变量名）导出到动态符号表中。
    #-Wno-array-bounds
    -Wno-format-truncation

    -ftrivial-auto-var-init=pattern
    -D_FORTIFY_SOURCE=3
    -fstack-clash-protection
    -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST
    )

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()


option(USE_OLD_LOG "Use old log Interface" OFF)

if (DEFINED USE_OLD_LOG)

add_definitions(-DUSE_OLD_LOG)

endif()

find_package(Boost REQUIRED)

find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

add_compile_options(${CXX_FLAGS})

add_subdirectory(muduo/base)
add_subdirectory(muduo/net)

