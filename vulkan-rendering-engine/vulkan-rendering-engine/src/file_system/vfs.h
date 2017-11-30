#ifndef VFS_H_
#define VFS_H_

#include "file_system.h"
#include <string>
#include <map>

namespace Pyro
{

    class VFS
    {

    public:
        static void mount(const std::string& name, const std::string& path, bool overrideOldOne = true);
        static void unmount(const std::string& name);
        static std::vector<uint32_t> readBinaryFile(const std::string& virtualPath);
        static bool fileExists(const std::string& virtualPath);

        static std::string resolvePhysicalPath(const std::string& name);

    private:
        static std::map<std::string, std::string> mountPoints;

    };



}


#endif // !VFS_H_

