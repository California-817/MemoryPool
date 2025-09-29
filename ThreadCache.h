#ifndef __XTEN_THREADCACHE_H__
#define __XTEN_THREADCACHE_H__
#include "nocopyable.h"
#include <memory>
namespace Xten
{
// 空闲链表个数
#define FREE_LISTS_NUM 208
    // 空闲链表结构----不存在真正的链表节点结构，空间就是节点，就能存储下一个节点地址
    class FreeList
    {
        NO_COPYABLE_MACRO(FreeList);

    public:
        FreeList();
        ~FreeList();
        // 头插内存块
        void Push(void *obj);
        // 头插一定范围内存块,n表示内存块个数
        void PushRange(void *begin, void *end, size_t n);
        // 头删一定范围内存块,n表示删除内存块个数
        void PopRange(void*& begin,void*& end,size_t n);
        // 头删内存块
        void *Pop();
        // 是否为空
        bool IsEmpty();
        // 获取当前内存块个数
        size_t GetCurSize() const { return _curSize; }
        // 返回向cc获取内存块数量的接口
        size_t &GetMaxSize() { return _maxSize; }

    private:
        void *_freeList; // 链表连起来的空间不一定是连续的
        size_t _maxSize; // 单次向cc申请内存块的个数
        size_t _curSize; // 当前的FreeList内存块个数
    };
    // 线程独占的threadCache结构
    class ThreadCache
    {
        NO_COPYABLE_MACRO(ThreadCache);

    public:
        typedef std::shared_ptr<ThreadCache> ptr;
        // 获取线程局部存储的ThreadCache对象
        static ThreadCache *GetThreadCache();
        ~ThreadCache();
        // 获取空间--任意Size
        void *Allocate(size_t size);
        // 释放空间--任意Size(作用是进行查找hash中FreeList的下标)
        void Deallocate(void *ptr, size_t size);

    private:
        ThreadCache() = default;
        // 向centralCache获取内存空间  index是hash下标,alignSize是对齐后大小
        void *fetchFromCentralCache(size_t index, size_t alignSize);

    private:
        // [1-128] 8B对齐        -------  [0,15] ------16
        // [128+1-1024] 16B对齐  ----------- [16,71] ------56
        // [1024+1-8*1024] 128B对齐 ---------[72,127] ------56
        // [8*1024+1-64*1024] 1024B对齐 ------[128,183]  -------56
        // [64*1024+1-256*1024] 8*1024B对齐 ------[184,207] ------24
        FreeList _freeLists[FREE_LISTS_NUM]; // 通过一个hash结构管理不同大小内存的空闲链表----会有的内部碎片问题
    };
} // namespace Xten

#endif