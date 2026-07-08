// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LaunchApiHandler.h"
#include <thread>
#include "PlatformFactory.h"
#include "DapLog.h"
#include "CommonUtils.h"
#include "LLDBApiEventHandler.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr LaunchApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto launchRequest = dynamic_cast<LaunchRequest*>(currentRequest.get());
    std::string error;
    if (launchRequest == nullptr) {
        error = "ERROR: LaunchRequest pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq);
    }
    this->ParseParameters(*launchRequest);
    ParseParam(*launchRequest);

    if (launchParams.vmMode == true) {
        return OperateInVMMode();
    }

    SBDebugger sbDebugger = debugger->GetSbDebugger();
    SBError sbError = SBError();
    if (launchParams.remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        ConnectToRemotePlatform(error);
        if (!error.empty()) {
            return debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq);
        }
        targetParam.filename = launchRequest->arguments.remoteFilePath.value().c_str();
        targetParam.platformName = launchRequest->arguments.remotePlatform.value().c_str();
    } else {
        targetParam.filename = launchRequest->arguments.program.value().c_str();
    }
    targetParam.sbError = sbError;
    SBTarget target = sbDebugger.CreateTarget(targetParam.filename, targetParam.targetTriple,
        targetParam.platformName, targetParam.addDependentModules, targetParam.sbError);
    if (!target.IsValid()) {
        if (sbError.GetCString() != nullptr) {
            LOG_ERROR("LaunchRequest failed to create target: ", sbError.GetCString());
        }
        error = "LaunchRequest failed to create target.";
        return debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq);
    }
    SetLaunchInfo(target);
    debugger->SetSbTarget(target);
    debugger->SetDisassemblyManager(DisassemblyManager(target));
    for (const auto &command : launchParams.startupCommands) {
        debugger->ExecCommand(command);
    }

    if (debugger->GetOption().supportsRunInTerminalRequest && !debugger->GetOption().externalConsole) {
        HandleRunInTerminal(*launchRequest);
        return nullptr;
    } else {
        SendInitializedEvent();
        return debugger->CompleteSuccessResponse(std::make_unique<LaunchResponse>(), currentRequest->seq);
    }
}

void LaunchApiHandler::WaitRunInTerminalResponse() const
{
    RunInTerminalResponse response = debugger->GetRunInTerminalResponse();
    int waitCount = 0;
    while (response.request_seq != runInTerminalRequestSeq) {
        // wait for RunInTerminalResponse again
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERNAL_CONSOLE_WAIT_MORE_MS));
        response = debugger->GetRunInTerminalResponse();
        waitCount++;
        if (waitCount >= INTERNAL_CONSOLE_WAIT_MORE_TIMES) {
            break;
        }
    }
    if (response.request_seq != runInTerminalRequestSeq) {
        string error = "ERROR: runInTerminal response not received !";
        debugger->SendResponse(
            *debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq));
        return;
    }
    if (!response.success) {
        string error;
        if (response.message.has_value()) {
            error = "ERROR(runInTerminal): " + response.message.value();
        } else {
            error = "ERROR: runInTerminal failed!";
        }
        debugger->SendResponse(
            *debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq));
        return;
    }
    if (response.shellProcessId.has_value()) {
#ifdef _WIN32
        lldb::SBLaunchInfo sbLaunchInfo = debugger->GetSbTarget().GetLaunchInfo();
        sbLaunchInfo.SetLaunchFlags(sbLaunchInfo.GetLaunchFlags() | eLaunchFlagDisableSTDIO);
        sbLaunchInfo.AddOpenFileAction(STDIN_FILENO, "CONIN$", true, false);
        sbLaunchInfo.AddOpenFileAction(STDOUT_FILENO, "CONOUT$", false, true);
        sbLaunchInfo.AddOpenFileAction(STDERR_FILENO, "CONOUT$", false, true);
        debugger->GetSbTarget().SetLaunchInfo(sbLaunchInfo);
        // launching a child process attached to another console
        FreeConsole();
        AttachConsole(response.shellProcessId.value());
#else
        std::unique_ptr<Platform> platform = PlatformFactory::Generate();
        string ttyName = platform->GetShellNameByPID(response.shellProcessId.value());
        LOG_INFO("the ttyName of debug terminal shell is ", ttyName);
        lldb::SBLaunchInfo sbLaunchInfo = debugger->GetSbTarget().GetLaunchInfo();
        sbLaunchInfo.AddOpenFileAction(STDIN_FILENO, ttyName.c_str(), true, false);
        sbLaunchInfo.AddOpenFileAction(STDOUT_FILENO, ttyName.c_str(), false, true);
        sbLaunchInfo.AddOpenFileAction(STDERR_FILENO, ttyName.c_str(), false, true);
        debugger->GetSbTarget().SetLaunchInfo(sbLaunchInfo);
#endif
    }
    debugger->SendResponse(
        *debugger->CompleteSuccessResponse(std::make_unique<LaunchResponse>(), currentRequest->seq));
    SendInitializedEvent();
}

