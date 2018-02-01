#include "file_system.h"

#ifdef _WIN32
#include <Windows.h>

namespace Pyro
{

    bool FileSystem::getLastWrittenFileTime(const std::string& filePath, SystemTime& sysTime)
    {
        HANDLE hFile;
        FILETIME ftLastWriteTime;
        SYSTEMTIME stUTC, stLocal;
        hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            Logger::Log("FileSystem-Windows: Could not open file '" + filePath + "'", LOGTYPE_WARNING);
            return false;
        }

        if (!GetFileTime(hFile, NULL, NULL, &ftLastWriteTime))
        {
            Logger::Log("FileSystem-Windows: Could not get the filetime of file '" + filePath + "'", LOGTYPE_WARNING);
            return false;
        }

        CloseHandle(hFile);

        FileTimeToSystemTime(&ftLastWriteTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        sysTime.year            = stLocal.wYear;
        sysTime.month           = stLocal.wMonth;
        sysTime.day             = stLocal.wDay;
        sysTime.hour            = stLocal.wHour;
        sysTime.minute          = stLocal.wMinute;
        sysTime.second          = stLocal.wSecond;
        sysTime.millisecond     = stLocal.wMilliseconds;

        return true;
    }


}


#endif