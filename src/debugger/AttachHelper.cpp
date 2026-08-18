// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "AttachHelper.h"
#include "CommonUtils.h"
#include "EnumUtils.h"

using namespace Dap::Protocol;
using namespace Dap;

void AttachHelper::ParseParameters(const AttachRequest &request)
{11
    attachParams.Clear();
    if (request.arguments.program.has_value()) {
        attachParams.program = request.arguments.program.value();
    }
    attachParams.targetPid = -1;
    if (request.arguments.processId.has_value()) {
        std::string pidStr = request.arguments.processId.value();
        CommonUtils::StringToInt(pidStr, attachParams.targetPid);
    }
    if (request.arguments.setupCommands.has_value()) {
        for (const auto &cmd : request.arguments.setupCommands.value()) {
            attachParams.setupCommands.emplace_back(cmd.ignoreFailures, cmd.text);
        }
    }
    if (request.arguments.scriptCommands.has_value()) {
        attachParams.scriptCommands = request.arguments.scriptCommands.value();
    }
    if (request.arguments.remote.has_value() && request.arguments.remote.value()) {
        attachParams.remote = RemoteClass::DEBUGGER_SERVER_REMOTE;
    } else {
        attachParams.remote = RemoteClass::NO_REMOTE;
    }
    if (attachParams.remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        ParseDebugerServerRemoteParam(request);
    }
    LOG_INFO(attachParams.ToString());
}

void AttachHelper::ParseDebugerServerRemoteParam(const AttachRequest &request)
{
    if (request.arguments.remotePlatform.has_value()) {
        attachParams.remotePlatform = request.arguments.remotePlatform.value();
    }
    if (request.arguments.remoteAddress.has_value()) {
        attachParams.remoteAddress = request.arguments.remoteAddress.value();
    }
    if (request.arguments.attachStep.has_value()) {
        attachParams.attachStep = request.arguments.attachStep.value();
    }
    if (request.arguments.startupCommands.has_value()) {
        attachParams.startupCommands = request.arguments.startupCommands.value();
    }
    if (request.arguments.afterConnectCommands.has_value()) {
        attachParams.afterConnectCommands = request.arguments.afterConnectCommands.value();
    }
    if (request.arguments.postAttachCommands.has_value()) {
        attachParams.postAttachCommands = request.arguments.postAttachCommands.value();
    }
    if (request.arguments.isContainer.has_value()) {
        attachParams.isContainer = request.arguments.isContainer.value();
    }
}

void AttachParams::Clear()
{
    targetPid = -1;
    program.clear();
    remote = RemoteClass::NO_REMOTE;
    attachStep = Dap::Protocol::AttachStep::ALL_STEPS;
    remotePlatform.clear();
    remoteAddress.clear();
    setupCommands.clear();
    scriptCommands.clear();
    startupCommands.clear();
    afterConnectCommands.clear();
    postAttachCommands.clear();
    remoteUser.clear();
    remoteUserPassword.clear();
    remoteUserKeyFile.clear();
    transportToolPath.clear();
}

std::string AttachParams::ToString() const
{
    std::string res;
    res += "Attach params:";
    res += "\n targetPid: " + std::to_string(targetPid);
    if (!program.empty()) {
        res += "\n program: " + program;
    }
    std::string anonymizedRemoteAddress = CommonUtils::AnonymizeRemoteAddress(remoteAddress);
    if (remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        res += "\n remote: DEBUGGER_SERVER_REMOTE";
        res += "\n remotePlatform: " + remotePlatform;
        res += "\n remoteAddress: " + anonymizedRemoteAddress;
        if (isContainer) {
            res += "\n isContainer: true";
        }
    } else {
        res += "\n remote: NO_REMOTE";
    }
    return res;
}
