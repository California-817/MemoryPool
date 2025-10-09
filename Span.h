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

        PageID pageId = 0; // 页编号----类似于虚拟地址中的前20位形成的一二级页表

        size_t pageCount = 0; // 所拥有页数
        size_t usedCount = 0; // 给ThreadCache使用空间块数

        void *list = nullptr; // 可用空间的链表起始地址

        bool isUsed=false; //表示这个Span是否被使用
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
        //头插一个Span节点
        void PushFront(Span* node)
        {
            Insert(_head,node);
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
        //链表是否为空
        bool IsEmpty()
        {
            return _head->next==_head;
        }
        //获取非空的第一个Span----会将Span从链表中删除
        Span* GetFrontSpan()
        {
            Span* front=Begin();
            Erase(front);
            return front;
        }
        //返回头节点
        Span* Begin()
        {
            return _head->next;
        }
        //返回最后一个节点的下一个节点---哨兵节点
        Span* End()
        {
            return _head;
        }
        // 获取锁
        std::mutex &GetMutex() { return _mtx; }

    private:
        Span *_head;
        std::mutex _mtx; // 互斥锁-----多个tc会访问一个cc的某个spanlist
    };
} // namespace Xten

#endif