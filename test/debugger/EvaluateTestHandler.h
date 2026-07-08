// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_EVALUATE_TEST_HANDLER_H
#define DAP_SERVER_EVALUATE_TEST_HANDLER_H

#include "EvaluateHandler.h"

class EvaluateTestHandler : public Dap::EvaluateHandler {
public:
    explicit EvaluateTestHandler(std::shared_ptr<Dap::MIDebugger> debugger) : EvaluateHandler(debugger) {}
    ~EvaluateTestHandler() override = default;

    void HandleRequestTest(std::unique_ptr<Dap::Protocol::Request> request)
    {
        EvaluateHandler::HandleRequest(std::move(request));
    }

    void HandleResultTest(const Dap::MIResultRecord &result)
    {
        EvaluateHandler::HandleResult(result);
    }

    bool CheckMIResultRecordTest(const Dap::MIResultRecord &result)
    {
        return EvaluateHandler::CheckMIResultRecord(result);
    }

    Dap::EvaluateResultType GetEvaluateResultTypeTest()
    {
        return EvaluateHandler::GetEvaluateResultType();
    }

    bool IsArrayTest(const std::string value, const std::string type)
    {
        return EvaluateHandler::IsArray(value, type);
    }

    void SetResultOfResponseTest(Dap::Protocol::EvaluateResponse &res)
    {
        return EvaluateHandler::SetResultOfResponse(res);
    }

    void SetEvaluateStatus(const Dap::EvaluateStatus &status)
    {
        this->evaluateStatus = status;
    }

    void SetEvaluateResultType(const Dap::EvaluateResultType &type)
    {
        this->evaluateResultType = type;
    }

    void AddMiResult(const std::string &key, const std::string &value)
    {
        this->miResultMap[key] = value;
    }
};

#endif // DAP_SERVER_EVALUATE_TEST_HANDLER_H