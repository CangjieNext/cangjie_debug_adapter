// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_SET_VARIABLE_APIHANDLER_H
#define DAP_LLDBAPI_SET_VARIABLE_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"
#include "ValueParser.h"

namespace Dap {
class SetVariableApiHandler : public RequestApiHandlerBase {
public:
    explicit SetVariableApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~SetVariableApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse(int64_t parentIndex, lldb::SBValue &sbValue);
    Dap::ValueParser GetExprValueParser(lldb::SBValue &value, std::string &expression) const;
    std::string FormatCharValueExpr(const std::string &expression, bool isUnsigned) const;
    bool IsSimpleLiteral(const std::string &expression, lldb::BasicType basicType) const;

private:
    constexpr static int CHAR_RANGE = 256;
};
};

#endif // DAP_LLDBAPI_SET_VARIABLE_APIHANDLER_H
