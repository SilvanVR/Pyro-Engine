#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_


namespace Pyro
{

    class Allocator
    {

    public:
        static void* allocate(size_t size);
        static void  freeMem(void* mem);

        static void* allocateDebug(size_t size, const char* file, unsigned int line);
        static void  freeMemDebug(void* mem, const char* file, unsigned int line);
    };

}


#endif // !ALLOCATOR_H_

