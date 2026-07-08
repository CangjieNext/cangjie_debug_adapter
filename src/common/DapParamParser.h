// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PARAM_PARSER_H
#define DAP_PARAM_PARSER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "Option.h"

namespace Dap {
class DapParamParser {
public:
    static DapParamParser &Instance();
    virtual bool ParseDebugOption(const std::vector<std::string> &args, Option &option);
    // get one key-value by parse once
    bool ParseField(const std::string &data);
    // return -1 when no usable port
    int GetPort() const;
    std::string GetLogPath() const;
    std::string GetLogLevel() const;
    bool GetLogEnable() const;
    // MB
    int GetLogSize() const;
    std::string GetLinkType() const;
    std::string Error() const;
    void SetLogPath(const std::string &logPath);
    void Clear();

public:
    const static std::string SYMBOL_PORT;
    const static std::string SYMBOL_LOG_PATH;
    const static std::string SYMBOL_LINK_TYPE;     // std or socket
    const static std::string SYMBOL_LOG_SIZE;   // MB
    const static std::string SYMBOL_LOG_LEVEL; // FATAL/ERROR/WARN/INFO/DEBUG
    const static std::string SYMBOL_LOG_ENABLE;
    const static int MIN_PORT_NUM = 0;
    const static int MAX_PORT_NUM = 65535;
    const static int ERROR_PORT = -1;
    const static std::string LINK_TYPE_STD;
    const static std::string LINK_TYPE_SOCKET;

protected:
    std::string error;
    DapParamParser() = default;
    ~DapParamParser() = default;

private:
    bool CheckDirPath(const std::string &dirPath);
    bool CheckPortString(const std::string &portStr);
    bool CheckBoolString(const std::string &str);

    bool ParsePort(const std::string &portStr);
    bool ParseLogPath(const std::string &logPath);
    bool ParseLinkType(const std::string &linkType);
    bool ParseLogSize(const std::string &logSize);
    bool ParseLogLevel(const std::string &logLevel);
    bool ParseLogEnable(const std::string &logEnable);

private:
    std::map<std::string, std::string> paramMap;
};
}
#endif // DAP_PARAM_PARSER_H
