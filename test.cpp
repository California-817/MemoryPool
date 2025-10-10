#include "ObjPool.h"
#include <iostream>
#include <vector>
#include "xmalloc.h"
class obj
{
public:
    obj()
    {
        std::cout << "obj" << std::endl;
    }
    ~obj()
    {
        std::cout << "~obj" << std::endl;
    }

private:
    int a;
    char b;
};
void testObjPool()
{
    std::vector<obj *> objs;
    Xten::ObjPool<obj> pool;
    for (int i = 0; i < 10; i++)
    {
        objs.push_back(pool.New());
    }
    for (auto &e : objs)
    {
        pool.Delete(e);
    }
}
void test_1()
{
    // 40byte
    void *p1 = Xten::xmalloc(5); // 1. 1
    void *p2 = Xten::xmalloc(8); // 2. 3
    void *p3 = Xten::xmalloc(4); // none. 3
    void *p4 = Xten::xmalloc(6); // 3. 6
    void *p5 = Xten::xmalloc(3); // none. 6
    void *p6 = Xten::xmalloc(3); // none. 6
    void *p7 = Xten::xmalloc(3); // none. 6

    // 一共申请6内存块 使用5内存块 curSize=1 usedCount=6 maxSize=4
    // free
    Xten::xfree(p1, 5); // curSize=2 usedCount=6
    Xten::xfree(p2, 8); // curSize=3 usedCount=6
    Xten::xfree(p3, 4); //.       4.          6
    Xten::xfree(p4, 6); //.       5 进行向上层回收---回收4个内存块
    // curSize=1 usedCount=2 maxSize=4
    Xten::xfree(p5, 3);
    // curSize=2 usedCount=2 maxSize=4
    Xten::xfree(p6, 3);
    Xten::xfree(p7, 3);
}
void test_2()
{
    void* p1=Xten::xmalloc(257*1024); //65页
    void* p2=Xten::xmalloc(129*1024*4); //129页
    Xten::xfree(p1,257*1024);
    Xten::xfree(p2,129*1024*4);
}
int main()
{
    // testObjPool();
    // void* ret=Xten::xmalloc(100);
    // Xten::xfree(ret);
    // test_1();
    test_2();
    return 0;
}