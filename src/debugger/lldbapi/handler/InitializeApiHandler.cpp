// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "InitializeApiHandler.h"
#include "DapLog.h"
#include "LLDBApiEventHandler.h"
#include "TimeTravelDebugManager.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

UniqueResponsePtr InitializeApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<InitializeRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        std::string error = "ERROR: InitializeRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<InitializeResponse>(), error, currentRequest->seq);
    }
    ParseParam(*reqPtr);
    debugger->SetInitialized(true);

    // initialize and start event handler
    if (debugger->GetEventHandler() == nullptr) {
        auto eventHandler = std::make_shared<LLDBApiEventHandler>(debugger);
        debugger->SetEventHandler(eventHandler);
    }
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CPP) {
        // for DevEco, set default value of MaxStringSummaryLength to 256
        debugger->SetMaxStringSummaryLength("256");
    }
    if (!debugger->GetEventHandler()->StartListener()) {
        return debugger->CompleteFailureResponse(std::make_unique<InitializeResponse>(),
                                                 debugger->GetEventHandler()->GetErrorMsg(), currentRequest->seq);
    }
    return GenerateSuccessResponse();
}

void InitializeApiHandler::ParseParam(const InitializeRequest &req) const
{
    LLDBApiDebuggerOption currentOption = debugger->GetOption();
    if (req.arguments.linesStartAt1.has_value()) {
        currentOption.linesStartAt1 = req.arguments.linesStartAt1.value();
    }
    if (req.arguments.columnsStartAt1.has_value()) {
        currentOption.columnsStartAt1 = req.arguments.columnsStartAt1.value();
    }
    if (req.arguments.supportsRunInTerminalRequest.has_value()) {
        currentOption.supportsRunInTerminalRequest = req.arguments.supportsRunInTerminalRequest.value();
    }
    SetDebuggeeLanguageType(req.arguments.adapterID);
    currentOption.adapterID = req.arguments.adapterID;
    currentOption.disassemblyOption = ShowDisassembly::AUTO;
    debugger->SetOption(currentOption);
}

std::unique_ptr<Response> InitializeApiHandler::GenerateSuccessResponse() const
{
    std::unique_ptr<InitializeResponse> responsePointer = std::make_unique<InitializeResponse>();
    responsePointer->request_seq = currentRequest->seq;
    responsePointer->seq = debugger->GetAndIncCurrentMessageSeq();
    responsePointer->success = true;
    responsePointer->body = GenerateCapabilities();
    return responsePointer;
}

const Capabilities InitializeApiHandler::GenerateCapabilities() const
{
    Capabilities capabilities;
    capabilities.supportTerminateDebuggee = true;
    // supports stepping granularities (argument 'granularity' of the stepping requests)
    capabilities.supportsSteppingGranularity = true;
    capabilities.supportsConfigurationDoneRequest = true;
    capabilities.supportsConditionalBreakpoints = true;
    capabilities.supportsFunctionBreakpoints = true;
    capabilities.supportsSetVariable = true;
    capabilities.supportsDataBreakpoints = true;
    capabilities.supportsHitConditionalBreakpoints = true;
    capabilities.supportsDisassembleRequest = true;
    capabilities.supportsInstructionBreakpoints = true;
    capabilities.supportsReadMemoryRequest = true;
    capabilities.supportsWriteMemoryRequest = true;
    capabilities.supportsTimeTravelDebugging = true;
    capabilities.exceptionBreakpointFilters = GenerateExceptionBreakpointsFilters();
    capabilities.supportsSetExecutionPoint = true;
    capabilities.supportedTimeTravelRecordPointTypes = debugger->GetTimeTravelManager()->GetSupportedRecordPointTypes();
    capabilities.supportsLogPoints = true;
    return capabilities;
}

const vector<ExceptionBreakpointsFilter> InitializeApiHandler::GenerateExceptionBreakpointsFilters() const
{
    vector<ExceptionBreakpointsFilter> exceptionBreakpointsFilters;
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE) {
        return exceptionBreakpointsFilters;
    }
    std::vector<std::string> excepts = LLDB_API_EXCEPTION_FILTERS;
    for (const auto &except : excepts) {
        ExceptionBreakpointsFilter filter;
        filter.filter = except;
        filter.label = "catch " + except;
        filter.default_ = false;
        exceptionBreakpointsFilters.emplace_back(filter);
    }
    return exceptionBreakpointsFilters;
}

void InitializeApiHandler::SetDebuggeeLanguageType(const string &adapterID) const
{
    if (CommonUtils::RegexSearchIgnoreCase(adapterID.data(), ".*(cangjie).*")) {
        debugger->SetDebuggeeLanguageType(DebuggeeLanguageType::CANGJIE);
    }
}