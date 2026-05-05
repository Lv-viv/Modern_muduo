#ifndef __MUDUO_BASE_NOCOPYABLE_H__
#define __MUDUO_BASE_NOCOPYABLE_H__

namespace muduo::base
{

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete; 
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}

#endif 