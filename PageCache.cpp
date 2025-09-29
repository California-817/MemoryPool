#include "PageCache.h"
#include "MPutil.hpp"
namespace Xten
{

    // 获取指定页数的Span------全新未进行内存块划分----多线程调用
    Span *PageCache::NewSpan(size_t pageNum)
    {
        Span *newspan = nullptr;
        _mtx.lock();
        newspan = newSpanLockfree(pageNum);
        _mtx.unlock();
        return newspan;
    }
    // 获取指定内存块所属的Span指针
    Span *PageCache::MemoryPtr2Span(void *ptr)
    {
        PageID pgid = ((PageID)ptr >> PAGE_SHIFT);
        auto iter = _PageId2Span.find(pgid);
        if (iter == _PageId2Span.end())
        {
            assert(false);
            return nullptr;
        }
        return iter->second;
    }
    Span *PageCache::newSpanLockfree(size_t pageNum)
    {
        assert(pageNum > 0 && pageNum < PAGE_NUM);
        // 1.对应的SpanList中有Span
        if (!_spanLists[pageNum].IsEmpty())
        {
            Span *ret = _spanLists[pageNum].GetFrontSpan();
            // 建立PageId与Span指针的映射关系
            for (PageID i = 0; i < ret->pageCount; i++)
            {
                _PageId2Span[ret->pageId + i] = ret;
            }
            return ret;
        }
        // 2.去页数更多的SpanList中查找Span并进行分裂
        for (int i = pageNum + 1; i < PAGE_NUM; i++)
        {
            if (_spanLists[i].IsEmpty())
                continue;
            // 找到一个非空的页数更多的spanlist
            Span *nspan = _spanLists[i].GetFrontSpan();
            // 进行分裂成2个Span
            Span *mspan = new Span();
            nspan->pageCount = pageNum;
            mspan->pageId = (nspan->pageId + pageNum);
            mspan->pageCount = i - pageNum;
            // 将另一个放入i-pageNum页数链表中
            _spanLists[i - pageNum].PushFront(mspan);
            for (PageID i = 0; i < nspan->pageCount; i++)
            {
                _PageId2Span[nspan->pageId + i] = nspan;
            }
            return nspan;
        }
        // 3.调用系统调用接口进行获取Span----直接申请128页的空间并创建Span
        void *ptr = MemoryPoolUtil::SystemCallMemory((PAGE_NUM - 1) << PAGE_SHIFT);
        assert(ptr);
        Span *bigSpan = new Span();
        bigSpan->pageId = ((size_t)ptr >> PAGE_SHIFT);
        bigSpan->pageCount = PAGE_NUM - 1;
        _spanLists[PAGE_NUM - 1].PushFront(bigSpan);
        return newSpanLockfree(pageNum); // 递归调用 此时一定能走到情况2
    }
    // 从CentralCache中回收空闲Span并合并成更大页空间的Span
    void PageCache::RecycleFreeSpanFromCC(Span *span)
    {
        //todo
    }
    PageCache PageCache::_instance;
} // namespace Xten
