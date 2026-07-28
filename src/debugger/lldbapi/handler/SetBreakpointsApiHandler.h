// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SET_BREAKPOINTS_APIHANDLER_H
#define DAP_LLDBAPI_SET_BREAKPOINTS_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"
#include "BreakPointMap.h"

namespace Dap {
class SetBreakpointsApiHandler : public RequestApiHandlerBase {
public:
    explicit SetBreakpointsApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~SetBreakpointsApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse();
    Dap::Protocol::Breakpoint GenerateBreakpointBase(int64_t id, int64_t line, int64_t column,
        bool verified, const std::string &msg = "");

private:
    int32_t invalidBreakpointId = -1;
    std::string filePath;
    std::vector<Dap::Protocol::Breakpoint> breakpointResult;
    void AddBreakpoints(const std::vector<Dap::Protocol::SourceBreakpoint>& breakpoints);
    void RemoveBreakpoints() const;
};
};

#endif // DAP_LLDBAPI_SET_BREAKPOINTS_APIHANDLER_H
