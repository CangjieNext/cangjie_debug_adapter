// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SOURCE_APIHANDLER_H
#define DAP_LLDBAPI_SOURCE_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class SourceApiHandler : public RequestApiHandlerBase {
public:
    explicit SourceApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~SourceApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse(int64_t sourceRef);
};
};

#endif // DAP_LLDBAPI_SOURCE_APIHANDLER_H
