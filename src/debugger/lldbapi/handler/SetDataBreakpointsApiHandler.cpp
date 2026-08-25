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
        if (IsConditionUnsupported(condition)) {
            continue;
        }
        DataBreakpointAccessType accessType = dataBreakpoint.accessType.value_or(DataBreakpointAccessType::WRITE);
        bool read = accessType != DataBreakpointAccessType::WRITE;
        bool write = accessType != DataBreakpointAccessType::READ;
        lldb::addr_t address = 0;
        size_t size = 0;
        if (!ParseDataId(variableAddressAndSize, address, size)) {
            continue;
        }
        lldb::watch_id_t watchpointId = CreateWatchpoint(sbTarget, address, size, read, write,
            dataBreakpoint, currentBreak);
        if (!LLDB_WATCH_ID_IS_VALID(watchpointId)) {
            continue;
        }
        currentBreak.id = watchpointId;
        currentBreak.verified = true;
        dataBreakpointSet.AddDataBreakPoint(watchpointId);
        breakpointResult.emplace_back(currentBreak);
    }
}

lldb::watch_id_t SetDataBreakpointsApiHandler::CreateWatchpoint(const lldb::SBTarget &sbTarget,
    lldb::addr_t address, size_t size, bool read, bool write,
    const DataBreakpoint &dataBreakpoint, Breakpoint &currentBreak)
{
    SBError sbError = SBError();
    SBWatchpoint sbBreakpoint = sbTarget.WatchAddress(address, size, read, write, sbError);
    if (sbError.Fail() || !sbBreakpoint.IsValid()) {
        const char *errorMessage = sbError.GetCString();
        GenerateFailureBreakpointResult(currentBreak,
            errorMessage == nullptr ? "Failed to create data breakpoint." : errorMessage);
        return LLDB_INVALID_WATCH_ID;
    }
    lldb::watch_id_t watchpointId = sbBreakpoint.GetID();
    if (!LLDB_WATCH_ID_IS_VALID(watchpointId)) {
        GenerateFailureBreakpointResult(currentBreak, "Failed to create data breakpoint.");
        return LLDB_INVALID_WATCH_ID;
    }
    std::string condition = dataBreakpoint.condition.value_or("");
    if (!condition.empty()) {
        sbBreakpoint.SetCondition(condition.data());
    }
    int hitCount = 0;
    if (dataBreakpoint.hitCondition.has_value()) {
        CommonUtils::StringToInt(dataBreakpoint.hitCondition.value(), hitCount);
    }
    if (hitCount > 0) {
        sbBreakpoint.SetIgnoreCount(hitCount - 1);
    }
    return watchpointId;
}

bool SetDataBreakpointsApiHandler::IsConditionUnsupported(const std::string &condition)
{
    if (condition.empty()) {
        return false;
    }
#ifdef _DARWIN
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE) {
        Breakpoint currentBreak;
        currentBreak.verified = false;
        currentBreak.id = invalidBreakpointId;
        currentBreak.message = "Conditional breakpoints are not supported.";
        breakpointResult.emplace_back(currentBreak);
        return true;
    }
#else
    if (debugger->GetOption().OhosCangjieDebug()) {
        Breakpoint currentBreak;
        currentBreak.verified = false;
        currentBreak.id = invalidBreakpointId;
        currentBreak.message = "Conditional breakpoints are not supported.";
        breakpointResult.emplace_back(currentBreak);
        return true;
    }
#endif
    return false;
}

bool SetDataBreakpointsApiHandler::ParseDataId(const std::string &dataId, lldb::addr_t &address, size_t &size)
{
    size_t separator = dataId.find('/');
    if (separator == std::string::npos || separator == 0 || separator + 1 == dataId.size() ||
        dataId.find('/', separator + 1) != std::string::npos) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId format, expected 'address/size'.");
        return false;
    }
    vector<string> addrAndSize = CommonUtils::SplitString(dataId, "/");
    constexpr size_t addressAndSizeElementCount = 2;
    if (addrAndSize.size() != addressAndSizeElementCount) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId format, expected 'address/size'.");
        return false;
    }
    return ParseAddressAndSize(addrAndSize, address, size);
}

bool SetDataBreakpointsApiHandler::ParseAddressAndSize(const std::vector<std::string> &addrAndSize,
    lldb::addr_t &address, size_t &size)
{
    auto isUnsignedDecimal = [](const std::string &value) {
        return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char character) {
            return std::isdigit(character) != 0;
        });
    };
    if (!isUnsignedDecimal(addrAndSize[0]) || !isUnsignedDecimal(addrAndSize[1])) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address and size must be numbers.");
        return false;
    }
    unsigned long long addressValue;
    unsigned long long sizeValue;
    size_t addressParsedLength = 0;
    size_t sizeParsedLength = 0;
    try {
        addressValue = stoull(addrAndSize[0], &addressParsedLength);
        sizeValue = stoull(addrAndSize[1], &sizeParsedLength);
    } catch (const std::invalid_argument&) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address and size must be numbers.");
        return false;
    } catch (const std::out_of_range&) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address or size is out of range.");
        return false;
    }
    if (addressParsedLength != addrAndSize[0].size() || sizeParsedLength != addrAndSize[1].size() ||
        addressValue > std::numeric_limits<addr_t>::max() || sizeValue > std::numeric_limits<size_t>::max()) {
        Breakpoint currentBreak;
        GenerateFailureBreakpointResult(currentBreak, "Invalid dataId: address or size is out of range.");
        return false;
    }
    address = static_cast<addr_t>(addressValue);
    size = static_cast<size_t>(sizeValue);
    return true;
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
