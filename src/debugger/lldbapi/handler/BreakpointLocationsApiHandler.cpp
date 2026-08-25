// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "BreakpointLocationsApiHandler.h"
#include <limits>
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

Dap::UniqueResponsePtr Dap::BreakpointLocationsApiHandler::HandleRequest(Dap::UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto req = dynamic_cast<BreakpointLocationsRequest*>(currentRequest.get());
    if (req == nullptr) {
        std::string error = "ERROR: Invalid parameter, BreakpointLocationsRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<BreakpointLocationsResponse>(),
            error,  Protocol::SEQ_INVALID);
    }
    if (!req->arguments.has_value()) {
        std::string error = "ERROR: BreakpointLocationsRequest arguments are missing.";
        return debugger->CompleteFailureResponse(std::make_unique<BreakpointLocationsResponse>(),
            error, currentRequest->seq);
    }

    filterStepinLocation = req->arguments->filterStepinLocation.value_or(false);
    proLine = 0;

    std::vector<Dap::Protocol::BreakpointLocation> locations =  GenBreakpointLocations();

    std::unique_ptr<BreakpointLocationsResponse> responsePointer = std::make_unique<BreakpointLocationsResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    responsePointer->breakpoints = std::move(locations);
    return responsePointer;
}

vector<BreakpointLocation> BreakpointLocationsApiHandler::GenBreakpointLocations()
{
    auto req = dynamic_cast<BreakpointLocationsRequest*>(currentRequest.get());
    SBAddress sbAddress = ResolveStartSBAddress();
    if (!sbAddress.IsValid()) {
        return {};
    }
    lldb::addr_t currentAddress = sbAddress.GetLoadAddress(debugger->GetSbTarget());
    if (currentAddress == LLDB_INVALID_ADDRESS) {
        return {};
    }
    std::vector<Dap::Protocol::BreakpointLocation> locations;
    firstLocation = GenBkptLocationOfSBAddress(sbAddress);
    // check whether startAddress is the first address of the location
    if (firstLocation.column.has_value() && req->arguments->source.path.has_value() &&
        LocationIsValid(firstLocation, this->GetIncOfSBAddress(sbAddress))) {
        SBAddress firstAddress = GetSBAddressByLocation(req->arguments->source.path.value(), firstLocation.line,
            firstLocation.column.value());
        if (firstAddress.GetLoadAddress(debugger->GetSbTarget()) == sbAddress.GetLoadAddress(debugger->GetSbTarget())) {
            locations.push_back(firstLocation);
        }
    }

    constexpr uint32_t readInstructionCount = 2;
    endLine = req->arguments->endLine.has_value() ? req->arguments->endLine.value() : req->arguments->line;
    endColumn = req->arguments->endColumn.has_value() ? req->arguments->endColumn.value(): -1;

    while (sbAddress.IsValid()) {
        // Reads assembly code from the starting position until out of range or failure
        SBInstructionList sbInstructionList = debugger->GetSbTarget().ReadInstructions(sbAddress, readInstructionCount);
        if (sbInstructionList.GetSize() < readInstructionCount) {
            LOG_INFO("read instruction error.");
            break;
        }
        SBAddress nextAddress = sbInstructionList.GetInstructionAtIndex(1).GetAddress();
        lldb::addr_t nextLoadAddress = nextAddress.GetLoadAddress(debugger->GetSbTarget());
        if (!nextAddress.IsValid() || nextLoadAddress == LLDB_INVALID_ADDRESS || nextLoadAddress <= currentAddress) {
            LOG_INFO("instruction address did not advance.");
            break;
        }
        sbAddress = nextAddress;
        currentAddress = nextLoadAddress;
        BreakpointLocation location = GenBkptLocationOfSBAddress(sbAddress);
        if (ShouldSkipLocation(location, sbAddress, locations)) {
            continue;
        }
        locations.emplace_back(location);
    }
    return locations;
}

SBAddress BreakpointLocationsApiHandler::ResolveStartSBAddress()
{
    auto req = dynamic_cast<BreakpointLocationsRequest*>(currentRequest.get());
    SBAddress sbAddress;
    if (req->arguments->startAddress.has_value()) {
        lldb::addr_t address;
        if (CommonUtils::HexStringToUnsignedLongLong(req->arguments->startAddress.value(), address)) {
            sbAddress = SBAddress(address, debugger->GetSbTarget());
        }
    } else if (req->arguments->source.path.has_value()) {
        sbAddress = this->GetSBAddressByLocation(req->arguments->source.path.value(), req->arguments->line);
    }
    return sbAddress;
}

