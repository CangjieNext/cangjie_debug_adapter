// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_RESETSOURCEMAPAPIHANDLER_H
#define DAP_SERVER_RESETSOURCEMAPAPIHANDLER_H

#include <utility>

#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
const std::string CLEAR_SOURCE_MAP_COMMAND = "settings clear target.source-map";

class ResetSourceMapApiHandler : public RequestApiHandlerBase {
public:
    explicit ResetSourceMapApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(
        std::move(apiDebugger)) {};
    ~ResetSourceMapApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;
private:
    bool ResetSourceMap(const std::unordered_map<std::string, std::string>& sourceMap);
    std::string error;
};
}

#endif // DAP_SERVER_RESETSOURCEMAPAPIHANDLER_H
