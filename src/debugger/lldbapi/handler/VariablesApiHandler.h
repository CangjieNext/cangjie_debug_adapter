// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_VARIABLES_APIHANDLER_H
#define DAP_LLDBAPI_VARIABLES_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class VariablesApiHandler : public RequestApiHandlerBase {
public:
    explicit VariablesApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~VariablesApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    std::vector<Dap::Protocol::Variable> GetLocalVariables(lldb::SBFrame &frame);
    std::vector<Dap::Protocol::Variable> GetStaticVariables(lldb::SBFrame &frame);
    std::vector<Dap::Protocol::Variable> GetGlobalVariables(lldb::SBFrame &frame);
    std::vector<Dap::Protocol::Variable> GetRegisterVariables(lldb::SBFrame &frame);
    std::vector<Dap::Protocol::Variable> GetReturnVariables(lldb::SBFrame &frame);
    std::vector<Dap::Protocol::Variable> GetChildVariables(lldb::SBValue &sbValue, int64_t start, int64_t count);
    std::vector<Dap::Protocol::Variable> GenerateAllVariables(lldb::SBFrame &frame,
                                                        lldb::SBValueList &valueList, lldb::ValueType valueTypeFilter);
    Dap::Protocol::Variable GenerateVariable(lldb::SBValue &sbValue);
    void SetVariableChildInfo(lldb::SBValue &sbValue, Dap::Protocol::Variable &variable, std::string &evaluateName);
    UniqueResponsePtr GenerateSuccessResponse(std::vector<Dap::Protocol::Variable> &variables);

private:
    int64_t curVariableRef = -1;
};
};

#endif // DAP_LLDBAPI_VARIABLES_APIHANDLER_H
