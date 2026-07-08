// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DapParamParser.h"
#include "CommonUtils.h"
#include "DapLog.h"

using namespace Dap;

const std::string DapParamParser::SYMBOL_PORT = "--port=";
const std::string DapParamParser::SYMBOL_LOG_PATH = "--logpath=";
const std::string DapParamParser::SYMBOL_LINK_TYPE = "--link=";     // std or socket
const std::string DapParamParser::SYMBOL_LOG_SIZE = "--logsize=";   // MB
const std::string DapParamParser::SYMBOL_LOG_LEVEL = "--loglevel="; // FATAL/ERROR/WARN/INFO/DEBUG
const std::string DapParamParser::SYMBOL_LOG_ENABLE = "--logEnable=";
const std::string DapParamParser::LINK_TYPE_STD = "std";
const std::string DapParamParser::LINK_TYPE_SOCKET = "socket";

DapParamParser &DapParamParser::Instance()
{
    static DapParamParser instance;
    return instance;
}

bool DapParamParser::ParsePort(const std::string &portStr)
{
    if (!CheckPortString(portStr)) {
        error = "ERROR: Port error, port range is " + std::to_string(MIN_PORT_NUM) + "-" +
            std::to_string(MAX_PORT_NUM) + ".";
        return false;
    }
    paramMap[SYMBOL_PORT] = portStr;
    return true;
}

bool DapParamParser::ParseLogPath(const std::string &logPath)
{
    std::string logPathTmp = logPath;
    if (!CommonUtils::NormalizeFilePath(logPathTmp, error)) {
        error = "ERROR: log " + error;
        return false;
    }
    if (!CheckDirPath(logPathTmp) || !CommonUtils::CreateFolderRecursively(logPathTmp)) {
        error = "ERROR: log path has illegal characters or is created failed.";
        return false;
    }
    Dap::DapLog::SetLogFolderPermissions(logPathTmp);
    if (!logPathTmp.empty() && (logPathTmp.at(logPathTmp.length() - 1) != '/')) {
        logPathTmp += '/';
    }
    paramMap[SYMBOL_LOG_PATH] = logPathTmp;
    return true;
}

bool DapParamParser::ParseLinkType(const std::string &linkType)
{
    paramMap[SYMBOL_LINK_TYPE] = linkType;
    return true;
}

bool DapParamParser::ParseLogSize(const std::string &logSize)
{
    int size = 0;
    if (!CommonUtils::StringToInt(logSize, size) || size <= 0) {
        error = "ERROR: log size is invalid!";
        return false;
    }
    paramMap[SYMBOL_LOG_SIZE] = logSize;
    return true;
}

bool DapParamParser::ParseLogLevel(const std::string &logLevel)
{
    paramMap[SYMBOL_LOG_LEVEL] = logLevel;
    return true;
}

bool DapParamParser::ParseLogEnable(const std::string &logEnable)
{
    paramMap[SYMBOL_LOG_ENABLE] = logEnable;
    return true;
}

bool DapParamParser::ParseDebugOption(const std::vector<std::string> &args, Option &option)
{
    if (args.size() <= 1) {
        error = "ERROR: Startup parameter count is not enough.";
        std::cerr << error << std::endl;
        return false;
    }
    for (unsigned int i = 1; i < args.size(); i++) {
        if (!this->ParseField(args.at(i))) {
            std::cerr << error << std::endl;
            return false;
        }
    }
    int port = this->GetPort();
    if (port == -1) {
        error = "ERROR: Dap server port parameters invalid.";
        return false;
    } else {
        option.port = port;
    }
    option.logPath = this->GetLogPath();
    if (option.logPath.empty()) {
        option.logPath = "./";
    }
    option.debugSessionMode = DebugSessionMode::SINGLE;
    option.hostIP = "127.0.0.1";
    return true;
}