void LaunchApiHandler::SendInitializedEvent() const
{
    InitializedEvent initializedEvent;
    initializedEvent.seq = debugger->GetAndIncCurrentMessageSeq();
    debugger->SendEvent(initializedEvent);
}

void LaunchApiHandler::ParseParam(const LaunchRequest &launchRequest) const
{
    // showStaticGlobalVars will be decided by LaunchRequest.arguments in future
    auto currentOption = debugger->GetOption();
    if (launchRequest.arguments.args.has_value()) {
        currentOption.args = launchRequest.arguments.args.value();
    }
    if (launchRequest.arguments.env.has_value()) {
        currentOption.env = launchRequest.arguments.env.value();
    }
    currentOption.showStaticGlobalVars = true;
    if (launchRequest.arguments.remote.has_value() && launchRequest.arguments.remote.value()) {
        currentOption.remote = RemoteClass::DEBUGGER_SERVER_REMOTE;
    }
    currentOption.debuggeePath = currentOption.remote == RemoteClass::DEBUGGER_SERVER_REMOTE
        ? launchRequest.arguments.remoteFilePath.value() : launchRequest.arguments.program.value();
    if (launchRequest.arguments.cwd.has_value()) {
        currentOption.projectDirectory = launchRequest.arguments.cwd.value();
    }
    if (launchRequest.arguments.type.has_value()) {
        currentOption.clientDebugType = launchRequest.arguments.type.value();
    }
    if (launchRequest.arguments.externalConsole.has_value()) {
        currentOption.externalConsole = launchRequest.arguments.externalConsole.value();
    }
    if (launchRequest.arguments.stopAtEntry.has_value() && launchRequest.arguments.stopAtEntry.value()) {
        currentOption.stopAtEntry = launchRequest.arguments.stopAtEntry.value();
    }
    currentOption.launchParams = launchParams;
    debugger->SetOption(currentOption);
}

void LaunchApiHandler::HandleRunInTerminal(const LaunchRequest &launchRequest)
{
    SendRunInTerminal(launchRequest);
    // Return quickly by using thread, to receive runInTerminal response
    std::thread t = std::thread(std::mem_fn(&LaunchApiHandler::WaitRunInTerminalResponse), shared_from_this());
    t.detach();
    // SendResponse in WaitRunInTerminalResponse()
}

void LaunchApiHandler::SendRunInTerminal(const LaunchRequest &launchRequest)
{
    (void)launchRequest;
    RunInTerminalRequest request;
    request.seq = debugger->GetAndIncCurrentMessageSeq();
    runInTerminalRequestSeq = request.seq;
    request.arguments.kind = RunInTerminalKind::INTEGRATED;
    request.arguments.cwd = debugger->GetOption().projectDirectory;
    request.arguments.title =
            debugger->GetOption().clientDebugType + ": " + CommonUtils::GetFileName(debugger->GetOption().debuggeePath);
    request.arguments.env = std::vector<EnvPair>();
    debugger->SendRequest(request);
}

