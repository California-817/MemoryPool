#include "CentralCache.h"
#include "MPutil.hpp"
#include "PageCache.h"
namespace Xten
{
    size_t CentralCache::fetchRangeMemory2TC(void *&begin, void *&end, size_t batchNum, size_t size)
    {
        size_t index = MemoryPoolUtil::FindIndex(size);
        // 多线程的tc对cc的同一个SpanList访问需要加锁
        _spanLists[index].GetMutex().lock();
        // 根据index找到指定空间块大小的SpanList  有三种情况  [ 1.有span有空间 2.有span无空间 3.无span无空间 ]
        Span *span = GetOneSpan(_spanLists[index], size); // 获取一个有空闲空间span结构
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
        span->usedCount += realBatchNum;        // 更新该span分配给tc使用的空间块数量
        *((void **)end) = nullptr;              // 断开分配给tc的空闲块链表空间与span中空间的联系
        _spanLists[index].GetMutex().unlock();
        return realBatchNum;
    }
    // 从tc中回收一定范围的空间----这些内存块一定在同一个SpanList中,但是不一定会在同一个Span中
    void CentralCache::RecycleRangeMemoryFromTC(void *begin,size_t size)
    {
        size_t index=MemoryPoolUtil::FindIndex(size);
        //多tc回收并发访问这个接口 需要加锁
        _spanLists[index].GetMutex().lock();
        //通过块地址确定内存块所在页的PageId----从而确定所属Span
        while(begin)
        {
            void* next=*((void**)begin);
            Span* pspan=PageCache::GetInstance()->MemoryPtr2Span(begin); //一定在index下标处的Spanlist中
            //插回Span中
            *((void**)begin)=pspan->list;
            pspan->list=begin;
            //回收一个内存块,Span对应的usedCount--
            pspan->usedCount--;
            if(pspan->usedCount==0)
            {
                //从CC的Spanlist中删除----一个span只能在cc或者pc的spanlist中 不能同时存在
                _spanLists[index].Erase(pspan);
                pspan->list=nullptr;
                pspan->next=nullptr;
                pspan->prev=nullptr;
                //说明这个Span所管理的所有页空间都是空闲的,可以交给PageCache合并成更大的页------减少外部碎片
                //此时这个Span已经和CentralCache的该Spanlist脱离了---释放锁
                _spanLists[index].GetMutex().unlock();
                PageCache::GetInstance()->RecycleFreeSpanFromCC(pspan);
                //归还后重新加锁
                _spanLists[index].GetMutex().lock();
            }
            begin=next;
        }
        _spanLists[index].GetMutex().unlock();
    }
    // 获取一个管理空间不为空的Span
    Span *CentralCache::GetOneSpan(SpanList &list, size_t size)
    {
        // 先看该spanlist有没有可用span,如果没有则去PageCache中获取
        Span *cur = list.Begin();
        Span *end = list.End();
        while (cur != end)
        {
            if (cur->list)
                return cur;
            cur = cur->next;
        }

        // 将cc的某个SpanList的锁解掉----使得在向PageCache获取Span时,这个SpanList可以被其他线程访问
        list.GetMutex().unlock();
        //======================================================================================
        //========== 下面这一段代码不涉及到对CentraCache中的某个SpanList操作的代码==================

        // 没有找到可用的span [ 1.有Span无free空间 2.无Span ] ---去PageCache中获取
        Span *newSpan = PageCache::GetInstance()->NewSpan(MemoryPoolUtil::Size2Page(size));
        // 注意：这个newSpan还没有进行内存块大小划分
        // 1.进行内存划分
        char *begin = (char *)(newSpan->pageId << PAGE_SHIFT);
        char *tail = (char *)(begin + (newSpan->pageCount << PAGE_SHIFT));
        newSpan->list = (void *)begin;
        newSpan->objSize=size; //记录该span划分空间大小
        char *next = begin + size;
        while (next < tail)
        {
            *((void **)begin) = next;
            begin = next;
            next = next + size;
        }
        *((void **)begin) = nullptr; // 最后一个内存块的next=nullptr
        //======================================================================================
        list.GetMutex().lock(); // 重新加锁

        // 2.将这个newSpan链接到这个centralCache对应hash桶的Spanlist上
        list.PushFront(newSpan);
        return newSpan;
    }
    CentralCache CentralCache::_instance; // 类内static变量在main之前初始化，存放在data段
} // namespace Xten
