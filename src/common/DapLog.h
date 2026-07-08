// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LOG_H
#define DAP_LOG_H

#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <mutex>
#ifdef _LINUX
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "TimeUtils.h"
#include "DapParamParser.h"

namespace Dap {
enum class LogLevel : int {
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_FATAL
};

enum class LogOutType {
    TERMINAL,
    FILE,
    BOTH
};

class LogPrefix {
public:
    static LogPrefix &Instance()
    {
        static LogPrefix instance;
        return instance;
    }

    inline const std::string TimePrefix(const TimeStyle &style = TimeStyle::DATE_TIME_1)
    {
        return TimeUtils::Instance().NowStr(style);
    }

    inline const std::string LogLevelPrefix(const LogLevel &level)
    {
        static std::map<LogLevel, std::string> map = { { LogLevel::L_INFO, "[Info]" },
                                                       { LogLevel::L_WARN, "[Warn]" },
                                                       { LogLevel::L_ERROR, "[Error]" },
                                                       { LogLevel::L_FATAL, "[Fatal]" },
                                                       { LogLevel::L_DEBUG, "[Debug]" } };
        if (map.count(level) == 0) {
            return map[LogLevel::L_INFO];
        }
        return map[level];
    }
    inline const std::string LocationPrefix(const std::string &fileName, const std::string &funcName, const int &line)
    {
        std::stringstream ss;
        ss << " (" << fileName << ", " << funcName << ", line: " << line << ") |";
        return ss.str();
    }

private:
    LogPrefix() = default;
    ~LogPrefix() = default;
};

class DapLog {
public:
    DapLog(const LogOutType &type, const std::string &filePath) : outType(type), originFilePath(filePath)
    {
        Initialize();
    }
    ~DapLog() noexcept
    {
        Destroy();
    }

    static std::unique_ptr<DapLog> NewInstance(const LogOutType &type, const std::string &filePath)
    {
        std::unique_ptr<DapLog> instance = std::make_unique<DapLog>(type, filePath);
        return instance;
    }

    inline DapLog &SetOutType(const LogOutType &type)
    {
        if (this->outType != type) {
            this->outType = type;
        }
        return *this;
    }

    inline DapLog &SetFilePath(const std::string &path)
    {
        if (this->filePath != path) {
            this->filePath = path;
        }
        return *this;
    }

    inline DapLog &SetLogLevel(const LogLevel &logLevel)
    {
        if (this->level != logLevel) {
            this->level = logLevel;
        }
        return *this;
    }

    inline DapLog &SetMaxSize(const int size)
    {
        if (this->maxSize != size) {
            this->maxSize = size;
        }
        return *this;
    }

    template <typename... ARGS> inline DapLog &Info(const ARGS... args)
    {
        Log(LogLevel::L_INFO, LogPrefix::Instance().LogLevelPrefix(LogLevel::L_INFO), args...);
        return *this;
    }

    template <typename... ARGS> inline DapLog &Warn(const ARGS... args)
    {
        Log(LogLevel::L_WARN, LogPrefix::Instance().LogLevelPrefix(LogLevel::L_WARN), args...);
        return *this;
    }

    template <typename... ARGS> inline DapLog &Error(const ARGS... args)
    {
        Log(LogLevel::L_ERROR, LogPrefix::Instance().LogLevelPrefix(LogLevel::L_ERROR), args...);
        return *this;
    }

    template <typename... ARGS> inline DapLog &Fatal(const ARGS... args)
    {
        Log(LogLevel::L_FATAL, LogPrefix::Instance().LogLevelPrefix(LogLevel::L_FATAL), args...);
        return *this;
    }

    template <typename... ARGS> inline DapLog &Debug(const ARGS... args)
    {
        Log(LogLevel::L_DEBUG, LogPrefix::Instance().LogLevelPrefix(LogLevel::L_DEBUG), args...);
        return *this;
    }

    template <typename... ARGS> inline DapLog &operator << (const ARGS... args)
    {
        Log(args...);
        return *this;
    }

