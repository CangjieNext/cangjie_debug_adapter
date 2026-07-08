// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetDataBreakpointsApiHandler.h"
#include <thread>
#include "CommonUtils.h"
#include "DapLog.h"
#include "DataId.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetDataBreakpointsApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetDataBreakpointsRequest*>(currentRequest.get());
    std::string error;
    if (reqPtr == nullptr) {
        error = "ERROR: Invalid parameter, SetDataBreakpointsRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    RemoveBreakpoints();
    AddBreakpoints(reqPtr->arguments.breakpoints);
    return GenerateSuccessResponse();
}

void SetDataBreakpointsApiHandler::AddBreakpoints(const std::vector<DataBreakpoint> &dataBreakpoints)
{
    SBTarget sbTarget = debugger->GetSbTarget();
    DataBreakpointSet& dataBreakpointSet = debugger->GetDataBreakpoints();
    for (const auto &dataBreakpoint : dataBreakpoints) {
        std::string variableAddressAndSize = dataBreakpoint.dataId;
        Breakpoint currentBreak;
        if (variableAddressAndSize.empty()) {
            GenerateFailureBreakpointResult(currentBreak, "DataId is empty.");
            continue;
        }
        try {
            std::string condition = dataBreakpoint.condition.has_value() ? dataBreakpoint.condition.value() : "";
#ifdef _DARWIN
            if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && !condition.empty()) {
                Breakpoint currentBreak;
                currentBreak.verified = false;
                currentBreak.id = invalidBreakpointId;
                currentBreak.message = "Conditional breakpoints are not supported.";
                breakpointResult.emplace_back(currentBreak);
                continue;
            }
#else
            if (debugger->GetOption().OhosCangjieDebug() && !condition.empty()) {
                Breakpoint currentBreak;
                currentBreak.verified = false;
                currentBreak.id = invalidBreakpointId;
                currentBreak.message = "Conditional breakpoints are not supported.";
                breakpointResult.emplace_back(currentBreak);
                continue;
            }
#endif
            bool read = dataBreakpoint.accessType.value() != DataBreakpointAccessType::WRITE;
            bool write = dataBreakpoint.accessType.value() != DataBreakpointAccessType::READ;
            vector<string> addrAndSize = CommonUtils::SplitString(variableAddressAndSize, "/");
            if (addrAndSize.size() < 2) {
                GenerateFailureBreakpointResult(currentBreak, "Invalid dataId format, expected 'address/size'.");
                continue;
            }
            addr_t address = stoull(addrAndSize[0]);
            size_t size = stoull(addrAndSize[1]);
            SBError sbError = SBError();
            SBWatchpoint sbBreakpoint = sbTarget.WatchAddress(address, size, read, write, sbError);
            int hitCount = 0;
            if (!condition.empty()) {
                sbBreakpoint.SetCondition(condition.data());
            }
            if (dataBreakpoint.hitCondition.has_value()) {
                CommonUtils::StringToInt(dataBreakpoint.hitCondition.value(), hitCount);
            }
            if (hitCount > 0) {
                sbBreakpoint.SetIgnoreCount(hitCount - 1);
            }
            if (sbError.Fail()) {
                GenerateFailureBreakpointResult(currentBreak, sbError.GetCString());
                continue;
            }
            currentBreak.id = sbBreakpoint.GetID();
            currentBreak.verified = sbBreakpoint.IsValid();
        } catch (exception &e) {
            currentBreak.verified = false;
            LOG_INFO("An error has occurred, such as dataID resolution failure.");
        }
        if (currentBreak.id.has_value()) {
            dataBreakpointSet.AddDataBreakPoint(currentBreak.id.value());
        }
        breakpointResult.emplace_back(currentBreak);
    }
}

std::unique_ptr<Response> SetDataBreakpointsApiHandler::GenerateSuccessResponse()
{
    std::unique_ptr<SetDataBreakpointsResponse> responsePointer = std::make_unique<SetDataBreakpointsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = breakpointResult;
    return responsePointer;
}

void SetDataBreakpointsApiHandler::GenerateFailureBreakpointResult(Breakpoint& currentBreak, string message)
{
    currentBreak.verified = false;
    currentBreak.message = message;
    breakpointResult.emplace_back(currentBreak);
    LOG_ERROR(message);
}

void SetDataBreakpointsApiHandler::RemoveBreakpoints() const
{
    SBTarget sbTarget = debugger->GetSbTarget();
    DataBreakpointSet& set = debugger->GetDataBreakpoints();
    sbTarget.DisableAllWatchpoints();
    // for avoid watchpoint0 (delete watchpoint when disabled after watchpoint triggered),
    // disable watchpoint before delete (it takes at least 40 ms to complete)
    constexpr int waitMS = 100;
    std::this_thread::sleep_for(std::chrono::milliseconds(waitMS));
    sbTarget.DeleteAllWatchpoints();
    set.RemoveAllDataBreakPoint();
}