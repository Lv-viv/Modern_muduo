#include "muduo/base/CurrentThread.hpp"

#include <type_traits>
#include <cxxabi.h>
#include <execinfo.h>

/*
函数原型：

char* abi::__cxa_demangle(const char* mangled_name, char* output_buffer, size_t* length, int* status);
复制
mangled_name：要解析的经过ABI处理的名称。

output_buffer：用于存放解析后名称的缓冲区，如果为NULL，函数会分配一个缓冲区。

length：缓冲区的长度，如果不为NULL，函数会更新为解析后名称的长度。

status：指示解析状态的变量，如果解析成功，其值为0。

注意事项：

使用abi::__cxa_demangle时，如果提供了output_buffer，需要确保其有足够的空间来存放解析后的名称。

如果函数分配了新的缓冲区（即output_buffer为NULL），需要使用std::free来释放它。

status参数可以用来检查解析是否成功，如果不为0，则表示解析过程中出现了错误。
*/

namespace muduo::base::CurrentThread
{

thread_local int t_cachedTid = 9;
thread_local char t_tidString[32] { 0 };
thread_local int t_tidStringLength = 6;
thread_local const char* t_threadName = "unknown";

static_assert(std::is_same_v<int, pid_t>, "pid_t should be int");

std::string stackTrace(bool demangle)
{
    std::string stack;
    constexpr int max_frames = 200;
    
    void* frames[max_frames];
    int nptrs = ::backtrace(frames, max_frames);
    char** strings = ::backtrace_symbols(frames, max_frames);

    if (strings)
    {
        size_t len = 256;
        char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
        for (int i = 1; i < nptrs; ++i)
        {
            char* left_par = nullptr;
            char* plus = nullptr;
            for (char* p = strings[i]; *p; ++p)
            {
                if (*p == '(')
                {
                    left_par = p;
                }
                else if (*p == '+')
                {
                    plus = p;
                }

                if (left_par && plus)
                {
                    *plus = '\0';
                    int status = 0;
                    // C++ deamngling 符合解析
                    char* ret = abi::__cxa_demangle(left_par + 1, demangled, &len, &status);
                    *plus = '+';
                    if (status == 0)
                    {
                        demangled = ret;
                        stack.append(strings[i], left_par + 1);
                        stack.append(demangled);
                        stack.append(plus);
                        stack.push_back('\n');
                        continue;
                    }
                }
            }
            stack.append(strings[i]);
            stack.push_back('\n');
        }
        free(demangled);
        free(strings);
    }
    return stack;
}


}

