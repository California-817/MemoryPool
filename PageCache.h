#ifndef __XTEN_PAGECACHE_H__
#define __XTEN_PAGECACHE_H__
#include<mutex>
#include<unordered_map>
#include "nocopyable.h"
#include "Span.h"
namespace Xten
{
#define PAGE_NUM 128+1
    class PageCache
    {
    public:
        NO_COPYABLE_MACRO(PageCache);

    public:
        static PageCache* GetInstance()
        {
            return &_instance;
        }
        //获取指定页数的Span------全新未进行内存块划分------可能被多线程调用
        Span* NewSpan(size_t pageNum);
        //获取指定内存块所属的Span指针---分配给了CentralCache的Span的指针
        Span* MemoryPtr2Span(void* ptr);
        //从CentralCache中回收空闲Span并合并成更大页空间的Span
        void RecycleFreeSpanFromCC(Span* span);
    private:
        PageCache()=default;
        ~PageCache()=default;
        //NewSpan的无锁版本---防止递归调用产生死锁问题
        Span* newSpanLockfree(size_t pageNum);
    private:
        //PageCache的hash结构,是通过Span管理的页数[1-128]进行区分SpanLists
        SpanList _spanLists[PAGE_NUM];
        std::mutex _mtx; //该锁直接锁住整个PageCache的hash结构
        std::unordered_map<PageID,Span*> _PageId2Span; //PageId与相应的Span指针的映射关系
        //----因为线程调用NewSpan进行n号SpanList获取Span,分裂的时候会影响其他的SpanList,因此任何时候只允许一个线程操作PageCache的hash结构
        static PageCache _instance; //饿汉单例
    };
} // namespace Xten

#endif