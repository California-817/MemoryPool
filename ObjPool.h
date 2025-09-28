#ifndef __XTEN_OBJPOOL_H__
#define __XTEN_OBJPOOL_H__
#include <unistd.h>
#include <memory>
#include<iostream>
#include "nocopyable.h"
#ifdef __linux__
#include <sys/mman.h>
#endif
// 定长对象内存池
namespace Xten
{
#define MMAP_SIZE 128 * 1024
    // 定义linux平台向os申请堆空间的函数
    inline void *SystemCallMemory(size_t size)
    {
#ifdef __linux__
        void *ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED)
            return nullptr;
#endif
        return ptr;
    }
    template <class T> // 模版只能在.h文件中进行定义
    class ObjPool
    {
        NO_COPYABLE_MACRO(ObjPool<T>);

    public:
        typedef std::shared_ptr<ObjPool> ptr;
        ObjPool();
        ~ObjPool();
        // 返回一个对象的指针
        T *New();
        // 释放一个指针指向的对象
        void Delete(T *ptr);

    private:
        char *_memory;        // 指向可用内存空间
        size_t _freeCapacity; // 剩余空间大小
        void *_freeList;      // 空闲链表管理上层释放的空间
    };

    template <class T>
    ObjPool<T>::ObjPool()
        : _memory(nullptr),
          _freeCapacity(0),
          _freeList(nullptr)
    {
    }
    template <class T>
    ObjPool<T>::~ObjPool()
    {
        //析构定长内存池需要保证内存都已经释放----暂时不考虑，因为上层应用层会一直使用
        std::cout<<"ObjPool<T>::~ObjPool()"<<std::endl;
    }
    // 返回一个对象的指针
    template <class T>
    T *ObjPool<T>::New()
    {
        T *obj = nullptr;
        // 1.先看空闲链表是否有回收空间
        if (_freeList)
        {
            // 拿到下一个内存块地址
            void *next = *((void **)_freeList);
            obj = (T *)_freeList;
            _freeList = next;
        } // 无回收空间
        else
        {
            size_t objSize = sizeof(T) > sizeof(void *) ? sizeof(T) : sizeof(void *);
            if (_freeCapacity < objSize) // 这一块空间的剩余空间不足一个对象(剩余的空间不会被利用，内部碎片问题)
            {
                _freeCapacity = MMAP_SIZE; // 128KB
                // 重新开辟空间
                // 原来的空间都被应用层使用，后续归还到空闲链表继续使用，无需关心释放问题
                _memory = (char *)SystemCallMemory(_freeCapacity);
                if (_memory == nullptr) // failed
                    throw std::bad_alloc();
            }
            // success or 剩余空间足够
            obj = (T *)_memory;
            _memory += objSize;
            _freeCapacity -= objSize;
        }
        // placement new 创建对象
        new (obj) T();
        return obj;
    }
    // 释放一个指针指向的对象
    template <class T>
    void ObjPool<T>::Delete(T *ptr)
    {
        ptr->~T(); //1.显式调用析构函数
        //2.归还空间到空闲链表---头插
        void* next=_freeList;
        *((void**)ptr)=next;
        _freeList=ptr;
    }
} // namespace Xten

#endif