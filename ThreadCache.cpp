#include "ThreadCache.h"
#include "MPutil.hpp"
#include <iostream>
#include "CentralCache.h"
namespace Xten
{
    // 线程局部存储保证每个线程都有一个threadCache
    static thread_local ThreadCache *t_threadCache = nullptr;
    FreeList::FreeList()
        : _freeList(nullptr)
    {
    }
    FreeList::~FreeList()
    {
        std::cout << "FreeList::~FreeList()" << std::endl;
    }
    // 头插内存块
    void FreeList::Push(void *obj)
    {
        *((void **)obj) = _freeList;
        _freeList = obj;
    }
    // 头插一定范围内存块
    void FreeList::PushRange(void *begin, void *end)
    {
        assert(begin && end);
        *((void **)end) = _freeList;
        _freeList = begin;
    }
    // 头删内存块
    void *FreeList::Pop()
    {
        void *ret = nullptr;
        if (_freeList)
        {
            ret = _freeList;
            _freeList = *((void **)_freeList);
        }
        return ret;
    }
    bool FreeList::IsEmpty()
    {
        return !_freeList;
    }

    ThreadCache::~ThreadCache()
    {
        std::cout << "ThreadCache::~ThreadCache()" << std::endl;
    }
    // 获取空间--任意Size
    void *ThreadCache::Allocate(size_t size)
    {
        assert(0 < size && size <= MAX_ALLOC_SIZE);
        // 1.大小对齐 例如 20B--->24B   产生内部碎片
        size_t alignSize = MemoryPoolUtil::RoundUp(size);
        // 2.查找大小对应的hash表下标对应的FreeList
        size_t index = MemoryPoolUtil::FindIndex(size);
        // 3.直接去这个index对应的FreeList获取内存即可
        if (!_freeLists[index].IsEmpty())
        {
            return _freeLists[index].Pop();
        }
        // FreeList没有内存块----去centralCache获取
        return fetchFromCentralCache(index, alignSize);
    }
    // 释放空间
    void ThreadCache::Deallocate(void *ptr, size_t size)
    {
        assert(ptr && 0 < size && size <= MAX_ALLOC_SIZE);
        size_t index = MemoryPoolUtil::FindIndex(size);
        _freeLists[index].Push(ptr);
        // 将空间还给tc后，其实还需要将tc中的空闲空间还给cc
        // todo
    }
    // 从CentralCache获取空间
    void *ThreadCache::fetchFromCentralCache(size_t index, size_t alignSize)
    {
        // 1.根据一个慢启动调节算法计算要获取多少块空间
        size_t batchNum = std::min(_freeLists[index].GetMaxSize(), MemoryPoolUtil::Size2Num(alignSize));
        if (batchNum != MemoryPoolUtil::Size2Num(alignSize))
        {
            // 该freeList单次申请内存块没有达到上限
            _freeLists[index].GetMaxSize()++;
        }
        // 2.真正调用CentralCache的接口进行获取空间
        void *begin = nullptr; // 输出型参数
        void *end = nullptr;
        size_t realBatchNum = CentralCache::GetInstance()->fetchRangeMemory2TC(begin, end, batchNum, alignSize);
        assert(realBatchNum >= 1);
        if (realBatchNum == 1)
        {
            // 只分配一块直接返回给用户层,不需要进行freeList的头插内存块
            assert(begin == end);
        }
        else
        {
            // 实际分配块数大于1块,需要头插剩余内存块
            _freeLists[index].PushRange((void *)(*((void **)begin)), end);
        }
        *((void**)begin)=nullptr;//防止外部拿到这个地址进行非法内存访问
        return begin;
    }
    ThreadCache *ThreadCache::GetThreadCache()
    {
        if (t_threadCache)
        {
            return t_threadCache;
        }
        t_threadCache = new ThreadCache();
        return t_threadCache;
    }
} // namespace Xten
