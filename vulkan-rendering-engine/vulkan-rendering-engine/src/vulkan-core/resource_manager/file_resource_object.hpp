#ifndef FILE_RESOURCE_OBJECT_H_
#define FILE_RESOURCE_OBJECT_H_

#include "resource_object.hpp"
#include "file_system/vfs.h"

namespace Pyro
{

    class FileResourceObject : public ResourceObject
    {
    public:
        //FileResourceObject() : m_filePath(""), m_fileTime(), m_isReloading(false) {}
        FileResourceObject(const std::string& filepath, const std::string& name = "")
            : ResourceObject(name != "" ? name : filepath), m_filePath(filepath), m_isReloading(false)
        {
            if (filepath != "")
                if (FileSystem::fileExists(VFS::resolvePhysicalPath(filepath)))
                    FileSystem::getLastWrittenFileTime(VFS::resolvePhysicalPath(filepath), m_fileTime);
        }
        virtual ~FileResourceObject() {}

        // Getter's
        const std::string& getFilePath() const { return m_filePath; }
        const SystemTime& getFileTime() const { return m_fileTime; }

    private:
        std::string     m_filePath;
        SystemTime      m_fileTime;
        bool            m_isReloading;
    };


}


#endif // !FILE_RESOURCE_OBJECT_H_
