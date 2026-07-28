// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_LAUNCH_APIHANDLER_H
#define DAP_LLDBAPI_LAUNCH_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class LaunchApiHandler : public RequestApiHandlerBase, public std::enable_shared_from_this<LaunchApiHandler>,
    public LaunchHelper {
public:
    struct TargetParam {
        const char *filename;
        const char *targetTriple = nullptr;
        const char *platformName = nullptr;
        bool addDependentModules = false;
        lldb::SBError sbError;
    } targetParam;
    explicit LaunchApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~LaunchApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

private:
    void ParseParam(const Dap::Protocol::LaunchRequest &launchRequest) const;
    void SendInitializedEvent() const;

    void HandleRunInTerminal(const Dap::Protocol::LaunchRequest& launchRequest);
    void SendRunInTerminal(const Dap::Protocol::LaunchRequest& launchRequest);
    void WaitRunInTerminalResponse() const;
    void ConnectToRemotePlatform(std::string &error);
    UniqueResponsePtr OperateInVMMode() const;
    void SetLaunchInfo(lldb::SBTarget target);

    unsigned int runInTerminalRequestSeq = -1;
    constexpr static int INTERNAL_CONSOLE_WAIT_MORE_MS = 50;
    constexpr static int INTERNAL_CONSOLE_WAIT_MORE_TIMES = 60;
    constexpr static int CONNECT_REMOTE_TRY_COUNT = 3;
    constexpr static int CONNECT_REMOTE_WAIT_MS = 100;
};
};

#endif // DAP_LLDBAPI_LAUNCH_APIHANDLER_H