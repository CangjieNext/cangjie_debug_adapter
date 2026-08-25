// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_BREAKPOINTLOCATIONSAPIHANDLER_H
#define DAP_SERVER_BREAKPOINTLOCATIONSAPIHANDLER_H

#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class BreakpointLocationsApiHandler : public RequestApiHandlerBase {
public:
    explicit BreakpointLocationsApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~BreakpointLocationsApiHandler() override = default;

    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    std::vector<Protocol::BreakpointLocation> GenBreakpointLocations();
    bool LocationIsValid(const Protocol::BreakpointLocation &location, const std::string& inc = "");
    std::string GetIncOfSBAddress(const lldb::SBAddress& sbAddress);
    lldb::SBAddress GetSBAddressByLocation(const std::string &filePath, int64_t line, int64_t column = -1);
    Protocol::BreakpointLocation GenBkptLocationOfSBAddress(lldb::SBAddress sbAddress);

    const std::set<std::string> stepInInc = { "bl", "blr", "blx", "BL", "BLR", "BLX" };
    bool filterStepinLocation = false;
    int64_t proLine = 0; // cache line number of last instruction code to adapt line number follow
};
}

#endif // DAP_SERVER_BREAKPOINTLOCATIONSAPIHANDLER_H
