// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetBreakpointsApiHandler.h"
#include "DapLog.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetBreakpointsApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetBreakpointsRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        std::string error = "ERROR: SetBreakpointsRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    if (reqPtr->arguments.sourceModified.has_value() && reqPtr->arguments.sourceModified.value()) {
        LOG_INFO("The source file is modified!");
    }
    if (reqPtr->arguments.source.path.has_value()) {
        filePath = reqPtr->arguments.source.path.value();
        filePath = CommonUtils::ReplaceString(filePath, "\\", "/");
    }
    this->RemoveBreakpoints();
    if (reqPtr->arguments.breakpoints.has_value()) {
        this->AddBreakpoints(reqPtr->arguments.breakpoints.value());
    }
    return GenerateSuccessResponse();
}

void SetBreakpointsApiHandler::AddBreakpoints(const std::vector<SourceBreakpoint> &breakpoints)
{
    SBTarget sbTarget = debugger->GetSbTarget();
    for (const auto &breakpoint : breakpoints) {
        int line = debugger->GetOption().linesStartAt1 ? breakpoint.line : breakpoint.line + 1;
        int column = breakpoint.column.has_value() ? breakpoint.column.value() : -1;
        string condition = breakpoint.condition.has_value() ? breakpoint.condition.value() : "";
#ifdef _DARWIN
        if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(
                GenerateBreakpointBase(invalidBreakpointId, breakpoint.line, column, false, msg));
            continue;
        }
#else
        if (debugger->GetOption().OhosCangjieDebug() && !condition.empty()) {
            string msg = "Conditional breakpoints are not supported.";
            breakpointResult.emplace_back(
                GenerateBreakpointBase(invalidBreakpointId, breakpoint.line, column, false, msg));
            continue;
        }
#endif
        SBBreakpoint sbBreakpoint;
        if (column < 0) {
            sbBreakpoint = sbTarget.BreakpointCreateByLocation(filePath.c_str(), line);
        } else {
            SBFileSpec sbFileSpec(filePath.c_str());
            SBFileSpecList sbFileSpecList;
            sbBreakpoint = sbTarget.BreakpointCreateByLocation(sbFileSpec, line, column, 0, sbFileSpecList);
        }
        int hitCount = 0;
        if (breakpoint.hitCondition.has_value()) {
            CommonUtils::StringToInt(breakpoint.hitCondition.value(), hitCount);
        }
        if (hitCount > 0) {
            sbBreakpoint.SetIgnoreCount(hitCount - 1);
        }
        if (breakpoint.hitOnce.has_value() && breakpoint.hitOnce.value()) {
            sbBreakpoint.SetOneShot(true);
        }
        if (!condition.empty()) {
            sbBreakpoint.SetCondition(breakpoint.condition->data());
        }
        Breakpoint currentBreak = GenerateBreakpointBase(sbBreakpoint.GetID(), breakpoint.line,
            sbBreakpoint.GetLocationAtIndex(0).GetAddress().GetLineEntry().GetColumn(), sbBreakpoint.IsValid());
        if (currentBreak.id.has_value()) {
            if (breakpoint.logMessage.has_value()) {
                debugger->AddBreakPointLogMsg(breakpoint.logMessage.value(), currentBreak.id.value());
            }
        }
        breakpointResult.emplace_back(currentBreak);
    }
}

std::unique_ptr<Response> SetBreakpointsApiHandler::GenerateSuccessResponse()
{
    std::unique_ptr<SetBreakpointsResponse> responsePointer = std::make_unique<SetBreakpointsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = std::move(breakpointResult);
    return responsePointer;
}

Breakpoint SetBreakpointsApiHandler::GenerateBreakpointBase(int64_t id, int64_t line, int64_t column,
    bool verified, const string &msg)
{
    Breakpoint currentBreak;
    currentBreak.id = id;
    currentBreak.line = line;
    currentBreak.source = Source();
    currentBreak.source->path = filePath;
    currentBreak.verified = verified;
    currentBreak.column = column;
    if (!msg.empty()) {
        currentBreak.message = msg;
    }
    if (id != -1) {
        string convertedFilePath = CommonUtils::ConvertDriveLetterToUppercase(filePath);
        debugger->GetBreakpointMap().AddBreakPoint(convertedFilePath, currentBreak.id.value());
    }
    return currentBreak;
}

void SetBreakpointsApiHandler::RemoveBreakpoints() const
{
    string convertedFilePath = CommonUtils::ConvertDriveLetterToUppercase(filePath);
    BreakPointMap &breakPointMap = debugger->GetBreakpointMap();
    if (!breakPointMap.HasPath(convertedFilePath)) {
        LOG_WARN("WARN: No file path exist, filePath: ", convertedFilePath);
        return;
    }
    SBTarget sbTarget = debugger->GetSbTarget();
    std::set<int> breakPointsSet = breakPointMap.GetSet(convertedFilePath);
    for (auto it = breakPointsSet.begin(); it!=breakPointsSet.end(); it++) {
        sbTarget.BreakpointDelete(*it);
        if (!debugger->RemoveBreakPointLogMsg(*it)) {
            LOG_WARN("WARN: No log point exist, filePath: ", convertedFilePath, " breakpoint id: ", *it);
        }
    }
    breakPointMap.RemoveBreakPoint(convertedFilePath);
}