void LaunchApiHandler::ConnectToRemotePlatform(std::string &error)
{
    if (launchParams.remotePlatform.empty() || launchParams.remoteAddress.empty()) {
        error  = "ERROR: Remote platform name or remote address is empty!";
        return;
    }
    SBPlatform platform(launchParams.remotePlatform.c_str());
    if (!platform.IsValid()) {
        error = "Invalid remote platform: " + launchParams.remotePlatform;
        return;
    }
    debugger->GetSbDebugger().SetSelectedPlatform(platform);
    SBPlatformConnectOptions connectOptions(launchParams.remoteAddress.c_str());
    SBError sbError;
    for (int i = 0; i < CONNECT_REMOTE_TRY_COUNT; ++i) {
        sbError = platform.ConnectRemote(connectOptions);
        if (sbError.Success()) {
            LOG_INFO("Connected to remote address successfully!");
            break;
        }
        LOG_INFO("Failed connect to " + launchParams.remoteAddress, ", number of times = ", i);
        if (i < CONNECT_REMOTE_TRY_COUNT - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_REMOTE_WAIT_MS));
        }
    }
    if (!sbError.Success()) {
        error = "Failed connect to " + launchParams.remoteAddress + ": " + sbError.GetCString();
        return;
    }
    SBFileSpec localSbFileSpec(launchParams.program.c_str());
    SBFileSpec remoteSbFileSpec(launchParams.remoteFilePath.c_str());
    sbError = platform.Put(localSbFileSpec, remoteSbFileSpec);
    if (!sbError.Success()) {
        error = "Failed to push debuggee binary to remote platform: " +
            launchParams.remoteAddress + sbError.GetCString();
    }
    for (const auto &command : launchParams.scriptCommands) {
        debugger->ExecCommand(command);
    }
}

UniqueResponsePtr LaunchApiHandler::OperateInVMMode() const
{
    string error;
    if (launchParams.vmAddress.empty() || launchParams.vmPort == -1) {
        error = "ERROR: vmAddress or vmPort is empty on VM Mode!";
        return debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq);
    }

    SBTarget sbTarget = debugger->GetSbDebugger().CreateTarget(nullptr);
    SBError sbError;
    SBListener sbListener = debugger->GetSBListener();
    string url = launchParams.vmAddress + ":" + std::to_string(launchParams.vmPort);
    sbTarget.ConnectRemote(sbListener, url.c_str(), nullptr, sbError);
    if (!sbError.Success()) {
        error = "Failed connect to " + url + ", " + sbError.GetCString();
        return debugger->CompleteFailureResponse(std::make_unique<LaunchResponse>(), error, currentRequest->seq);
    }
    debugger->SetSbTarget(sbTarget);
    debugger->SetDisassemblyManager(DisassemblyManager(sbTarget));

    for (const auto &command : launchParams.scriptCommands) {
        debugger->ExecCommand(command);
    }
    SendInitializedEvent();
    return debugger->CompleteSuccessResponse(std::make_unique<LaunchResponse>(), currentRequest->seq);
}

// Set parameters and environment variables for program startup
void LaunchApiHandler::SetLaunchInfo(SBTarget target)
{
    lldb::SBLaunchInfo sbLaunchInfo = target.GetLaunchInfo();
    sbLaunchInfo.SetArguments(nullptr, false);
    if (!debugger->GetOption().args.empty()) {
        for (const auto& arg : debugger->GetOption().args) {
            const char *argv[2] = {arg.c_str(), nullptr};
            sbLaunchInfo.SetArguments(argv, true);
        }
    }
    if (!debugger->GetOption().env.empty()) {
        std::vector<string> envs;
        for (const auto& it : debugger->GetOption().env) {
            envs.push_back(it.first + "=" + it.second);
        }
        int envNums = envs.size();
        const char* env[envNums + 1];
        for (int i = 0; i < envNums; ++i) {
            env[i] = envs[i].c_str();
        }
        env[envNums] = nullptr;
        sbLaunchInfo.SetEnvironmentEntries(env, true);
    }
    target.SetLaunchInfo(sbLaunchInfo);
}