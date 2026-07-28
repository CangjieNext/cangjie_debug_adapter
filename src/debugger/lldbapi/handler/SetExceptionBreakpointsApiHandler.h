// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SET_EXCEPTION_BREAKPOINTS_APIHANDLER_H
#define DAP_LLDBAPI_SET_EXCEPTION_BREAKPOINTS_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class SetExceptionBreakpointsApiHandler : public RequestApiHandlerBase {
public:
    explicit SetExceptionBreakpointsApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~SetExceptionBreakpointsApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse();

private:
    const lldb::LanguageType LANGUAGE_TYPE_C_PLUS_PLUS = lldb::eLanguageTypeC_plus_plus;
    std::vector<Dap::Protocol::Breakpoint> breakpointResult;
    void AddBreakpoints(const std::vector<std::string> &filters);
    void RemoveBreakpoints() const;
    bool IsFiltersCorrect(const std::vector<std::string> &filters) const;
};
};

#endif // DAP_LLDBAPI_SET_EXCEPTION_BREAKPOINTS_APIHANDLER_H