bool BreakpointLocationsApiHandler::ShouldSkipLocation(const BreakpointLocation &location,
    const SBAddress &sbAddress, const std::vector<BreakpointLocation> &locations)
{
    if (location.line > endLine) {
        return true;
    }
    if (endColumn != -1 && location.line == endLine && location.column.value_or(-1) > endColumn) {
        return true;
    }
    if (firstLocation.line == location.line && firstLocation.column == location.column) {
        return true;
    }
    if (!LocationIsValid(location, this->GetIncOfSBAddress(sbAddress))) {
        return true;
    }
    if (locations.end() != std::find_if(locations.begin(), locations.end(),
        [&](const BreakpointLocation& x) { return x.line == location.line && x.column == location.column; })) {
        // Do not store the redundant location, different assembly code may be mapped to the same row and column.
        return true;
    }
    return false;
}

SBAddress BreakpointLocationsApiHandler::GetSBAddressByLocation(const string &filePath, int64_t line, int64_t column)
{
    // Obtain the corresponding address based on the source code path and line number:
    // Set a breakpoint, obtain the breakpoint address, then delete the breakpoint.
    SBAddress sbAddress;
    SBBreakpoint sbBreakpoint;
    SBTarget sbTarget = debugger->GetSbTarget();
    int64_t lineOffset = debugger->GetOption().linesStartAt1 ? 0 : 1;
    int64_t columnOffset = debugger->GetOption().columnsStartAt1 ? 0 : 1;
    if (line < 0 || line > std::numeric_limits<uint32_t>::max() - lineOffset ||
        column > std::numeric_limits<uint32_t>::max() - columnOffset) {
        return sbAddress;
    }
    uint32_t adaptedLine = static_cast<uint32_t>(line + lineOffset);
    if (adaptedLine == 0) {
        return sbAddress;
    }
    int64_t adaptedColumn = column + columnOffset;
    if (adaptedColumn > 0) {
        SBFileSpecList sbFileSpecList;
        SBFileSpec sbFileSpec(filePath.c_str());
        sbBreakpoint = sbTarget.BreakpointCreateByLocation(sbFileSpec, adaptedLine,
            static_cast<uint32_t>(adaptedColumn), 0, sbFileSpecList);
    } else {
        sbBreakpoint = sbTarget.BreakpointCreateByLocation(filePath.c_str(), adaptedLine);
    }

    if (!sbBreakpoint.IsValid()) {
        return sbAddress;
    }
    if (sbBreakpoint.GetNumLocations() == 0) {
        sbTarget.BreakpointDelete(sbBreakpoint.GetID());
        return sbAddress;
    }
    sbAddress = sbBreakpoint.GetLocationAtIndex(0).GetAddress();
    sbTarget.BreakpointDelete(sbBreakpoint.GetID());
    return sbAddress;
}

Dap::Protocol::BreakpointLocation BreakpointLocationsApiHandler::GenBkptLocationOfSBAddress(SBAddress sbAddress)
{
    if (!sbAddress.IsValid()) {
        return {};
    }
    int adaptLineNum = debugger->GetOption().linesStartAt1 ? 0 : 1;
    int adaptColumnNum = debugger->GetOption().columnsStartAt1 ? 0 : 1;

    SBLineEntry sbLineEntry = sbAddress.GetLineEntry();
    if (!sbLineEntry.IsValid()) {
        return {};
    }
    int64_t line = sbLineEntry.GetLine();
    int64_t column = sbLineEntry.GetColumn();
    if (line == 0) {
        line = proLine;
    } else {
        proLine = line;
    }
    BreakpointLocation location;
    location.line = line - adaptLineNum;
    location.column = column - adaptColumnNum;
    location.address = CommonUtils::Dec2HexAddress(sbAddress.GetLoadAddress(debugger->GetSbTarget()));
    return location;
}

bool BreakpointLocationsApiHandler::LocationIsValid(const BreakpointLocation &location, const string &inc)
{
    if (location.line == -1) {
        return false;
    }
    auto req = dynamic_cast<BreakpointLocationsRequest*>(currentRequest.get());
    int64_t startLine = req->arguments->line;
    int64_t endLine = req->arguments->endLine.has_value()? req->arguments->endLine.value() : startLine;
    int64_t startColumn = req->arguments->column.has_value()? req->arguments->column.value() : -1;
    int64_t endColumn = req->arguments->endColumn.has_value()? req->arguments->endColumn.value() : -1;
    int64_t locationColumn = location.column.value_or(-1);
    if (location.line < startLine) {
        return false;
    }
    if (location.line > endLine) {
        return false;
    }
    if (location.line == startLine && locationColumn < startColumn) {
        return false;
    }
    if (endColumn != -1 && location.line == endLine && locationColumn > endColumn) {
        return false;
    }
    if (filterStepinLocation) {
        if (stepInInc.count(inc) > 0) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

std::string BreakpointLocationsApiHandler::GetIncOfSBAddress(const lldb::SBAddress& sbAddress)
{
    SBInstructionList sbInstructionList = debugger->GetSbTarget().ReadInstructions(sbAddress, 1);
    if (sbInstructionList.GetSize() == 0) {
        return "";
    }
    auto incPtr = sbInstructionList.GetInstructionAtIndex(0).GetMnemonic(debugger->GetSbTarget());
    if (incPtr == nullptr) {
        return "";
    }
    string inc = incPtr;
    return inc;
}
