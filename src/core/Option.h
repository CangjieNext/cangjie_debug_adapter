// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_OPTION_H
#define DAP_SERVER_OPTION_H

#include <string>
#include "Config.h"

namespace Dap {
struct Option {
    DebugSessionMode debugSessionMode;
    StartMode startMode;
    PlatformType platform;
    std::string debuggerPath;
    std::string debuggeePath;
    std::string exeParams;
    std::string hostIP;
    std::string logPath;
    int port;
};
}

#endif // DAP_SERVER_OPTION_H
