// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SourceApiHandler.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SourceApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SourceRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: SourceRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SourceResponse>(), error, currentRequest->seq);
    }
    return GenerateSuccessResponse(reqPtr->arguments.sourceReference);
}

UniqueResponsePtr SourceApiHandler::GenerateSuccessResponse(int64_t sourceRef)
{
    std::unique_ptr<SourceResponse> responsePointer = std::make_unique<SourceResponse>();
    debugger->InitResponse(*responsePointer, currentRequest->seq, true);
    std::optional<DisassembledRange> optionalRange = debugger->GetDisassemblyManager().FindRangeBySourceRef(sourceRef);
    if (optionalRange) {
        responsePointer->content = optionalRange.value().GetSourceContent();
    } else {
        responsePointer->content = "";
    }
    responsePointer->mimeType = "text/x-dap.disassembly";
    return responsePointer;
}