// get one key-value by parse once
bool DapParamParser::ParseField(const std::string &data)
{
    std::string::size_type pos = data.find(SYMBOL_PORT);
    if (pos != std::string::npos) {
        return ParsePort(data.substr(pos + SYMBOL_PORT.length()));
    }
    pos = data.find(SYMBOL_LOG_PATH);
    if (pos != std::string::npos) {
        return ParseLogPath(data.substr(pos + SYMBOL_LOG_PATH.length()));
    }
    pos = data.find(SYMBOL_LINK_TYPE);
    if (pos != std::string::npos) {
        return ParseLinkType(data.substr(pos + SYMBOL_LINK_TYPE.length()));
    }
    pos = data.find(SYMBOL_LOG_LEVEL);
    if (pos != std::string::npos) {
        return ParseLogLevel(data.substr(pos + SYMBOL_LOG_LEVEL.length()));
    }
    pos = data.find(SYMBOL_LOG_ENABLE);
    if (pos != std::string::npos) {
        return ParseLogEnable(data.substr(pos + SYMBOL_LOG_ENABLE.length()));
    }
    pos = data.find(SYMBOL_LOG_SIZE);
    if (pos != std::string::npos) {
        return ParseLogSize(data.substr(pos + SYMBOL_LOG_SIZE.length()));
    }
    LOG_WARN(data, " has not been supported.");
    return true;
}

int DapParamParser::GetPort() const
{
    if (paramMap.count(SYMBOL_PORT) == 0) {
        return ERROR_PORT;
    }
    return std::stoi(paramMap.at(SYMBOL_PORT));
}

std::string DapParamParser::GetLogPath() const
{
    if (paramMap.count(SYMBOL_LOG_PATH) == 0) {
        return "";
    }
    return paramMap.at(SYMBOL_LOG_PATH);
}

void DapParamParser::SetLogPath(const std::string &logPath)
{
    paramMap[SYMBOL_LOG_PATH] = logPath;
}

std::string DapParamParser::GetLogLevel() const
{
    if (paramMap.count(SYMBOL_LOG_LEVEL) == 0) {
        return "INFO";
    }
    return paramMap.at(SYMBOL_LOG_LEVEL);
}

bool DapParamParser::GetLogEnable() const
{
    if (paramMap.count(SYMBOL_LOG_ENABLE) == 0) {
        return true;
    }
    return std::equal(paramMap.at(SYMBOL_LOG_ENABLE).begin(), paramMap.at(SYMBOL_LOG_ENABLE).end(), "1");
}

int DapParamParser::GetLogSize() const
{
    const int constSize = 32; // MB
    if (paramMap.count(SYMBOL_LOG_SIZE) == 0) {
        return constSize;
    }
    std::string logSize = paramMap.at(SYMBOL_LOG_SIZE);
    int res = 0;
    if (!CommonUtils::StringToInt(logSize, res)) {
        return constSize;
    }
    return res;
}

std::string DapParamParser::GetLinkType() const
{
    if (paramMap.count(SYMBOL_LINK_TYPE) == 0) {
        return LINK_TYPE_STD; // default
    }
    return paramMap.at(SYMBOL_LINK_TYPE);
}

std::string DapParamParser::Error() const
{
    return error;
}

void DapParamParser::Clear()
{
    paramMap.clear();
}

bool DapParamParser::CheckDirPath(const std::string &dirPath)
{
    if (dirPath.empty()) {
        error = "ERROR: directory path is empty.";
        return false;
    }
#ifdef _WIN32
    if (CommonUtils::RegexSearch(dirPath, "[*?\"<>|]")) {
        error = "ERROR: " + dirPath + " has illegal characters.";
        return false;
    }
#endif
    return true;
}

bool DapParamParser::CheckPortString(const std::string &portStr)
{
    int port;
    try {
        port = std::stoi(portStr);
        if ((port < MIN_PORT_NUM) || (port > MAX_PORT_NUM)) {
            return false;
        }
    } catch (std::invalid_argument &) {
        // no conversion
        return false;
    } catch (std::out_of_range &) {
        // out of range
        return false;
    }
    return true;
}

bool DapParamParser::CheckBoolString(const std::string &str)
{
    int num;
    try {
        num = std::stoi(str);
        if (num > 1 || num < 0) {
            return false;
        }
    } catch (std::invalid_argument &) {
        // no conversion
        return false;
    } catch (std::out_of_range &) {
        // out of range
        return false;
    }
    return true;
}