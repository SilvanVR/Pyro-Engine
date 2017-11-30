#include "file_system.h"

#include <assert.h>
#include <fstream>
#include <sstream>

namespace Pyro
{

    std::string FileSystem::getDirectoryPath(const std::string& filePath)
    {
        return filePath.substr(0, filePath.find_last_of("/\\")) + "/";
    }

    bool FileSystem::fileExists(const std::string& filePath)
    {
        if (FILE *file = fopen(filePath.c_str(), "r")) {
            fclose(file);
            return true;
        }
        else {
            return false;
        }
    }

    std::vector<uint32_t> FileSystem::readBinaryFile(const char* path)
    {
        FILE *file = fopen(path, "rb");
        if (!file)
        {
            fprintf(stderr, "Failed to open binary file: %s\n", path);
            assert(0);
            return{};
        }

        fseek(file, 0, SEEK_END);
        long len = ftell(file) / sizeof(uint32_t);
        rewind(file);

        std::vector<uint32_t> spirv(len);
        if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
            spirv.clear();

        fclose(file);
        return spirv;
    }


    std::string FileSystem::getFileExtension(const std::string& filename)
    {
        std::vector<std::string> tokens = splitString(filename, '.');

        std::string fileExtension = tokens.back();

        return fileExtension;
    }

    std::string FileSystem::load(const std::string& filePath)
    {
        std::ifstream stream;

        if (!FileSystem::fileExists(filePath))
            Logger::Log("FileSystem::load(): File '" + filePath + "' does not exist", LOGTYPE_ERROR);

        stream.open(filePath);
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        stream.close();

        return stringStream.str();
    }

#ifndef _WIN32
    // Dummy implementations for OS dependant calls
    SystemTime FileSystem::getLastWrittenFileTime(const std::string& filePath)
    {
        SystemTime fileTime = {};
        return fileTime;
    }
#endif


}