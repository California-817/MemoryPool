#include "xmalloc.h"
#include"ThreadCache.h"
#include<assert.h>
namespace Xten
{
    // 1.开空间函数
    void *xmalloc(size_t size)
    {
        ThreadCache* tc=ThreadCache::GetThreadCache();
        assert(tc);
        return tc->Allocate(size);
    }
    // 2.释放空间函数
    void xfree(void *ptr)
    {
        //todo
    }
} // namespace Xten
