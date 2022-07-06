#pragma once
#include "DebugColors.h"
#include <iostream>
#include "Core.h"

#define LOG_I(message) Ren::Logger::LogI(message, __FILE__, __LINE__)
#define LOG_E(message) { Ren::Logger::LogE(message, __FILE__, __LINE__); }
#define LOG_C(message) { Ren::Logger::LogC(message, __FILE__, __LINE__); }
#define LOG_W(message) Ren::Logger::LogW(message, __FILE__, __LINE__)

namespace Ren
{
    enum class LogType: int {
        Info,
        Error,
        Critical,
        Warning
    };

    inline std::string LogTypeToString(const LogType& t)
    {
        switch (t)
        {
        case LogType::Info:
            return DC_INFO;
        case LogType::Error:
            return DC_ERROR;
        case LogType::Warning:
            return DC_WARNING;
        case LogType::Critical:
            return "[" DC_RED "CRITICAL ERROR" DC_DEFAULT "]";
        default:
            return "[unknown]";
        }
    }

    struct LogEntry
    {
        LogType type;
        std::string message;
        std::string file;
        int line;
    };

    class LogEntryHandler
    {
    public:
        LogEntryHandler() {}
        virtual ~LogEntryHandler() {}

        virtual void HandleEntry(const LogEntry& entry)
        {
            if (entry.file == "" && entry.line == -1)
                std::cout << LogTypeToString(entry.type) << ": " << entry.message << std::endl;
            else
                std::cout << entry.file << ":" << entry.line << ": " << LogTypeToString(entry.type) << " " << entry.message << std::endl;
        }
    };

    // TODO: Logger should be able to output logs to a file, which will then contain: date, time, file, line, thread and message.
    //       etc.
    class Logger
    {
        inline static LogEntryHandler mBasicEntryHandler;
    public:
        inline static LogEntryHandler* EntryHandler = &mBasicEntryHandler;

        static void Log(const LogType& type, const std::string& message, const std::string& file, const int& line)
        {
            if (EntryHandler)
                EntryHandler->HandleEntry({type, message, file, line});
        }
        static void LogI(const std::string& message, const std::string& file, const int& line) { Log(LogType::Info, message, file, line); }
        static void LogE(const std::string& message, const std::string& file, const int& line) { Log(LogType::Error, message, file, line); }
        static void LogC(const std::string& message, const std::string& file, const int& line) { Log(LogType::Critical, message, file, line); }
        static void LogW(const std::string& message, const std::string& file, const int& line) { Log(LogType::Warning, message, file, line); }

        inline static void SetHandler(LogEntryHandler* handler) { EntryHandler = handler; }
        inline static void SetDefaultHandler() { EntryHandler = &mBasicEntryHandler; }
    private:
        Logger() {};
    };
}