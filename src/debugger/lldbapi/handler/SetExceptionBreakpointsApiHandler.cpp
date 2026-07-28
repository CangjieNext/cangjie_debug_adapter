// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetExceptionBreakpointsApiHandler.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetExceptionBreakpointsApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetExceptionBreakpointsRequest*>(currentRequest.get());
    std::string error;
    if (reqPtr == nullptr) {
        error = "ERROR: SetExceptionBreakpointsRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetExceptionBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    if (!IsFiltersCorrect(reqPtr->arguments.filters)) {
        error = "ERROR: The filters have fields except throw and catch.";
        return debugger->CompleteFailureResponse(std::make_unique<SetExceptionBreakpointsResponse>(),
                                                 error, currentRequest->seq);
    }
    RemoveBreakpoints();
    AddBreakpoints(reqPtr->arguments.filters);
    return GenerateSuccessResponse();
}

void SetExceptionBreakpointsApiHandler::AddBreakpoints(const std::vector<std::string> &filters)
{
    SBTarget sbTarget = debugger->GetSbTarget();
    ExceptionBreakpointSet &exceptionBreakpointSet = debugger->GetExceptionBreakpoints();
    for (const auto &filter : filters) {
        bool catchBp = false;
        bool throwBp = false;
        if (filter == LLDB_API_EXCEPTION_CATCH) {
            catchBp = true;
        } else if (filter == LLDB_API_EXCEPTION_THROW) {
            throwBp = true;
        }
        SBBreakpoint sbBreakpoint = sbTarget.BreakpointCreateForException(LANGUAGE_TYPE_C_PLUS_PLUS, catchBp, throwBp);
        Breakpoint currentBreak;
        currentBreak.id = sbBreakpoint.GetID();
        currentBreak.verified = sbBreakpoint.IsValid() && sbBreakpoint.GetNumLocations() > 0;
        if (currentBreak.id.has_value()) {
            exceptionBreakpointSet.AddExceptionBreakPoint(currentBreak.id.value());
        }
        breakpointResult.emplace_back(currentBreak);
    }
}

std::unique_ptr<Response> SetExceptionBreakpointsApiHandler::GenerateSuccessResponse()
{
    std::unique_ptr<SetExceptionBreakpointsResponse> responsePointer =
            std::make_unique<SetExceptionBreakpointsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = std::move(breakpointResult);
    return responsePointer;
}

void SetExceptionBreakpointsApiHandler::RemoveBreakpoints() const
{
    SBTarget sbTarget = debugger->GetSbTarget();
    ExceptionBreakpointSet &exceptionBreakpointSet = debugger->GetExceptionBreakpoints();
    std::set<int> set = exceptionBreakpointSet.GetAllSet();
    if (!set.empty()) {
        for (const int &it : set) {
            sbTarget.BreakpointDelete(it);
        }
        exceptionBreakpointSet.RemoveAllExceptionBreakPoint();
    }
}

bool SetExceptionBreakpointsApiHandler::IsFiltersCorrect(const std::vector<std::string> &filters) const
{
    const std::vector<std::string> filterStrings = LLDB_API_EXCEPTION_FILTERS;
    for (const auto &filter : filters) {
        if (find(filterStrings.begin(), filterStrings.end(), filter) == filterStrings.end()) {
            return false;
        }
    }
    return true;
}