// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_ADDSOURCEMAPAPIHANDLER_H
#define DAP_SERVER_ADDSOURCEMAPAPIHANDLER_H

#include <utility>

#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
const std::string SET_SOURCE_MAP_COMMAND = "settings set target.source-map ";
const std::string SHOW_SOURCE_MAP_COMMAND = "settings show target.source-map";
const std::string OH_TOOLCHAIN_PATH_PATTERN = "/home/openharmony/llvm.*/toolchain/";
const std::unordered_set<short> sourceMappingSupportLanguageType = { lldb::eLanguageTypeC_plus_plus,
                                                                     lldb::eLanguageTypeC_plus_plus_03,
                                                                     lldb::eLanguageTypeC_plus_plus_11,
                                                                     lldb::eLanguageTypeC_plus_plus_14 };


class AddSourceMapApiHandler : public RequestApiHandlerBase {
public:
    explicit AddSourceMapApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(
            std::move(apiDebugger)) {};
    ~AddSourceMapApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;
private:
    static std::string ExtractCompiledRootDirPath(lldb::SBModule &sbModule);
    bool SetSourceMap(const std::unordered_map<std::string, std::string>& sourceMap);
    bool IsValidPath(const std::string &path) const;
    std::string error;
};
}

#endif // DAP_SERVER_ADDSOURCEMAPAPIHANDLER_H
