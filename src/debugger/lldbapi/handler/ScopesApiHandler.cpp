// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ScopesApiHandler.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr ScopesApiHandler::HandleRequest(UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<ScopesRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ScopesRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<ScopesResponse>(), error, currentRequest->seq);
    }
    curFrameId = reqPtr->arguments.frameId;
    curFrame = debugger->GetLiveFrame(curFrameId);
    if (!curFrame.IsValid()) {
        string error = "ERROR: Invalid frame, curFrameId=" + std::to_string(curFrameId);
        return debugger->CompleteFailureResponse(std::make_unique<ScopesResponse>(), error, currentRequest->seq);
    }
    auto process = debugger->GetSbTarget().GetProcess();
    bool cjThreadSelected = false;
    if (debugger->GetOption().OhosCangjieDebug()) {
        ContainerInfo frameInfo = debugger->GetContainerManager().GetContainerInfo(curFrameId);
        if (frameInfo.threadId != -1 && debugger->GetCjThreadIndexById(frameInfo.threadId).value()) {
            SBCJThread cjThread = process.GetCJThreadAtIndex(debugger->GetCjThreadIndexById(frameInfo.threadId).value());
            if (cjThread.IsValid() && process.SetSelectedCJThread(cjThread)) {
                cjThreadSelected = true;
            }
        }
    }
    if (!cjThreadSelected) {
        process.SetSelectedThread(curFrame.GetThread());
    }
    process.GetSelectedThread().SetSelectedFrame(curFrame.GetFrameID());
    std::vector<Scope> scopes;
    if (debugger->GetDebuggeeLanguageType() != DebuggeeLanguageType::CANGJIE) {
        scopes.emplace_back(GetScope(DapContainer::RETURN_VALUE));
    }
    scopes.emplace_back(GetScope(DapContainer::LOCALS));
    if (debugger->GetOption().showStaticGlobalVars) {
        scopes.emplace_back(GetScope(DapContainer::STATICS));
        scopes.emplace_back(GetScope(DapContainer::GLOBALS));
    }
    scopes.emplace_back(GetScope(DapContainer::REGISTERS));
    return GenerateSuccessResponse(scopes);
}

Scope ScopesApiHandler::GetScope(DapContainer scopeType)
{
    Scope scope;
    string interName;
    switch (scopeType) {
        case DapContainer::LOCALS :
            scope.name = "Locals";
            scope.expensive = false;
            scope.presentationHint = "locals";
            interName = "[Locals]";
            break;
        case DapContainer::STATICS :
            scope.name = "Statics";
            scope.expensive = true;
            scope.presentationHint = "statics";
            interName = "[Statics]";
            break;
        case DapContainer::GLOBALS :
            scope.name = "Globals";
            scope.expensive = true;
            scope.presentationHint = "globals";
            interName = "[Globals]";
            break;
        case DapContainer::REGISTERS :
            scope.name = "Registers";
            scope.expensive = true;
            scope.presentationHint = "registers";
            interName = "[Registers]";
            break;
        case DapContainer::RETURN_VALUE :
            scope.name = "ReturnValues";
            scope.expensive = false;
            scope.presentationHint = "return values";
            interName = "[ReturnValues]";
            break;
        default :
            scope.name = "Scope not supported";
            return scope;
    }
    scope.variablesReference = debugger->GetContainerManager().GetContainerIndex(curFrameId, interName);
    ContainerInfo containerInfo;
    containerInfo.containerType = scopeType;
    containerInfo.container = curFrame;
    ContainerInfo frameInfo = debugger->GetContainerManager().GetContainerInfo(curFrameId);
    containerInfo.threadId = frameInfo.threadId;
    containerInfo.frameIndex = frameInfo.frameIndex;
    debugger->GetContainerManager().SetContainerInfo(scope.variablesReference, containerInfo);
    scope.expensive = false;
    return scope;
}

UniqueResponsePtr ScopesApiHandler::GenerateSuccessResponse(std::vector<Scope> &scopes)
{
    std::unique_ptr<ScopesResponse> responsePtr = std::make_unique<ScopesResponse>();
    debugger->InitResponse(*responsePtr,  currentRequest->seq, true);
    responsePtr->scopes = std::move(scopes);
    return responsePtr;
}

