#include "logger.h"

#include "../file_system/vfs.h"
#include "../time/time_manager.h"
#include <iostream>
#include <fstream>

#define PRINT_LOG_LEVELS 0

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward declarations
    //---------------------------------------------------------------------------
    
    // Return the type of a log-message as a string: [LOGTYPE]
    std::string getTypeAsString(LogType logtype, LogLevel logLevel)
    {
        std::string result = "[UNKNOWN]";
        switch (logtype)
        {
        case LogType::LOGTYPE_INFO:
            result = "[Info";
            break;
        case LogType::LOGTYPE_WARNING:
            result = "[Warning";
            break;
        case LogType::LOGTYPE_ERROR:
            result = "[Error";
            break;
        default:
            break;
        }
#if PRINT_LOG_LEVELS
        result += " <LVL: " + std::to_string(logLevel) + ">";
#endif
        result += "]";

        return result;
    }

    //---------------------------------------------------------------------------
    //  Static declarations
    //---------------------------------------------------------------------------

    std::vector<LOG_MESSAGE> Logger::messages;
    LogLevel Logger::currentLogLevel = LOG_LEVEL_ALL;

    //---------------------------------------------------------------------------
    //  Static Functions
    //---------------------------------------------------------------------------

    // Stores the message and prints it out on stdout.
    //[LogType]: Message
    void Logger::Log(const std::string& msg, LogType logtype, LogLevel logLevel)
    {
        // Skip unimportant log-messages
        if(logLevel > currentLogLevel)
            return;

#ifdef  NDEBUG
        switch (logtype)
        {
        case LogType::LOGTYPE_INFO:
            break;
        case LogType::LOGTYPE_WARNING:
            break;
        case LogType::LOGTYPE_ERROR:
            reportErrorAndTerminate(msg);
            break;
        default:
            break;
        }
#else
        // Store the message
        LOG_MESSAGE message;
        message.message = msg;
        message.type = logtype;
        message.level = logLevel;

        // Store the message for write them later to a file if desired
        //messages.push_back(message); 

        switch (logtype)
        {
        case LogType::LOGTYPE_INFO:
            std::cout << getTypeAsString(logtype, logLevel) << ": " << msg << std::endl;
            break;
        case LogType::LOGTYPE_WARNING:
            std::cout << getTypeAsString(logtype, logLevel) << ": " << msg << std::endl;
            break;
        case LogType::LOGTYPE_ERROR:
            std::cerr << getTypeAsString(logtype, logLevel) << ": " << msg << std::endl;
            reportErrorAndTerminate(msg);
            break;
        default:
            break;
        }
#endif
    }

    // Called when Logger::Log(..., LOGTYPE_ERROR) was called - Terminates the program with the error-message
    void Logger::reportErrorAndTerminate(const std::string& msg)
    {
        const std::string outMessage = msg + ". Program will be terminated.";
        #ifdef _WIN32
            MessageBox(NULL, outMessage.c_str(), "ERROR", MB_OK | MB_ICONERROR);
        #elif
            std::cerr << msg << std::endl;
        #endif
        exit(1);
    }

    // Write all Messages to a file (Not implemented yet)
    void Logger::writeMessagesToFile(bool overrideOldLog)
    {
        std::string fileName      = "pyro engine";
        std::string fileExtension = ".log";
        std::string fullFileName  = fileName + fileExtension;

        std::string directoryPath = VFS::resolvePhysicalPath("/log/");
        std::string date          = TimeManager::getCurrentDate();
        std::replace(date.begin(), date.end(), ':', '_'); // Windows does not allow ":" characters in a filename

        std::string physicalPath  = directoryPath + std::string(fullFileName);

        // Check if the file already exists if it shouldn't be overriden
        if (!overrideOldLog)
        {
            bool exists = FileSystem::fileExists(physicalPath);
            if (exists) // Use the date as the file-name instead, which is obviously unique
                physicalPath = directoryPath + date + fileExtension;
        }

        // Write stuff to the file
        std::ofstream logFile;
        logFile.open(physicalPath);

        logFile << "Written Log at: " << date << std::endl << std::endl;
        for (const auto& msg : messages)
            logFile << getTypeAsString(msg.type, msg.level) << ": " << msg.message << std::endl;

        logFile.close();
    }


}