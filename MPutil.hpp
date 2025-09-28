#ifndef __XTEN_UTIL_H__
#define __XTEN_UTIL_H__
#include <unistd.h>
#include <assert.h>
namespace Xten
{
    class MemoryPoolUtil
    {
    public:
        // 向上对齐到指定大小
        static size_t RoundUp(size_t size)
        {
            assert(size > 0);
#define XX(size, alignNum) \
    ((size + alignNum - 1) & ~(alignNum - 1));
            if (size <= 128)
            {
                return XX(size, 8);
            }
            else if (size <= 1024)
            {
                return XX(size, 16);
            }
            else if (size <= 8 * 1024)
            {
                return XX(size, 128);
            }
            else if (size <= 64 * 1024)
            {
                return XX(size, 1024);
            }
            else if (size <= 256 * 1024)
            {
                return XX(size, 8 * 1024);
            }
#undef XX
            else
            {
                assert(false);
            }
            return -1;
        }
        // 根据大小查找hash下标
        static size_t FindIndex(size_t size)
        {
            size_t prevs[4] = {16, 56, 56, 56};
#define XX(size, align_shift) \
    ((size + (1 << align_shift) - 1) >> align_shift) - 1;
            if (size <= 128)
            {
                return XX(size, 3);
            }
            else if (size <= 1024)
            {
                return XX(size, 4) + prevs[0];
            }
            else if (size <= 8 * 1024)
            {
                return XX(size, 7) + prevs[0] + prevs[1];
            }
            else if (size <= 64 * 1024)
            {
                return XX(size, 10) + prevs[0] + prevs[1] + prevs[2];
            }
            else if (size <= 256 * 1024)
            {
                return XX(size, 13) + prevs[0] + prevs[1] + prevs[2] + prevs[3];
            }
#undef XX
            else
            {
                assert(false);
            }
            return -1;
        }
        // 根据对齐内存大小判断申请空间块数 [2-500]
        static size_t Size2Num(size_t alignsize)
        {
            size_t num = MAX_ALLOC_SIZE / alignsize;
            if (num > 500)
                num = 500;
            if (num < 2)
                num = 2;
            return num;
        }
    };
}
#endif