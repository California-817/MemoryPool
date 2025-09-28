#ifndef __XTEN_CENTRALCACHE_H__
#define __XTEN_CENTRALCACHE_H__
#include "Span.h"
#include "nocopyable.h"
namespace Xten
{
// SpanList的个数
#define FREE_LISTS_NUM 208
    class CentralCache
    {
    public:
        NO_COPYABLE_MACRO(CentralCache);
        // 获取饿汉单例的CentralCache结构
        static CentralCache *GetInstance()
        {
            return &_instance;
        }
        //向threadCache分配一定范围的空间 
        //[begin---空间起始地址] [end---空间结束地址] [batchNum---申请块数] [size---申请单块空间大小(已经对齐)] [ret---实际分配块数]
        size_t fetchRangeMemory2TC(void*& begin,void*& end,size_t batchNum,size_t size);
        //获取一个管理空间不为空的Span
        Span* GetOneSpan(SpanList& list,size_t size);
    private:
        CentralCache() = default;
        ~CentralCache() = default;
        // [1-128] 8B对齐        -------  [0,15] ------16
        // [128+1-1024] 16B对齐  ----------- [16,71] ------56
        // [1024+1-8*1024] 128B对齐 ---------[72,127] ------56
        // [8*1024+1-64*1024] 1024B对齐 ------[128,183]  -------56
        // [64*1024+1-256*1024] 8*1024B对齐 ------[184,207] ------24
        SpanList _spanLists[FREE_LISTS_NUM];
        static CentralCache _instance;
    };
} // namespace Xten

#endif