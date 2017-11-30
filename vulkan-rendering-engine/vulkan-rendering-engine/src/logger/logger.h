#ifndef LOGGER_H_
#define LOGGER_H_

#include "platform.hpp"
#include <vector>
#include <string>

// TODO: 
// - Inherit from a "System-Class" (which initializes the Logger, deinitializes it) --> then it writes all messages to a file if enabled
// - Add the timer so you know when a message were created 
// - Colored output
// - using the UI-System of the Engine instead of stdout


namespace Pyro
{
    #define LOG_LEVEL_VERY_IMPORTANT    LOG_LEVEL_0
    #define LOG_LEVEL_IMPORTANT         LOG_LEVEL_1
    #define LOG_LEVEL_NOT_SO_IMPORTANT  LOG_LEVEL_2
    #define LOG_LEVEL_NOT_IMPORTANT     LOG_LEVEL_3

    enum LogType
    {
        LOGTYPE_INFO = 0,
        LOGTYPE_WARNING = 1,
        LOGTYPE_ERROR = 2
    };

    enum LogLevel
    {
        LOG_LEVEL_0 = 0, // Most important log-level
        LOG_LEVEL_1 = 1,
        LOG_LEVEL_2 = 2,
        LOG_LEVEL_3 = 3,

        LOG_LEVEL_ALL
    };

    struct LOG_MESSAGE
    {
        std::string message;
        LogType     type;
        LogLevel    level;
    };

    //---------------------------------------------------------------------------
    //  Logger class
    //---------------------------------------------------------------------------

    class Logger
    {
    public:
        static void Log(const std::string&, LogType logtype = LOGTYPE_INFO, LogLevel logLevel = LOG_LEVEL_0);

        // Change the Log-Level. Only log-messages with log-level <= current-log-level will be stored and displayed
        // E.g. newLogLevel = LOG_LEVEL_1 displays all log-messages with level 0 + 1
        // LOG_LEVEL_ALL displays all messages then
        static void setLogLevel(LogLevel newLogLevel) { currentLogLevel = newLogLevel; }

        // Write all Messages to a file
        static void writeMessagesToFile(bool overrideOldLog = true);

    private:
        static std::vector<LOG_MESSAGE> messages;
        static LogLevel currentLogLevel; // Log only messages with a level below/equal this one

        // Called when Logger::Log(..., LOGTYPE_ERROR) was called - Terminates the program with the error-message
        static void reportErrorAndTerminate(const std::string& msg);
    };



}



#endif // !LOGGER_H_
