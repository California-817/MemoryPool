#include "CentralCache.h"
#include "MPutil.hpp"
namespace Xten
{
    size_t CentralCache::fetchRangeMemory2TC(void *&begin, void *&end, size_t batchNum, size_t size)
    {
        size_t index = MemoryPoolUtil::FindIndex(size);
        // 多线程的tc对cc的同一个SpanList访问需要加锁
        _spanLists[index].GetMutex().lock();
        // 根据index找到指定空间块大小的SpanList  有三种情况  [ 1.有span有空间 2.有span无空间 3.无span无空间 ]
        Span *span = GetOneSpan(_spanLists[index], size);
        assert(span && span->list);
        // 1.从这个Span中获取batchNum个size大小的内存块,不足则有多少取多少
        begin = span->list;
        end = span->list;
        size_t realBatchNum = 1;
        while (*((void **)end) != nullptr && (realBatchNum < batchNum))
        {
            // 后面还有空间 && 没达到batchNum块数的要求
            end = (void *)(*((void **)end));
            realBatchNum++;
        }
        // 达到了申请空间块数或者空间块数不足了
        span->list = (void *)(*((void **)end)); // 更新span管理的链表空闲空间
        *((void **)end) = nullptr;  //断开分配给tc的空闲块链表空间与span中空间的联系
        _spanLists[index].GetMutex().unlock();
        return realBatchNum;
    }
    // 获取一个管理空间不为空的Span
    Span *CentralCache::GetOneSpan(SpanList &list, size_t size)
    {
        // todo
        return nullptr;
    }
    CentralCache CentralCache::_instance; // 类内static变量在main之前初始化，存放在data段
} // namespace Xten
