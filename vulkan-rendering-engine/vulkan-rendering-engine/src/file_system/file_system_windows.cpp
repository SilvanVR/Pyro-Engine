#include "file_system.h"

#ifdef _WIN32
#include <Windows.h>

namespace Pyro
{

    SystemTime FileSystem::getLastWrittenFileTime(const std::string& filePath)
    {
        HANDLE hFile;
        FILETIME ftLastWriteTime;
        SYSTEMTIME stUTC, stLocal;
        hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            Logger::Log("FileSystem-Windows: Could not open file '" + filePath + "'", LOGTYPE_ERROR);
        }

        if (!GetFileTime(hFile, NULL, NULL, &ftLastWriteTime))
        {
            Logger::Log("FileSystem-Windows: Could not get the filetime of file '" + filePath + "'", LOGTYPE_ERROR);
        }

        CloseHandle(hFile);

        FileTimeToSystemTime(&ftLastWriteTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        SystemTime sysTime = {};
        sysTime.year            = stLocal.wYear;
        sysTime.month           = stLocal.wMonth;
        sysTime.day             = stLocal.wDay;
        sysTime.hour            = stLocal.wHour;
        sysTime.minute          = stLocal.wMinute;
        sysTime.second          = stLocal.wSecond;
        sysTime.millisecond     = stLocal.wMilliseconds;

        return sysTime;
    }


}


#endif