#include "muduo/base/FileUtil.hpp"
#include "muduo/base/Logging.hpp"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include <string_view>

namespace muduo::base
{

namespace FileUtil
{

ReadSmallFile::ReadSmallFile(StringArg filename)
    : m_fd(::open(filename.c_str(), O_RDONLY|O_CLOEXEC))
    , m_err(0)
{
    m_buff[0] = '\0';
    if (m_fd < 0)
    {
        m_err = errno;
    }
}

ReadSmallFile::~ReadSmallFile()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
    }
}

int ReadSmallFile::readToBuff(int *size)
{
    int err = m_err;
    if (m_fd >= 0)
    {
        ssize_t n = ::pread(m_fd, m_buff, sizeof(m_buff) - 1, 0);
        if (n >= 0)
        {
            if (size)
            {
                *size = static_cast<int>(n);
            }
            m_buff[n] = '\0';
        }
        else
        {
            err = errno;
        }
    }

    return err;
}

AppendFile::AppendFile(StringArg filename)
    : m_fp(::fopen(filename.c_str(), "ae"))
    , m_writtenBytes(0)
{
    assert(m_fp);
    ::setbuffer(m_fp, m_buffer, sizeof(m_buffer));
}

AppendFile::~AppendFile()
{
    if (m_fp)
    {
        ::fclose(m_fp);
        m_fp = nullptr;
    }
}

void AppendFile::append(const char *logline, const size_t len)
{
    size_t written = 0;

    while(written != len)
    {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain)
        {
            int err = ferror(m_fp);
            if (err)
            {
                fprintf(m_fp, "AppendFile::append Faild %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }
    m_writtenBytes += written;
}

void AppendFile::flush()
{
    ::fflush(m_fp);
}

size_t AppendFile::write(const char *logline, const size_t len)
{
    return ::fwrite_unlocked(logline, 1, len, m_fp);
}

}
}
