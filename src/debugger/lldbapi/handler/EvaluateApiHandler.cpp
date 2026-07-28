// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "EvaluateApiHandler.h"
#include "DapLog.h"
#include "VariablesReferenceCreator.h"
#include "ValueParser.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr EvaluateApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    EvaluateResponse res;
    auto reqPtr = dynamic_cast<EvaluateRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: EvaluateRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<EvaluateResponse>(), error, currentRequest->seq);
    }
    SBProcess sbProcess = debugger->GetSbTarget().GetProcess();
    int64_t frameId = reqPtr->arguments.frameId.has_value() ? reqPtr->arguments.frameId.value() : -1;
    SBFrame frame = GetFrameForEvaluation(sbProcess, frameId);
    if (!frame.IsValid()) {
        string error = "ERROR: Invalid frameId.";
        return debugger->CompleteFailureResponse(std::make_unique<EvaluateResponse>(), error, currentRequest->seq);
    }
    string expression = reqPtr->arguments.expression;
    debugger->UpdateCategory(true);
    SBValue value = debugger->FindExprValue(frame, expression);
    return HandleEvaluateValue(reqPtr, value, expression);
}

SBFrame EvaluateApiHandler::GetFrameForEvaluation(SBProcess &sbProcess, int64_t frameId)
{
    if (frameId == -1) {
        SBThread thread = sbProcess.GetSelectedThread();
        return thread.GetSelectedFrame();
    }
    ContainerInfo containerInfo = debugger->GetContainerManager().GetContainerInfo(frameId);
    auto *framePtr = std::get_if<SBFrame>(&containerInfo.container);
    if (framePtr == nullptr) {
        return SBFrame();
    }
    if (debugger->GetOption().OhosCangjieDebug() &&
        debugger->GetCjThreadIndexById(containerInfo.threadId).has_value()) {
        SBCJThread sbCjThread = sbProcess.GetCJThreadAtIndex(
            debugger->GetCjThreadIndexById(containerInfo.threadId).value());
        SBThread hostThread = sbProcess.GetThreadByID(sbCjThread.GetHostThreadID());
        if (!hostThread.IsValid()) {
            hostThread = sbProcess.GetSelectedThread();
        }
        return hostThread.GetFrameAtIndex(containerInfo.frameIndex);
    }
    return *framePtr;
}

UniqueResponsePtr EvaluateApiHandler::HandleEvaluateValue(EvaluateRequest *reqPtr, SBValue &value, string &expression)
{
    string maxStringSummaryLength;
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CPP && reqPtr->arguments.isViewMore.has_value()
        && reqPtr->arguments.isViewMore.value()) {
        maxStringSummaryLength = debugger->GetMaxStringSummaryLength();
        // Set MaxStringSummaryLength to 1048576(1024*1024), a maximum of 1 MB string is displayed.
        debugger->SetMaxStringSummaryLength("1048576");
    }
    ValueParser valueParser(value, debugger);
    string result;
    bool success = valueParser.GetValue(result);
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CPP && reqPtr->arguments.isViewMore.has_value()
        && reqPtr->arguments.isViewMore.value()) {
        if (maxStringSummaryLength.empty()) {
            // If the original value cannot be obtained, the default value(256) is used.
            maxStringSummaryLength = "256";
        }
        debugger->SetMaxStringSummaryLength(maxStringSummaryLength);
    }
    if (success) {
        return GenerateSuccessResponse(value, expression, result);
    } else {
        return debugger->CompleteFailureResponse(std::make_unique<EvaluateResponse>(), result, currentRequest->seq);
    }
}

UniqueResponsePtr EvaluateApiHandler::GenerateSuccessResponse(SBValue &sbValue, string &expression, string &result)
{
    std::unique_ptr<EvaluateResponse> responsePointer = std::make_unique<EvaluateResponse>();
    debugger->InitResponse(*responsePointer,  currentRequest->seq, true);
    responsePointer->result = result;
    responsePointer->resultType = sbValue.GetType().GetName();
    int numChild = GetNumOfVariableChildren(sbValue);
    if (numChild > 0) {
        responsePointer->variablesReference = debugger->GetContainerManager().GetContainerIndex(0, expression);
        debugger->GetContainerManager().SetExprOfVarRef(responsePointer->variablesReference, expression);
        if (ValueParser::IsArrayType(sbValue, debugger->GetDebuggeeLanguageType())) {
            responsePointer->indexedVariables = numChild;
        } else {
            responsePointer->namedVariables = numChild;
        }
    }
    ContainerInfo containerInfo;
    containerInfo.containerType = DapContainer::VALUE;
    containerInfo.container = sbValue;
    debugger->GetContainerManager().SetContainerInfo(responsePointer->variablesReference, containerInfo);
    if (sbValue.GetType().IsPointerType()) {
        responsePointer->memoryReference = sbValue.GetValue();
    }
    lldb::addr_t address = sbValue.GetLoadAddress();
    responsePointer->memoryAddress = CommonUtils::Dec2HexAddress(address, CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU);
    return responsePointer;
}

int EvaluateApiHandler::GetNumOfVariableChildren(SBValue &sbValue)
{
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && sbValue.GetValue() == NULL) {
        // Adapt cangjie option variable internal variables, do not show specific implementation
        // Set child member information only when value and summary are both null
        SBStream sbStream;
        SBTypeSummaryOptions summaryOptions;
        summaryOptions.SetLanguage(debugger->GetLanguageType());
        const char *charSummary = sbValue.GetSummary(sbStream, summaryOptions);
        if (charSummary != nullptr && charSummary[0] != '\0') {
            return 0;
        }
    }
    return sbValue.GetNumChildren();
}