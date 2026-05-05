#ifndef __MUDUO_BASE_FILEUTIL_H__
#define __MUDUO_BASE_FILEUTIL_H__


#include "muduo/base/noncopyable.hpp"
#include "muduo/base/StringPiech.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <unistd.h>

namespace muduo::base
{

namespace FileUtil
{
// read small file < 64KB

template<typename T>
concept IsString = requires(T v)
{
    v.size();
    v.append("", 0);
    v.reserve();
};

class ReadSmallFile : public noncopyable
{
public:
    ReadSmallFile(StringArg filename);
    ~ReadSmallFile();

    template <IsString String>
    int readToString(int maxSize, String* content, int64_t* fileSize, int64_t* modifyTime, int64_t* createTime);

    int readToBuff(int* size);

    const char* buffer() const { return m_buff; }


private:
    static constexpr int kBuffSize = 64 * 1024;

    int m_fd;
    int m_err;
    char m_buff[kBuffSize];
};

template <IsString String>
inline int ReadSmallFile::readToString(int maxSize, String *content, int64_t* fileSize, int64_t* modifyTime, int64_t* createTime)
{
    static_assert(sizeof(off_t) == 8, "__FILE_OFFSET_BITS=64");
    assert(content != nullptr);

    int err = m_err;
    if (m_fd >= 0)
    {
        content->clear();
        if(fileSize)
        {
            struct stat statbuf;
            if (::fstat(m_fd, &statbuf) == 0)
            {   
                // 常规设备
                if (S_ISREG(statbuf.st_mode))
                {
                    *fileSize = static_cast<int64_t>(statbuf.st_size);
                    content->reserve(std::min(implicit_cast<int64_t>(maxSize), *fileSize));
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    err = EISDIR;
                }

                if (modifyTime)
                {
                    *modifyTime = static_cast<int64_t>(statbuf.st_mtime);
                }

                if (createTime)
                {
                    *createTime = static_cast<int64_t>(statbuf.st_ctime);
                }
            }
            else
            {
                err = errno;
            }
        }

        while (content->size() < implicit_cast<size_t>(maxSize))
        {
            size_t toRead = std::min(implicit_cast<size_t>(maxSize - content->size()), sizeof(m_buff));
            ssize_t n = ::read(m_fd, m_buff,toRead);
            if (n > 0)
            {
                content->append(m_buff, n);
            }
            else
            {
                if (n < 0)
                {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

template <IsString String>
inline int readFile(const StringArg &filename, 
                    int maxSize, 
                    String *content, 
                    int64_t *fileSize = nullptr,
                    int64_t *modifyTime = nullptr, 
                    int64_t *createTime = nullptr)
{
    ReadSmallFile file(filename);
    return  file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

class AppendFile : public noncopyable
{
public:
    explicit AppendFile(StringArg filename);
    ~AppendFile();

    void append(const char* logline, const size_t len);

    void flush();

    off_t writtenBytes() const { return m_writtenBytes; }

private:
    size_t write(const char* logline, const size_t len);

    FILE* m_fp;
    char m_buffer[64 * 1024];
    off_t m_writtenBytes;
};


}
}


#endif