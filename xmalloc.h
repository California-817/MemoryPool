#ifndef __XTEN_XMALLOC_H__
#define __XTEN_XMALLOC_H__
#include<unistd.h>
namespace Xten
{
    //1.开空间函数
    void* xmalloc(size_t size);
    //2.释放空间函数
    void xfree(void* ptr);
} // namespace Xten

#endif