    template <typename... ARGS>
    static inline void PrintLog(const LogLevel &logLevel, const std::string &fileName, const std::string &funcName,
                                const int &line, const ARGS... args)
    {
        std::lock_guard<std::mutex> lock(printInstanceMutex);
        if (printInstance == nullptr) {
            printInstance = std::make_unique<DapLog>(LogOutType::TERMINAL, "");
            printInstance->SetLogLevel(LogLevel::L_DEBUG);
        }
        printInstance->Log(logLevel, "[DAP SERVER] | ", LogPrefix::Instance().TimePrefix(), " | ",
                           LogPrefix::Instance().LogLevelPrefix(logLevel),
                           LogPrefix::Instance().LocationPrefix(fileName, funcName, line), " ", args..., "\n");
    }

    template <typename... ARGS>
    static inline void RecordLog(const LogLevel &logLevel, const std::string &fileName, const std::string &funcName,
                                 const int &line, const ARGS... args)
    {
        bool logEnable = DapParamParser::Instance().GetLogEnable();
        if (!logEnable) {
            return;
        }
        std::lock_guard<std::mutex> lock(logInstanceMutex);
        InitRecordLog();
        logInstance->Log(logLevel, "[DAP SERVER] | ", LogPrefix::Instance().TimePrefix(), " | ",
                         LogPrefix::Instance().LogLevelPrefix(logLevel),
                         LogPrefix::Instance().LocationPrefix(fileName, funcName, line), " ", args..., "\n");
    }

    static void SetLogFolderPermissions(const std::string &logFolderPath)
    {
#ifdef _LINUX
        // modify the folder permission to 775
        chmod(logFolderPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP);
#else
        (void)logFolderPath;
#endif
    }

    static void SetLogWritingPermissions(const std::string &logfilePath)
    {
#ifdef _LINUX
        // modify the log file permission to 640
        chmod(logfilePath.c_str(), S_IRUSR | S_IWUSR | S_IRGRP);
#else
        (void)logfilePath;
#endif
    }

    static void SetLogArchivedPermissions()
    {
#ifdef _LINUX
        // modify the log file permission to 440
        chmod(logInstance->filePath.c_str(), S_IRUSR | S_IRGRP);
#endif
    }

    template <typename T, typename... ARGS> inline void Log(const LogLevel &logLevel, const T &t, const ARGS... args)
    {
        if (logLevel < level) {
            return;
        }
        std::string str = GetString(t, args...);
        LogStr(str);
    }

private:
    void Initialize()
    {
        filePath = originFilePath;
        std::string::size_type pos = filePath.find_last_of(".");
        if (pos != std::string::npos) {
            filePath.insert(pos, "_" + std::to_string(++count));
        } else {
            filePath += ("_" + std::to_string(++count));
        }
        if ((this->outType != LogOutType::TERMINAL) && !this->filePath.empty()) {
            this->ofs.open(filePath, std::ofstream::out | std::ofstream::app);
            SetLogWritingPermissions(filePath);
        }
    }

    void Destroy()
    {
        if (ofs.is_open()) {
            ofs.close();
        }
    }

    template <typename T> inline std::string GetString(const T &t)
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    template <typename T, typename... ARGS> inline std::string GetString(const T &t, const ARGS... args)
    {
        std::string str = GetString(t);
        str += GetString(args...);
        return str;
    }

    inline void Check()
    {
        if ((currentSize >= static_cast<unsigned long>(maxSize)) && ofs.is_open()) {
            ofs.close();
            SetLogArchivedPermissions();
            currentSize = 0;
            filePath = originFilePath;
            filePath.insert(filePath.find_last_of("."), "_" + std::to_string(++count));
            ofs.open(filePath, std::ofstream::out | std::ofstream::app);
            SetLogWritingPermissions(filePath);
        }
    }

    inline void Append(const std::string &str)
    {
        if (ofs.is_open()) {
            ofs << str;
            ofs.flush();
            currentSize += str.length();
        }
    }

    inline void LogStr(const std::string &str)
    {
        if (outType == LogOutType::BOTH || outType == LogOutType::TERMINAL) {
            (*outMap[level]) << str;
        }
        if (outType == LogOutType::BOTH || outType == LogOutType::FILE) {
            Check();
            Append(str);
        }
    }

    template <typename T> inline void Log(const T &t)
    {
        std::string str = GetString(t);
        LogStr(str);
    }

