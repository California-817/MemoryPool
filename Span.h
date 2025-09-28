#ifndef __XTEN_SPAN_H__
#define __XTEN_SPAN_H__
#include <unistd.h>
#include <mutex>
#include <assert.h>
#include "nocopyable.h"
namespace Xten
{
    typedef size_t PageID;
    // centralcache和pagecache需要用到的Span结构体
    struct Span
    {
        // 双向链表组织起来
        Span *prev = nullptr;
        Span *next = nullptr;

        PageID pageId = 0; // 页编号

        size_t pageCount = 0; // 所拥有页数
        size_t usedCount = 0; // 给ThreadCache使用空间块数

        void *list = nullptr; // 可用空间的链表起始地址
    };
    // span组织起来的双向带头循环链表
    class SpanList
    {
    public:
        NO_COPYABLE_MACRO(SpanList);
        SpanList()
            : _head(new Span())
        {
            // 创建一个哨兵头结点即可
            _head->next = _head;
            _head->prev = _head;
        }
        // 在指定pos位置之后插入一个span节点
        void Insert(Span *pos, Span *node)
        {
            assert(pos && node);
            Span *next = pos->next;
            pos->next = node;
            node->prev = pos;
            node->next = next;
            next->prev = node;
        }
        // 删除一个node节点,只是从链表中断链接关系
        void Erase(Span *node)
        {
            // 只是从链表中断链接关系
            assert(node && (node != _head));
            Span *prev = node->prev;
            Span *next = node->next;
            prev->next = next;
            next->prev = prev;
        }
        // 获取锁
        std::mutex &GetMutex() { return _mtx; }

    private:
        Span *_head;
        std::mutex _mtx; // 互斥锁-----多个tc会访问一个cc的某个spanlist
    };
} // namespace Xten

#endif