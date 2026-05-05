#ifndef __MUDUO_BASE_COPYABLE_H__
#define __MUDUO_BASE_COPYABLE_H__

namespace muduo::base
{
/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
public:
    copyable() = default;
    ~copyable() = default;
};

}
#endif