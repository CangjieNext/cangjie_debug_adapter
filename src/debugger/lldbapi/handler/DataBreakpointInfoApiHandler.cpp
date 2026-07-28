// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DataBreakpointInfoApiHandler.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr DataBreakpointInfoApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto dataBreakpointInfoReq = dynamic_cast<DataBreakpointInfoRequest*>(currentRequest.get());
    if (dataBreakpointInfoReq == nullptr) {
        std::string error = "ERROR: Invalid parameter, DataBreakpointInfoRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<DataBreakpointInfoResponse>(),
                                                 error, currentRequest->seq);
    }
    string expression = dataBreakpointInfoReq->arguments.name;
    if (expression.empty()) {
        std::string error = "ERROR: Invalid data.";
        return debugger->CompleteFailureResponse(std::make_unique<DataBreakpointInfoResponse>(),
            error, currentRequest->seq);
    }
    SBTarget sbTarget = debugger->GetSbTarget();
    SBFrame sbFrame = sbTarget.GetProcess().GetSelectedThread().GetSelectedFrame();
    if (!dataBreakpointInfoReq->arguments.variablesReference.has_value()) {
        return GenerateSuccessResponse();
    }
    string evaluateName = debugger->GetContainerManager().
        GetExprOfVarRef(dataBreakpointInfoReq->arguments.variablesReference.value());
    if (!evaluateName.empty() && expression.at(0) != '[') {
        evaluateName = evaluateName + ".";
    }
    evaluateName = evaluateName + expression;
    if (!sbFrame) {
        std::string error = "The selected frame is invalid.";
        return debugger->CompleteFailureResponse(std::make_unique<DataBreakpointInfoResponse>(),
                                                 error, currentRequest->seq);
    }
    int64_t containerRef = dataBreakpointInfoReq->arguments.variablesReference.value();
    SBValue value = debugger->GetContainerManager().FindVarSBValue(dataBreakpointInfoReq->arguments.name, containerRef,
        debugger->GetSbDebugger());
    if (!value.IsValid()) {
        std::string error = "Variable not found.";
        return debugger->CompleteFailureResponse(std::make_unique<DataBreakpointInfoResponse>(),
                                                 error, currentRequest->seq);
    }
    // set libraryRenderer to true, avoid crash on macOS
    debugger->UpdateCategory(true);
    // dataId format : variable address + / + variable size
    string dataId = std::to_string(value.GetLoadAddress()) + "/" + std::to_string(value.GetByteSize());
    debugger->UpdateCategory();
    return GenerateSuccessResponse(dataId, evaluateName, true);
}

std::unique_ptr<Response> DataBreakpointInfoApiHandler::GenerateSuccessResponse(std::optional<std::string> dataId,
    std::optional<std::string> name, bool supportDataBreakPoint)
{
    std::unique_ptr<DataBreakpointInfoResponse> responsePointer = std::make_unique<DataBreakpointInfoResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    if (dataId.has_value()) {
        responsePointer->dataId = dataId.value();
    }
    if (name.has_value()) {
        responsePointer->description = name.value();
    }
    if (supportDataBreakPoint) {
        responsePointer->accessTypes = std::vector<DataBreakpointAccessType> {
                DataBreakpointAccessType::READ, DataBreakpointAccessType::WRITE, DataBreakpointAccessType::READ_WRITE
        };
    }
    return responsePointer;
}
