// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetFunctionBreakpointsApiHandler.h"
#include "DapLog.h"
#include "CommonUtils.h"
#include "FunctionBreakPointMap.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetFunctionBreakpointsApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetFunctionBreakpointsRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        std::string error = "ERROR: SetFunctionBreakpointsApiHandler Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetFunctionBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    this->RemoveBreakpoints();
    SBTarget sbTarget = debugger->GetSbTarget();
    if (!reqPtr->arguments.breakpoints.empty()) {
        this->AddBreakpoints(reqPtr->arguments.breakpoints);
    }
    return GenerateSuccessResponse();
}

void SetFunctionBreakpointsApiHandler::AddBreakpoints(const std::vector<FunctionBreakpoint> &breakpoints)
{
    SBTarget sbTarget = debugger->GetSbTarget();
    FunctionBreakPointMap &map = debugger->GetFunctionBreakpointMap();
    for (const auto &breakpoint : breakpoints) {
        SBBreakpoint sbBreakpoint;
        std::string function = CommonUtils::ReplaceString(breakpoint.name, "\\", "/");
        std::string condition = breakpoint.condition.has_value() ? breakpoint.condition.value() : "";
#ifdef _DARWIN
        if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(GenerateBreakpointBase(invalidBreakpointId, breakpoint.name, false, msg));
            continue;
        }
#else
        if (debugger->GetOption().OhosCangjieDebug() && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(GenerateBreakpointBase(invalidBreakpointId, breakpoint.name, false, msg));
            continue;
        }
#endif
        if (breakpoint.moduleName.has_value()) {
            std::string module = breakpoint.moduleName.value();
            sbBreakpoint = sbTarget.BreakpointCreateByName(function.c_str(), module.c_str());
        } else {
            sbBreakpoint = sbTarget.BreakpointCreateByName(function.c_str());
        }
        int hitCount = 0;
        if (breakpoint.hitOnce.has_value() && breakpoint.hitOnce.value()) {
            sbBreakpoint.SetOneShot(true);
        }
        if (breakpoint.hitCondition.has_value()) {
            CommonUtils::StringToInt(breakpoint.hitCondition.value(), hitCount);
        }
        if (hitCount > 0) {
            sbBreakpoint.SetIgnoreCount(hitCount - 1);
        }
        if (!condition.empty()) {
            sbBreakpoint.SetCondition(breakpoint.condition->data());
        }
        Breakpoint currentBreak = GenerateBreakpointBase(sbBreakpoint.GetID(), breakpoint.name,
            sbBreakpoint.IsValid() && sbBreakpoint.GetNumLocations() > 0);
        if (currentBreak.id.has_value()) {
            map.AddBreakPoint(breakpoint.name, currentBreak.id.value());
        }
        breakpointResult.emplace_back(currentBreak);
    }
}

std::unique_ptr<Response> SetFunctionBreakpointsApiHandler::GenerateSuccessResponse()
{
    std::unique_ptr<SetFunctionBreakpointsResponse> responsePointer =
            std::make_unique<SetFunctionBreakpointsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = std::move(breakpointResult);
    return responsePointer;
}

Breakpoint SetFunctionBreakpointsApiHandler::GenerateBreakpointBase(int64_t id, string funcName,
    bool verified, const string &msg)
{
    Breakpoint currentBreak;
    currentBreak.id = id;
    currentBreak.source = Source();
    currentBreak.source->name = funcName;
    currentBreak.verified = verified;
    if (!msg.empty()) {
        currentBreak.message = msg;
    }
    return currentBreak;
}

void SetFunctionBreakpointsApiHandler::RemoveBreakpoints() const
{
    SBTarget sbTarget = debugger->GetSbTarget();
    FunctionBreakPointMap &map = debugger->GetFunctionBreakpointMap();
    std::vector<std::set<int>> breakNumSets = map.GetAllSet();
    for (size_t i = 0; i < breakNumSets.size(); i++) {
        for (auto it = breakNumSets[i].begin(); it!=breakNumSets[i].end(); it++) {
            sbTarget.BreakpointDelete(*it);
        }
    }
    map.RemoveAllBreakPoint();
}
