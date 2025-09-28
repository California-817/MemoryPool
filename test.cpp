#include "ObjPool.h"
#include <iostream>
#include <vector>
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
int main()
{
    testObjPool();
    return 0;
}