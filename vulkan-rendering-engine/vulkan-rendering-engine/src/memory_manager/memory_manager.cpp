#include "memory_manager.h"

#include "logger/logger.h"

namespace Pyro
{

    MemoryInfo MemoryManager::memoryInfo;

    std::string MemoryManager::bytesToString(uint64_t bytes, bool binaryPrefix)
    {
        static const uint64_t gigabyte = 1000 * 1000 * 1000;
        static const uint64_t megabyte = 1000 * 1000;
        static const uint64_t kilobyte = 1000;

        static const uint64_t gibibyte = 1024 * 1024 * 1024;
        static const uint64_t mebibyte = 1024 * 1024;
        static const uint64_t kibibyte = 1024;

        uint64_t gb = binaryPrefix ? gibibyte : gigabyte;
        uint64_t mb = binaryPrefix ? mebibyte : megabyte;
        uint64_t kb = binaryPrefix ? kibibyte : kilobyte;

        std::string result;
        if(bytes > gb)
            result = std::to_string((float)bytes / gb) + (binaryPrefix ? " GiB" : " GB");
        else if(bytes > mb)
            result = std::to_string((float)bytes / mb) + (binaryPrefix ? " MiB" : " MB");
        else if (bytes > kb)
            result = std::to_string((float)bytes / kb) + (binaryPrefix ? " KiB" : " KB");
        else
            result = std::to_string((float)bytes) + " Bytes";

        return result;
    }

    void MemoryManager::log()
    {
        Logger::Log("-------------- MEMORY INFO ---------------", LOGTYPE_INFO);
        Logger::Log("Current Allocated: " + MemoryManager::bytesToString(MemoryManager::getMemoryInfo().currentAllocated), LOGTYPE_INFO);
        Logger::Log("Total Allocated: " + MemoryManager::bytesToString(MemoryManager::getMemoryInfo().totalAllocated), LOGTYPE_INFO);
        Logger::Log("Total Freed: " + MemoryManager::bytesToString(MemoryManager::getMemoryInfo().totalFreed), LOGTYPE_INFO);
        Logger::Log("Total Allocations: " + std::to_string(MemoryManager::getMemoryInfo().totalAllocations), LOGTYPE_INFO);
        Logger::Log("Total Deallocations: " + std::to_string(MemoryManager::getMemoryInfo().totalDeallocations), LOGTYPE_INFO);
        Logger::Log("------------------------------------------", LOGTYPE_INFO);
    }

#ifndef _WIN32
    SystemMemoryInfo MemoryManager::getSystemMemoryInfo()
    {
        Logger::Log("MemoryManager::getSystemMemoryInfo() is not implemented for this OS. "
                    "Everything will be 0.", LOGTYPE_WARNING);
        return SystemMemoryInfo();
    }
#endif // !_WIN32

}