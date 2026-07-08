// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LaunchHelper.h"
#include "CommonUtils.h"
#include "EnumUtils.h"

using namespace Dap::Protocol;
using namespace Dap;

void LaunchHelper::ParseParameters(const LaunchRequest &request)
{
    launchParams.Clear();
    if (request.arguments.program.has_value()) {
        launchParams.program = request.arguments.program.value();
    }
    if (request.arguments.cwd.has_value()) {
        launchParams.cwd = request.arguments.cwd.value();
    }
    if (request.arguments.scriptCommands.has_value()) {
        launchParams.scriptCommands = request.arguments.scriptCommands.value();
    }
    if (request.arguments.startupCommands.has_value()) {
        launchParams.startupCommands = request.arguments.startupCommands.value();
    }
    if (request.arguments.debuggeeStarted.has_value()) {
        launchParams.debuggeeStarted = request.arguments.debuggeeStarted.value();
    }
    if (request.arguments.remoteConnected.has_value()) {
        launchParams.remoteConnected = request.arguments.remoteConnected.value();
    }
    if (request.arguments.vmMode.has_value()) {
        launchParams.vmMode = request.arguments.vmMode.value();
        launchParams.vmAddress = request.arguments.vmAddress.has_value() ? request.arguments.vmAddress.value() : "";
        launchParams.vmPort = request.arguments.vmPort.has_value() ? request.arguments.vmPort.value() : -1;
        launchParams.debuggeeStarted = launchParams.vmMode;
    }
    ParseRemoteSetting(request);
    LOG_INFO(launchParams.ToString());
}

void LaunchHelper::ParseRemoteSetting(const LaunchRequest &request)
{
    if (request.arguments.remote.has_value() && request.arguments.remote.value()) {
        launchParams.remote = RemoteClass::DEBUGGER_SERVER_REMOTE;
    } else {
        launchParams.remote = RemoteClass::NO_REMOTE;
    }
    if (launchParams.remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        if (request.arguments.remotePlatform.has_value()) {
            launchParams.remotePlatform = request.arguments.remotePlatform.value();
        }
        if (request.arguments.remoteAddress.has_value()) {
            launchParams.remoteAddress = request.arguments.remoteAddress.value();
        }
        if (request.arguments.remoteFilePath.has_value()) {
            launchParams.remoteFilePath = request.arguments.remoteFilePath.value();
        }
    }
}

void LaunchParams::Clear()
{
    program.clear();
    remote = RemoteClass::NO_REMOTE;
    remoteFilePath.clear();
    remotePlatform.clear();
    remoteAddress.clear();
    scriptCommands.clear();
    startupCommands.clear();
    remoteUser.clear();
    remoteUserPassword.clear();
    remoteUserKeyFile.clear();
    transportToolPath.clear();
}

std::string LaunchParams::ToString() const
{
    std::string res;
    res += "Launch params:";
    if (!program.empty()) {
        res += "\n program: " + program;
    }
    if (remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        res += "\n remote: DEBUGGER_SERVER_REMOTE";
        res += "\n remotePlatform: " + remotePlatform;
        res += "\n remoteAddress: " + remoteAddress;
    } else {
        res += "\n remote: NO_REMOTE";
    }
    if (vmMode) {
        res += "\n VMMode: on";
        res += "\n vmAddress: " + vmAddress;
        res += "\n vmPort: " + std::to_string(vmPort);
    }
    return res;
}
