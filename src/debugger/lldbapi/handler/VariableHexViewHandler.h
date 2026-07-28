// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_VARIABLE_HEX_VIEW_HANDLER_H
#define DAP_VARIABLE_HEX_VIEW_HANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class VariableHexViewHandler : public RequestApiHandlerBase {
public:
    explicit VariableHexViewHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~VariableHexViewHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;
private:
    bool TypeFormatAdd(const std::string &dstType, const std::vector<std::string> &srcTypes);
    bool TypeFormatClear();
    bool IsValidTypeName(const std::string &typeName) const;
private:
    std::string error;
};
};

#endif // DAP_VARIABLE_HEX_VIEW_HANDLER_H
