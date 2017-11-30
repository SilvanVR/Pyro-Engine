#ifndef MEMORY_H_
#define MEMORY_H_

#include "allocator.h"

#define USE_CUSTOM_ALLOCATOR 1

#define DEBUG_NEW       new(__FILE__, __LINE__)
#define DEBUG_DELETE    delete(__FILE__, __LINE__)

#define pynew           DEBUG_NEW
#define pydelete        DEBUG_DELETE

#if USE_CUSTOM_ALLOCATOR

void* operator new(size_t size)
{
    return Pyro::Allocator::allocate(size);
}

void operator delete(void* mem)
{
    Pyro::Allocator::freeMem(mem);
}

void* operator new[](size_t size)
{
    return Pyro::Allocator::allocate(size);
}

void operator delete[](void* mem)
{
    Pyro::Allocator::freeMem(mem);
}

#endif

void* operator new(size_t size, const char* file, unsigned int line)
{
    return Pyro::Allocator::allocateDebug(size, file, line);
}

void operator delete(void* mem, const char* file, unsigned int line)
{
    Pyro::Allocator::freeMemDebug(mem, file, line);
}

void* operator new[](size_t size, const char* file, unsigned int line)
{
    return Pyro::Allocator::allocateDebug(size, file, line);
}

void operator delete[](void* mem, const char* file, unsigned int line)
{
    Pyro::Allocator::freeMemDebug(mem, file, line);
}



#endif // !MEMORY_H_