    static inline void InitRecordLog()
    {
        if (logInstance == nullptr) {
            std::string logPath = DapParamParser::Instance().GetLogPath();
            logPath += "./dap_server_" + TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_3) + ".log";
            logInstance = std::make_unique<DapLog>(LogOutType::FILE, logPath);
            LogLevel logLevel = GetLogLevel(DapParamParser::Instance().GetLogLevel());
            const int mb = 1024 * 1024;
            int logSize = DapParamParser::Instance().GetLogSize() * mb;
            logInstance->SetLogLevel(logLevel).SetMaxSize(logSize);
        }
    }

    static inline LogLevel GetLogLevel(const std::string &levelValue)
    {
        static std::map<std::string, LogLevel> map = { { "DEBUG", LogLevel::L_DEBUG },
                                                       { "INFO", LogLevel::L_INFO },
                                                       { "WARN", LogLevel::L_WARN },
                                                       { "ERROR", LogLevel::L_ERROR },
                                                       { "FATAL", LogLevel::L_FATAL } };
        LogLevel logLevel = LogLevel::L_DEBUG;
        if (map.count(levelValue) != 0) {
            logLevel = map.at(levelValue);
        }
        return logLevel;
    }

private:
    LogOutType outType = LogOutType::BOTH;
    std::string originFilePath;
    int maxSize = 32 * 1024 * 1024;
    unsigned long currentSize = 0;
    int count = 0;
    std::ofstream ofs;
    std::string filePath;
    LogLevel level = LogLevel::L_INFO;
    std::map<LogLevel, std::ostream *> outMap = { { LogLevel::L_DEBUG, &std::cout },
                                                  { LogLevel::L_INFO, &std::cout },
                                                  { LogLevel::L_WARN, &std::cout },
                                                  { LogLevel::L_ERROR, &std::cerr },
                                                  { LogLevel::L_FATAL, &std::cerr } };
    inline static std::mutex printInstanceMutex;
    inline static std::unique_ptr<DapLog> printInstance = nullptr;
    inline static std::mutex logInstanceMutex;
    inline static std::unique_ptr<DapLog> logInstance = nullptr;
};

inline static const std::string GetCompileFileName(const std::string &fileName)
{
    std::string res;
#ifdef _WIN32
    res = fileName.substr(fileName.find_last_of('\\') + 1);
#else
    res = fileName.substr(fileName.find_last_of('/') + 1);
#endif
    return res;
}

#ifndef FILE_NAME
#define FILE_NAME Dap::GetCompileFileName(__FILE__)
#endif

// call it as PRINT_INFO("print", " info ", " example", " ", 1)
#define PRINT_INFO(...) DapLog::PrintLog(LogLevel::L_INFO, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as PRINT_WARN("print", " warn ", " example", " ", 1)
#define PRINT_WARN(...) DapLog::PrintLog(LogLevel::L_WARN, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as PRINT_ERROR("print", " error ", " example", " ", 1)
#define PRINT_ERROR(...) DapLog::PrintLog(LogLevel::L_ERROR, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as PRINT_FATAL("print", " fatal ", " example", " ", 1)
#define PRINT_FATAL(...) DapLog::PrintLog(LogLevel::L_FATAL, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as PRINT_DEBUG("print", " debug ", " example", " ", 1)
#define PRINT_DEBUG(...) DapLog::PrintLog(LogLevel::L_DEBUG, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

// call it as LOG_INFO("print", " info ", " example", " ", 1)
#define LOG_INFO(...) \
Dap::DapLog::RecordLog(Dap::LogLevel::L_INFO, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as LOG_WARN("print", " warn ", " example", " ", 1)
#define LOG_WARN(...) \
Dap::DapLog::RecordLog(Dap::LogLevel::L_WARN, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as LOG_ERROR("print", " error ", " example", " ", 1)
#define LOG_ERROR(...) \
Dap::DapLog::RecordLog(Dap::LogLevel::L_ERROR, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as LOG_FATAL("print", " fatal ", " example", " ", 1)
#define LOG_FATAL(...) \
Dap::DapLog::RecordLog(Dap::LogLevel::L_FATAL, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// call it as LOG_DEBUG("print", " debug ", " example", " ", 1)
#define LOG_DEBUG(...) \
Dap::DapLog::RecordLog(Dap::LogLevel::L_DEBUG, FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)
} // end of namespace Dap

#endif // DAP_LOG_H