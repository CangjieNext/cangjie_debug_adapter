// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_DATA_BREAKPOINTS_APIHANDLER_H
#define DAP_LLDBAPI_DATA_BREAKPOINTS_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class DataBreakpointInfoApiHandler : public RequestApiHandlerBase {
public:
    explicit DataBreakpointInfoApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~DataBreakpointInfoApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse(std::optional<std::string> dataId = std::nullopt,
        std::optional<std::string> name = std::nullopt, bool supportDataBreakPoint = false);

private:
    Dap::Protocol::DataBreakpointInfoResponse currentResponse;
};
};

#endif // DAP_LLDBAPI_DATA_BREAKPOINTS_APIHANDLER_H
