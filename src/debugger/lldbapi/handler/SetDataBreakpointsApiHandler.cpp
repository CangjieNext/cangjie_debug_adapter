// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetDataBreakpointsApiHandler.h"
#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>
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
        return debugger->CompleteFailureResponse(std::make_unique<SetDataBreakpointsResponse>(),
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
        std::string condition = dataBreakpoint.condition.value_or("");
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
        DataBreakpointAccessType accessType = dataBreakpoint.accessType.value_or(DataBreakpointAccessType::WRITE);
        bool read = accessType != DataBreakpointAccessType::WRITE;
        bool write = accessType != DataBreakpointAccessType::READ;
        size_t separator = variableAddressAndSize.find('/');
        if (separator == std::string::npos || separator == 0 || separator + 1 == variableAddressAndSize.size() ||
            variableAddressAndSize.find('/', separator + 1) != std::string::npos) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId format, expected 'address/size'.");
            continue;
        }
        vector<string> addrAndSize = CommonUtils::SplitString(variableAddressAndSize, "/");
        constexpr size_t addressAndSizeElementCount = 2;
        if (addrAndSize.size() != addressAndSizeElementCount) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId format, expected 'address/size'.");
            continue;
        }
        auto isUnsignedDecimal = [](const std::string &value) {
            return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char character) {
                return std::isdigit(character) != 0;
            });
        };
        if (!isUnsignedDecimal(addrAndSize[0]) || !isUnsignedDecimal(addrAndSize[1])) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address and size must be numbers.");
            continue;
        }
        unsigned long long addressValue;
        unsigned long long sizeValue;
        size_t addressParsedLength = 0;
        size_t sizeParsedLength = 0;
        try {
            addressValue = stoull(addrAndSize[0], &addressParsedLength);
            sizeValue = stoull(addrAndSize[1], &sizeParsedLength);
        } catch (const std::invalid_argument&) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address and size must be numbers.");
            continue;
        } catch (const std::out_of_range&) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address or size is out of range.");
            continue;
        }
        if (addressParsedLength != addrAndSize[0].size() || sizeParsedLength != addrAndSize[1].size() ||
            addressValue > std::numeric_limits<addr_t>::max() || sizeValue > std::numeric_limits<size_t>::max()) {
            GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address or size is out of range.");
            continue;
        }
        addr_t address = static_cast<addr_t>(addressValue);
        size_t size = static_cast<size_t>(sizeValue);
        SBError sbError = SBError();
        SBWatchpoint sbBreakpoint = sbTarget.WatchAddress(address, size, read, write, sbError);
        if (sbError.Fail() || !sbBreakpoint.IsValid()) {
            const char *errorMessage = sbError.GetCString();
            GenerateFailureBreakpointResult(currentBreak,
                errorMessage == nullptr ? "Failed to create data breakpoint." : errorMessage);
            continue;
        }
        lldb::watch_id_t watchpointId = sbBreakpoint.GetID();
        if (!LLDB_WATCH_ID_IS_VALID(watchpointId)) {
            GenerateFailureBreakpointResult(currentBreak, "Failed to create data breakpoint.");
            continue;
        }
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
        currentBreak.id = watchpointId;
        currentBreak.verified = true;
        dataBreakpointSet.AddDataBreakPoint(watchpointId);
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
