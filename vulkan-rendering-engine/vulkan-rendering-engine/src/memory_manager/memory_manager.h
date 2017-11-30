#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <stdint.h>
#include <string>

namespace Pyro
{

    struct SystemMemoryInfo
    {
        float       percentageUsed;
        uint64_t    totalMB;
        uint64_t    freeMB;
        uint64_t    currentAllocatedMB;
    };

    struct MemoryInfo
    {
        uint64_t currentAllocated;
        uint64_t totalAllocated;
        uint64_t totalFreed;
        uint64_t totalAllocations;
        uint64_t totalDeallocations;

        MemoryInfo() : currentAllocated(0), totalAllocated(0), totalFreed(0), totalAllocations(0), totalDeallocations(0){}
    };


    class MemoryManager
    {
        friend class Allocator;

    public:
        static const MemoryInfo& getMemoryInfo(){ return memoryInfo; }
        static std::string bytesToString(uint64_t bytes, bool binaryPrefix = false);
        static void log();
        static SystemMemoryInfo getSystemMemoryInfo();

    private:
        static MemoryInfo memoryInfo;

    };







}



#endif // !MEMORY_MANAGER_H_
