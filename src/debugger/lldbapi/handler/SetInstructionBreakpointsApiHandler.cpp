// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetInstructionBreakpointsApiHandler.h"
#include "DapLog.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetInstructionBreakpointsApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetInstructionBreakpointsRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        std::string error = "ERROR: SetInstructionBreakpoints Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetInstructionBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    this->RemoveBreakpoints();
    if (!reqPtr->arguments.breakpoints.empty()) {
        this->AddBreakpoints(reqPtr->arguments.breakpoints);
    }
    return GenerateSuccessResponse();
}

void SetInstructionBreakpointsApiHandler::AddBreakpoints(const std::vector<InstructionBreakpoint> &breakpoints)
{
    SBTarget sbTarget = debugger->GetSbTarget();
    InstructionBreakPointMap &instructionBreakPointMap = debugger->GetInstructionBreakpointMap();
    for (const auto &breakpoint : breakpoints) {
        lldb::addr_t address;
        std::string instructionReference = breakpoint.instructionReference;
        if (!CommonUtils::HexStringToUnsignedLongLong(instructionReference, address)) {
            string msg = "Invalid instruction reference.";
            breakpointResult.emplace_back(GenerateBreakpointBase(-1, instructionReference, false, msg));
            continue;
        }
        std::string condition = breakpoint.condition.has_value() ? breakpoint.condition.value() : "";
#ifdef _DARWIN
        if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(
                GenerateBreakpointBase(invalidBreakpointId, instructionReference, false, msg));
            continue;
        }
#else
        if (debugger->GetOption().OhosCangjieDebug() && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(
                GenerateBreakpointBase(invalidBreakpointId, instructionReference, false, msg));
            continue;
        }
#endif
        if (breakpoint.offset.has_value()) {
            address += breakpoint.offset.value();
        }
        SBBreakpoint sbBreakpoint = sbTarget.BreakpointCreateByAddress(address);
        int hitCount = 0;
        if (breakpoint.hitCondition.has_value()) {
            CommonUtils::StringToInt(breakpoint.hitCondition.value(), hitCount);
        }
        if (hitCount > 0) {
            sbBreakpoint.SetIgnoreCount(hitCount - 1);
        }
        if (!condition.empty()) {
            sbBreakpoint.SetCondition(condition.data());
        }
        Breakpoint currentBreak = GenerateBreakpointBase(sbBreakpoint.GetID(), instructionReference,
            sbBreakpoint.IsValid() && sbBreakpoint.GetNumLocations() > 0);
        if (currentBreak.id.has_value()) {
            instructionBreakPointMap.AddBreakPoint(currentBreak.id.value(), instructionReference);
        }
        breakpointResult.emplace_back(currentBreak);
    }
}

std::unique_ptr<Response> SetInstructionBreakpointsApiHandler::GenerateSuccessResponse()
{
    std::unique_ptr<SetInstructionBreakpointsResponse> responsePointer =
        std::make_unique<SetInstructionBreakpointsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = std::move(breakpointResult);
    return responsePointer;
}

Breakpoint SetInstructionBreakpointsApiHandler::GenerateBreakpointBase(int64_t id, std::string instructionReference,
    bool verified, const std::string &msg)
{
    Breakpoint currentBreak;
    currentBreak.id = id;
    currentBreak.instructionReference = instructionReference;
    currentBreak.verified = verified;
    if (!msg.empty()) {
        currentBreak.message = msg;
    }
    return currentBreak;
}

void SetInstructionBreakpointsApiHandler::RemoveBreakpoints() const
{
    SBTarget sbTarget = debugger->GetSbTarget();
    InstructionBreakPointMap &instructionBreakPointMap = debugger->GetInstructionBreakpointMap();
    std::vector<int> breakpointIds = instructionBreakPointMap.GetAllBreakpointId();
    for (auto it = breakpointIds.begin(); it!=breakpointIds.end(); it++) {
        sbTarget.BreakpointDelete(*it);
    }
    instructionBreakPointMap.RemoveAllBreakPoint();
}