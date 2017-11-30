#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <build_options.h>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct SystemTime
    {
        unsigned short year = 0;
        unsigned short month = 0;
        unsigned short day = 0;
        unsigned short hour = 0;
        unsigned short minute = 0;
        unsigned short second = 0;
        unsigned short millisecond = 0;

        std::string toString() const { return TS(day)+"/"+TS(month)+"/"+TS(year)+" "+TS(hour)+":"+TS(minute)+":"+TS(second); }
        bool operator==(const SystemTime& other) const { return millisecond == other.millisecond && second == other.second && minute == other.minute && hour == other.hour && day == other.day && month == other.month && year == other.year; }
        bool operator!=(const SystemTime& other) const { return !(*this == other); }
    };

    //---------------------------------------------------------------------------
    //  FileSystem Class
    //---------------------------------------------------------------------------

    class FileSystem
    {

    public:
        static bool                     fileExists(const std::string& filePath);
        static std::string              getFileExtension(const std::string& filePath);
        static std::string              load(const std::string& filePath);
        static std::vector<uint32_t>    readBinaryFile(const char* filename);

        // "res/models/cat/cat.obj" --> return "res/models/cat/"
        static std::string              getDirectoryPath(const std::string& filePath);

        // OS dependant functions
        static SystemTime getLastWrittenFileTime(const std::string& filePath);
    };


}






#endif //!FILE_SYSTEM_H_