// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_ATTACH_HELPER_H
#define DAP_SERVER_ATTACH_HELPER_H

#include <string>
#include <vector>
#include "Protocol.h"
#include "DapLog.h"

namespace Dap {
struct AttachParams {
    // debug
    int targetPid = -1;
    std::string program;
    // tuple<0>: ignoreFailures, tuple<1>: command
    std::vector<std::tuple<bool, std::string>> setupCommands;
    std::vector<std::string> scriptCommands;
    bool isContainer = false;
    Dap::Protocol::AttachStep attachStep = Dap::Protocol::AttachStep::ALL_STEPS;
    std::vector<std::string> startupCommands;
    std::vector<std::string> afterConnectCommands;
    std::vector<std::string> postAttachCommands;
    // remote
    RemoteClass remote = RemoteClass::NO_REMOTE;
    std::string remotePlatform;
    std::string remoteAddress;
    // true: remoteAddress is a process-level gdb-remote endpoint (e.g. Apple debugserver), the target must be
    //       connected to it directly ("process connect") and then attached.
    // false: remoteAddress is an lldb-server platform endpoint ("platform connect"), the platform will spawn a
    //        gdb server for us on attach.
    bool connectToDebugServerDirectly = false;
    std::string port;
    std::string remoteUser;
    std::string remoteUserPassword;
    std::string remoteUserKeyFile;
    std::string transportToolPath;

    void Clear();
    std::string ToString() const;
};

class AttachHelper {
public:
    void ParseParameters(const Dap::Protocol::AttachRequest& request);
    // Apple device platforms (iOS/tvOS/watchOS/...) only expose a process-level debugserver, there is no
    // lldb-server platform mode on these devices.
    static bool IsAppleDeviceRemotePlatform(const std::string &remotePlatform);

    AttachParams attachParams;

private:
    void ParseDebugerServerRemoteParam(const Dap::Protocol::AttachRequest &request);

    const std::string PLATFORM_REMOTE_OHOS = "remote-ohos";
    const std::string PLATFORM_REMOTE_HOS = "remote-hos";
};
};

#endif // DAP_SERVER_ATTACH_HELPER_H
