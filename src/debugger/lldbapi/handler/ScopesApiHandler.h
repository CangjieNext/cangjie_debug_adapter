// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SCOPES_APIHANDLER_H
#define DAP_LLDBAPI_SCOPES_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"
#include "LLDB.h"
#include "ContainerManager.h"

namespace Dap {
class ScopesApiHandler : public RequestApiHandlerBase {
public:
    explicit ScopesApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~ScopesApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    Dap::Protocol::Scope GetScope(DapContainer scopeType);
    UniqueResponsePtr GenerateSuccessResponse(std::vector<Dap::Protocol::Scope> &scopes);

private:
    int64_t curFrameId = -1;
    lldb::SBFrame curFrame;
};
};

#endif // DAP_LLDBAPI_SCOPES_APIHANDLER_H
