// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_EVALUATE_APIHANDLER_H
#define DAP_LLDBAPI_EVALUATE_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class EvaluateApiHandler : public RequestApiHandlerBase {
public:
    explicit EvaluateApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~EvaluateApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;
    int GetNumOfVariableChildren(lldb::SBValue &sbValue);

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse(lldb::SBValue &sbValue, std::string &expression, std::string &result);
    lldb::SBFrame GetFrameForEvaluation(lldb::SBProcess &sbProcess, int64_t frameId);
    UniqueResponsePtr HandleEvaluateValue(Protocol::EvaluateRequest *reqPtr,
        lldb::SBValue &value, std::string &expression);
};
};

#endif // DAP_LLDBAPI_EVALUATE_APIHANDLER_H
