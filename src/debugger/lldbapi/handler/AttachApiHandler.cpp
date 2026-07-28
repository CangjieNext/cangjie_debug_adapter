// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "AttachApiHandler.h"
#include <thread>
#include "CommonUtils.h"

using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;
using namespace std;

UniqueResponsePtr AttachApiHandler::HandleRequest(UniqueRequestPtr request)
{
    std::string error;
    this->currentRequest = std::move(request);
    auto attachRequest = dynamic_cast<AttachRequest*>(currentRequest.get());
    if (attachRequest == nullptr) {
        error  = "ERROR: AttachRequest pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }

    auto currentOption = debugger->GetOption();
    currentOption.startMode = StartMode::ATTACH_MODE;
    this->ParseParameters(*attachRequest);
    currentOption.remote = attachParams.remote;
    currentOption.remotePlatform = attachParams.remotePlatform;
    if (attachRequest->arguments.showStaticGlobalVars.has_value()) {
        currentOption.showStaticGlobalVars = attachRequest->arguments.showStaticGlobalVars.value();
    }
    debugger->SetOption(currentOption);

    for (const auto &command : attachParams.setupCommands) {
        bool ignoreFailures = get<0>(command);
        bool res = false;
        bool success = debugger->ExecCommand(get<1>(command), error, res);
        if (!ignoreFailures && (!success || !res)) {
            string result = "Failed to execute command: " + get<1>(command) + "; result is " + error;
            return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), result, currentRequest->seq);
        }
    }
    if (attachParams.remote == RemoteClass::NO_REMOTE) {
        return AttachToLocalProcess();
    } else {
        return AttachToRemoteProcess();
    }
}

UniqueResponsePtr AttachApiHandler::AttachToLocalProcess() const
{
    std::string error;
    if (attachParams.targetPid == -1) {
        error = "ERROR: Target Pid is invalid!";
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }
    if (!AttachProcess(error)) {
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }
    InitializedEvent initializedEvent;
    initializedEvent.seq = debugger->GetAndIncCurrentMessageSeq();
    debugger->SendEvent(initializedEvent);
    return debugger->CompleteSuccessResponse(std::make_unique<AttachResponse>(), currentRequest->seq);
}

UniqueResponsePtr AttachApiHandler::AttachToRemoteProcess() const
{
    std::string error;
    if (attachParams.targetPid == -1 && attachParams.program.empty()) {
        error = "ERROR: Target pid and program is invalid!";
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }
    if (attachParams.remotePlatform.empty() || attachParams.remoteAddress.empty()) {
        error  = "ERROR: Remote platform name or remote address is empty!";
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }
    for (const auto &command : attachParams.startupCommands) {
        debugger->ExecCommand(command);
    }
    // select remote platform
    if (attachParams.isContainer) {
        bool res = false;
        bool success = debugger->ExecCommand("platform select " + attachParams.remotePlatform + " inner", error, res);
        SBPlatform platform = debugger->GetSbDebugger().GetSelectedPlatform();
        if (!(success && res && platform.IsValid())) {
            error = "Failed to select platform: " + attachParams.remotePlatform;
            return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
        }
    } else {
        SBPlatform platform(attachParams.remotePlatform.c_str());
        if (!platform.IsValid()) {
            error = "Invalid remote platform: " + attachParams.remotePlatform;
            return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
        }
        debugger->GetSbDebugger().SetSelectedPlatform(platform);
    }
    // connect to remote debugger server
    SBPlatformConnectOptions connectOptions(attachParams.remoteAddress.c_str());
    SBError sbError;
    string anonymizedRemoteAddress = CommonUtils::AnonymizeRemoteAddress(attachParams.remoteAddress);
    for (int i = 0; i < CONNECT_REMOTE_TRY_COUNT; ++i) {
        sbError = debugger->GetSbDebugger().GetSelectedPlatform().ConnectRemote(connectOptions);
        if (sbError.Success()) {
            LOG_INFO("Connected to remote address successfully!");
            break;
        }
        LOG_INFO("Failed connect to " + anonymizedRemoteAddress, ", number of times = ", i);
        if (i < CONNECT_REMOTE_TRY_COUNT - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_REMOTE_WAIT_MS));
        }
    }
    if (!sbError.Success()) {
        error = "Failed connect to " + anonymizedRemoteAddress + ": " + sbError.GetCString();
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }

    for (const auto &command : attachParams.afterConnectCommands) {
        debugger->ExecCommand(command);
    }
    if (!AttachProcess(error)) {
        return debugger->CompleteFailureResponse(std::make_unique<AttachResponse>(), error, currentRequest->seq);
    }
    for (const auto &command : attachParams.postAttachCommands) {
        debugger->ExecCommand(command);
    }
    InitializedEvent initializedEvent;
    initializedEvent.seq = debugger->GetAndIncCurrentMessageSeq();
    debugger->SendEvent(initializedEvent);
    return debugger->CompleteSuccessResponse(std::make_unique<AttachResponse>(), currentRequest->seq);
}

bool AttachApiHandler::AttachProcess(std::string &result) const
{
    SBTarget sbTarget;
    // Ignore the program name, create a target directly
    sbTarget = debugger->GetSbDebugger().CreateTarget(nullptr);
    if (!sbTarget.IsValid()) {
        result = "AttachRequest failed to create target.";
        return false;
    }
    debugger->SetSbTarget(sbTarget);
    debugger->SetDisassemblyManager(DisassemblyManager(sbTarget));
    auto listener = debugger->GetSBListener();
    SBError sbError = SBError();
    if (attachParams.targetPid != -1) {
        sbTarget.AttachToProcessWithID(listener, attachParams.targetPid, sbError);
    } else {
        sbTarget.AttachToProcessWithName(listener, attachParams.program.data(), false, sbError);
    }
    if (sbError.GetCString() != nullptr) {
        result = sbError.GetCString();
        result = "Attach failed: " + result;
        return false;
    }
    LOG_INFO("Attached to process successfully!");
    return true;
}
