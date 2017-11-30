#include "allocator.h"

#include "memory_manager.h"
#include "logger/logger.h"

#include <malloc.h>
#include <cstdint>
#include <cstring>
#include <assert.h>

namespace Pyro
{
    #define PY_MEMORY_ALIGNMENT 16
    #define PY_ALLOC(size)      _aligned_malloc(size, PY_MEMORY_ALIGNMENT)
    #define PY_FREE(block)      _aligned_free(block);

    void* Allocator::allocate(size_t size)
    {
        assert(size < 1024 * 1024 * 1024);

        MemoryManager::memoryInfo.currentAllocated += size;
        MemoryManager::memoryInfo.totalAllocated += size;
        MemoryManager::memoryInfo.totalAllocations++;

        size_t actualSize = size + sizeof(size_t);
        uint8_t* result = (uint8_t*)malloc(actualSize);
        memset(result, 0, actualSize);
        memcpy(result, &size, sizeof(size_t));
        result += sizeof(size_t);

        return result;
    }

    void Allocator::freeMem(void* mem)
    {
        if(mem == nullptr) return;

        uint8_t* memory = ((uint8_t*)mem) - sizeof(size_t);
        size_t size = *(size_t*)memory;

        MemoryManager::memoryInfo.totalFreed += size;
        MemoryManager::memoryInfo.currentAllocated -= size;
        MemoryManager::memoryInfo.totalDeallocations++;

        free(memory);
    }


    void* Allocator::allocateDebug(size_t size, const char* file, unsigned int line)
    {
        if (size > 1024 * 1024)
            Logger::Log("Large allocation (>1mb) " + std::string(file) + ":" + std::to_string(line), LOGTYPE_WARNING);

        return allocate(size);
    }

    void Allocator::freeMemDebug(void* mem, const char* file, unsigned int line)
    {
        freeMem(mem);
    }
}