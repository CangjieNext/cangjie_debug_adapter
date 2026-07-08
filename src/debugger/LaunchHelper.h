// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_LAUNCH_HELPER_H
#define DAP_SERVER_LAUNCH_HELPER_H

#include <string>
#include <vector>
#include "Protocol.h"
#include "DapLog.h"
#include "Config.h"

namespace Dap {
struct LaunchParams {
    // debug
    std::string program;
    std::string cwd; // working directory of program
    std::vector<std::string> scriptCommands;
    std::vector<std::string> startupCommands;
    bool debuggeeStarted = false; // if true, use "continue" to resume debuggee, not support runInTerminal
    // remote
    Dap::RemoteClass remote = Dap::RemoteClass::NO_REMOTE;
    std::string remotePlatform;
    std::string remoteAddress;
    std::string port;
    std::string remoteFilePath;
    std::string remoteUser;
    std::string remoteUserPassword;
    std::string remoteUserKeyFile;
    std::string transportToolPath;
    bool remoteConnected = false; // if true, remote already connected, do not need to connect to remote on the server

    bool vmMode = false;
    std::string vmAddress;
    int vmPort = -1;

    void Clear();
    std::string ToString() const;
};

class LaunchHelper {
public:
    LaunchParams launchParams;
    void ParseParameters(const Dap::Protocol::LaunchRequest& request);

private:
    void ParseRemoteSetting(const Protocol::LaunchRequest &request);
};
};

#endif // DAP_SERVER_LAUNCH_HELPER_H