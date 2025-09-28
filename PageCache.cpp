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
    Span *PageCache::newSpanLockfree(size_t pageNum)
    {
        assert(pageNum > 0 && pageNum < PAGE_NUM);
        // 1.对应的SpanList中有Span
        if (!_spanLists[pageNum].IsEmpty())
        {
            return _spanLists[pageNum].GetFrontSpan();
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
    PageCache PageCache::_instance;
} // namespace Xten
