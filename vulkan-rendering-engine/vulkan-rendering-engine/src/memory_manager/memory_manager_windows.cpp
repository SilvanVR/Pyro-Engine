#include "memory_manager.h"

#ifdef _WIN32

#include <Windows.h>

namespace Pyro
{

    SystemMemoryInfo MemoryManager::getSystemMemoryInfo()
    {
        MEMORYSTATUSEX statex;

        statex.dwLength = sizeof(statex);

        GlobalMemoryStatusEx(&statex);

        SystemMemoryInfo memInfo;
        memInfo.percentageUsed = (float)statex.dwMemoryLoad;
        memInfo.totalMB = statex.ullTotalPhys;
        memInfo.freeMB = statex.ullAvailPhys;
        memInfo.currentAllocatedMB = memInfo.totalMB - memInfo.freeMB;

        return memInfo;
    }

}

#endif