// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_ATTACH_API_HANDLER_H
#define DAP_SERVER_ATTACH_API_HANDLER_H

#include "LLDBApiDebugger.h"
#include "RequestApiHandlerBase.h"
#include "AttachHelper.h"

namespace Dap {
class AttachApiHandler : public RequestApiHandlerBase, public AttachHelper {
public:
    explicit AttachApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~AttachApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

private:
    UniqueResponsePtr AttachToLocalProcess() const;
    UniqueResponsePtr AttachToRemoteProcess() const;
    UniqueResponsePtr AttachViaRemotePlatform() const;
    UniqueResponsePtr AttachViaRemoteDebugServer() const;
    UniqueResponsePtr FinishRemoteAttach(lldb::SBTarget &sbTarget) const;
    bool SelectRemotePlatform(std::string &error) const;
    bool CreateAttachTarget(lldb::SBTarget &sbTarget, std::string &result) const;
    bool AttachToTarget(lldb::SBTarget &sbTarget, std::string &result) const;
    bool AttachProcess(std::string &result) const;

    constexpr static int CONNECT_REMOTE_TRY_COUNT = 50;
    constexpr static int CONNECT_REMOTE_WAIT_MS = 100;
};
};

#endif // DAP_SERVER_ATTACH_API_HANDLER_H
