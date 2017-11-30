#include "vfs.h"

#include "logger/logger.h"
#include <assert.h>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    std::map<std::string, std::string> VFS::mountPoints;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------


    //---------------------------------------------------------------------------
    //  Public Members
    //---------------------------------------------------------------------------

    void VFS::mount(const std::string& name, const std::string& path, bool overrideOldOne)
    {
        if(mountPoints.count(name) != 0 && !overrideOldOne) return;

        Logger::Log("Mount virtual-path '" + name + "' to '" + path + "'", LOGTYPE_INFO);
        mountPoints[name] = path;
    }

    void VFS::unmount(const std::string& name)
    {
        mountPoints.erase(name);
    }

    std::string VFS::resolvePhysicalPath(const std::string& name)
    {
        assert(name.size() >= 2);

        if (name[0] == '/')
        { 
            std::vector<std::string> strings = splitString(name, '/');
            const std::string& virtualDir = strings.front();
            assert(mountPoints.count(virtualDir) != 0);

            return mountPoints[virtualDir] + name.substr(virtualDir.size() + 1);
        }
        return name;
    }

    std::vector<uint32_t> VFS::readBinaryFile(const std::string& virtualPath)
    {
        std::string physicalPath = resolvePhysicalPath(virtualPath);
        return FileSystem::readBinaryFile(physicalPath.c_str());
    }


    bool VFS::fileExists(const std::string& virtualPath)
    {
        std::string physicalPath = resolvePhysicalPath(virtualPath);
        return FileSystem::fileExists(physicalPath);
    }

}