// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "BreakpointLocationsApiHandler.h"
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

    filterStepinLocation = req->arguments->filterStepinLocation.has_value() &&
        req->arguments->filterStepinLocation.value();
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
    SBAddress sbAddress;
    SBTarget target = debugger->GetSbTarget();
    if (req->arguments->startAddress.has_value()) {
        lldb::addr_t address;
        if (CommonUtils::HexStringToUnsignedLongLong(req->arguments->startAddress.value(), address)) {
            sbAddress = SBAddress(address, target);
        }
    } else {
        sbAddress = this->GetSBAddressByLocation(req->arguments->source.path.value(), req->arguments->line);
    }
    if (!sbAddress.IsValid()) {
        return {};
    }
    std::vector<Dap::Protocol::BreakpointLocation> locations;
    BreakpointLocation firstLocation = GenBkptLocationOfSBAddress(sbAddress);
    // check whether startAddress is the first address of the location
    SBAddress firstAddress = GetSBAddressByLocation(req->arguments->source.path.value(), firstLocation.line,
        firstLocation.column.value());
    if (LocationIsValid(firstLocation, this->GetIncOfSBAddress(sbAddress)) &&
        firstAddress.GetLoadAddress(debugger->GetSbTarget()) == sbAddress.GetLoadAddress(debugger->GetSbTarget())) {
        locations.push_back(firstLocation);
    }

    const int readIncNum = 2;
    int64_t endLine = req->arguments->endLine.has_value() ? req->arguments->endLine.value() : req->arguments->line;
    int64_t endColumn = req->arguments->endColumn.has_value() ? req->arguments->endColumn.value(): -1;

    while (true) {
        // Reads assembly code from the starting position until out of range or failure
        SBInstructionList sbInstructionList = target.ReadInstructions(sbAddress, readIncNum);
        sbAddress = sbInstructionList.GetInstructionAtIndex(1).GetAddress();
        if (!sbAddress.IsValid()) {
            LOG_INFO("read instruction error.");
            break;
        }
        BreakpointLocation location = GenBkptLocationOfSBAddress(sbAddress);
        if (location.line > endLine) {
            break;
        } else if (endColumn != -1 && location.line == endLine && location.column > endColumn) {
            break;
        } else if (firstLocation.line == location.line && firstLocation.column == location.column) {
            continue;
        } else if (!LocationIsValid(location, this->GetIncOfSBAddress(sbAddress))) {
            continue;
        } else if (locations.end() != std::find_if(locations.begin(), locations.end(),
            [&](const BreakpointLocation& x) { return x.line == location.line && x.column == location.column; })) {
            // Do not store the redundant location, different assembly code may be mapped to the same row and column.
            continue;
        }
        locations.emplace_back(location);
    }
    return locations;
}

SBAddress BreakpointLocationsApiHandler::GetSBAddressByLocation(const string &filePath, int line, int column)
{
    // Obtain the corresponding address based on the source code path and line number:
    // Set a breakpoint, obtain the breakpoint address, then delete the breakpoint.
    SBAddress sbAddress;
    SBBreakpoint sbBreakpoint;
    SBTarget sbTarget = debugger->GetSbTarget();
    line = line + (debugger->GetOption().linesStartAt1 ? 0 : 1);
    column = column + (debugger->GetOption().columnsStartAt1 ? 0 : 1);
    if (column > 0) {
        SBFileSpecList sbFileSpecList;
        SBFileSpec sbFileSpec(filePath.c_str());
        sbBreakpoint = sbTarget.BreakpointCreateByLocation(sbFileSpec, line, column, 0, sbFileSpecList);
    } else {
        sbBreakpoint =sbTarget.BreakpointCreateByLocation(filePath.c_str(), line);
    }

    if (!sbBreakpoint.IsValid()) {
        return sbAddress;
    }
    SBBreakpointLocation location = sbBreakpoint.GetLocationAtIndex(0);
    sbTarget.BreakpointDelete(sbBreakpoint.GetID());
    if (!location.IsValid()) {
        return sbAddress;
    }
    sbAddress = location.GetAddress();
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
    if (location.line < startLine) {
        return false;
    }
    if (location.line > endLine) {
        return false;
    }
    if (location.line == startLine && location.column < startColumn) {
        return false;
    }
    if (endColumn != -1 && location.line == endLine && location.column > endColumn) {
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
