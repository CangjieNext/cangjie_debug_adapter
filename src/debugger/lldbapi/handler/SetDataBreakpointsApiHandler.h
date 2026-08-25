// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SET_DATA_BREAKPOINTS_APIHANDLER_H
#define DAP_LLDBAPI_SET_DATA_BREAKPOINTS_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class SetDataBreakpointsApiHandler : public RequestApiHandlerBase {
public:
    explicit SetDataBreakpointsApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~SetDataBreakpointsApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse();
private:
    int32_t invalidBreakpointId = -1;
    std::vector<Dap::Protocol::Breakpoint> breakpointResult;
    void GenerateFailureBreakpointResult(Dap::Protocol::Breakpoint& currentBreak, std::string message);
    void AddBreakpoints(const std::vector<Dap::Protocol::DataBreakpoint> &breakpoints);
    bool IsConditionUnsupported(const std::string &condition);
    bool ParseDataId(const std::string &dataId, lldb::addr_t &address, size_t &size);
    lldb::watch_id_t CreateWatchpoint(const lldb::SBTarget &sbTarget, lldb::addr_t address, size_t size,
        bool read, bool write, const Dap::Protocol::DataBreakpoint &dataBreakpoint,
        Dap::Protocol::Breakpoint &currentBreak);
    bool ParseAddressAndSize(const std::vector<std::string> &addrAndSize, lldb::addr_t &address, size_t &size);
    void RemoveBreakpoints() const;
};
};

#endif // DAP_LLDBAPI_SET_DATA_BREAKPOINTS_APIHANDLER_H
