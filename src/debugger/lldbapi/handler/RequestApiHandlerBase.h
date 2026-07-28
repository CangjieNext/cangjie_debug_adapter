// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_REQUEST_HANDLER_BASE_H
#define DAP_LLDBAPI_REQUEST_HANDLER_BASE_H

#include <memory>
#include <string>
#include "LLDBApiDebugger.h"
#include "Protocol.h"
#include "DapLog.h"

namespace Dap {
class RequestApiHandlerBase {
public:
    explicit RequestApiHandlerBase(std::shared_ptr<LLDBApiDebugger> apiDebugger) : debugger(apiDebugger) {}
    virtual ~RequestApiHandlerBase() = default;

    virtual UniqueResponsePtr HandleRequest(UniqueRequestPtr request) = 0;

protected:
    virtual UniqueResponsePtr GenerateSuccessResponse() const
    {
        return nullptr;
    }

protected:
    std::shared_ptr<LLDBApiDebugger> debugger = nullptr;
    UniqueRequestPtr currentRequest = nullptr;
};
};

#endif // DAP_LLDBAPI_REQUEST_HANDLER_BASE_H
