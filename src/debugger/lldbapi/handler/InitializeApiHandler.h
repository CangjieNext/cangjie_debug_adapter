// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_INITIALIZE_APIHANDLER_H
#define DAP_LLDBAPI_INITIALIZE_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class InitializeApiHandler : public RequestApiHandlerBase {
public:
    explicit InitializeApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~InitializeApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    UniqueResponsePtr GenerateSuccessResponse() const override;

private:
    void ParseParam(const Dap::Protocol::InitializeRequest &req) const;
    const Dap::Protocol::Capabilities GenerateCapabilities() const;
    const std::vector<Dap::Protocol::ExceptionBreakpointsFilter> GenerateExceptionBreakpointsFilters() const;
    void SetDebuggeeLanguageType(const std::string &adapterID) const;
};
};

#endif // DAP_LLDBAPI_INITIALIZE_APIHANDLER_H
