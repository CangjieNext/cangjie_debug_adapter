// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StackTraceApiHandler.h"
#include "DapLog.h"
#include "CommonUtils.h"
#include "DisassemblyManager.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr StackTraceApiHandler::HandleRequest(std::unique_ptr<Request> request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<StackTraceRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: StackTraceRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<StackTraceResponse>(), error, currentRequest->seq);
    }
    auto stackTraceAction = [&](auto&& thread) -> UniqueResponsePtr {
        if (!thread.IsValid()) {
            string error = "ERROR: Invalid thread id.";
            return debugger->CompleteFailureResponse(std::make_unique<StackTraceResponse>(),
                error, currentRequest->seq);
        }
        uint32_t totalFrames = thread.GetNumFrames();
        unsigned int lowFrame = 0;
        unsigned int levels = UINT32_MAX;
        if (reqPtr->arguments.startFrame.has_value() && reqPtr->arguments.startFrame.value() > 0) {
            lowFrame = reqPtr->arguments.startFrame.value();
        }
        if (reqPtr->arguments.levels.has_value() && reqPtr->arguments.levels.value() > 0) {
            levels = reqPtr->arguments.levels.value();
        }
        unsigned int highFrame = lowFrame + levels;
        // overflow
        if (highFrame < lowFrame || highFrame < levels) {
            highFrame = UINT32_MAX;
        }
        for (unsigned i = lowFrame; i < highFrame && i < totalFrames; i++) {
            SBFrame frame = thread.GetFrameAtIndex(i);
            if (!frame.IsValid()) {
                break;
            }
            string frameName = "[" + std::to_string(reqPtr->arguments.threadId) + "," + std::to_string(i) + "]";
            int64_t frameId = debugger->GetContainerManager().GetContainerIndex(0, frameName);
            ContainerInfo containerInfo;
            containerInfo.containerType = DapContainer::STACKFRAME;
            containerInfo.container = frame;
            containerInfo.threadId = reqPtr->arguments.threadId;
            containerInfo.frameIndex = i;
            debugger->GetContainerManager().SetContainerInfo(frameId, containerInfo);
            ParseFrameInfo(frame, frameId);
        }
        return GenerateSuccessResponse(totalFrames);
    };
    if (debugger->GetOption().OhosCangjieDebug()) {
        SBThread thread;
        if (debugger->GetCjThreadIndexById(reqPtr->arguments.threadId).has_value()) {
            SBCJThread sbCjThread = debugger->GetSbTarget().GetProcess().
                GetCJThreadAtIndex(debugger->GetCjThreadIndexById(reqPtr->arguments.threadId).value());
            auto framePc = sbCjThread.GetFrameAtIndex(0).GetPC();
            if (framePc == 0 || framePc == LLDB_INVALID_ADDRESS) {
                return stackTraceAction(debugger->GetSbTarget().GetProcess().
                    GetThreadByID(sbCjThread.GetHostThreadID()));
            }
            return stackTraceAction(sbCjThread);
        } else {
            return stackTraceAction(debugger->GetSbTarget().GetProcess().GetSelectedThread());
        }
    }
    return stackTraceAction(debugger->GetSbTarget().GetProcess().GetThreadByIndexID(reqPtr->arguments.threadId));
}

void StackTraceApiHandler::ParseFrameInfo(lldb::SBFrame &sbFrame, int64_t frameId)
{
    StackFrame stackFrame;
    string unknown = "??";
    stackFrame.id = frameId;
    stackFrame.column = 0;
    stackFrame.instructionPointerReference = CommonUtils::Dec2HexAddress(sbFrame.GetPC(),
        CommonUtils::hexAddressWidth64BitsCpu);
    const char *pFnName = sbFrame.GetFunctionName();
    stackFrame.name = (pFnName != nullptr) ? pFnName : unknown;

    const char *pModuleName = sbFrame.GetModule().GetFileSpec().GetFilename();
    stackFrame.moduleName = (pModuleName != nullptr) ? std::string(pModuleName): unknown;

    lldb::addr_t pcAddr = sbFrame.GetPC();
    string sourcePath = CommonUtils::ReplaceString(GetFilePath(sbFrame, unknown), "\\", "/");
    Source source;
    std::string error;
    DisassemblyManager &disassemblyManager = debugger->GetDisassemblyManager();
    if (sourcePath != unknown && CommonUtils::CheckFilePath(sourcePath, error)) {
        source.path = sourcePath;
        unsigned line = sbFrame.GetLineEntry().GetLine();
        stackFrame.line = debugger->GetOption().linesStartAt1 ? line : line - 1;
        SBAddress startAddr;
        SBAddress endAddr;
        SBInstructionList insList;
        // obtain assembly address range of the stack to support mixed stacks(DevEco).
        disassemblyManager.GetSymbolAddrRangeAndInstructionList(pcAddr, startAddr, endAddr, insList);
        stackFrame.symbolInstructionRange =
                CommonUtils::Dec2HexAddress(startAddr.GetLoadAddress(debugger->GetSbTarget())) + "-" +
                CommonUtils::Dec2HexAddress(endAddr.GetLoadAddress(debugger->GetSbTarget()));
    } else {
        std::optional<DisassembledRange> range = disassemblyManager.GetDisassembledRange(pcAddr);
        stackFrame.line = 0;
        if (range.has_value()) {
            stackFrame.symbolInstructionRange = CommonUtils::Dec2HexAddress(range->GetStartLoadAddress()) + "-" +
                                                CommonUtils::Dec2HexAddress(range->GetEndLoadAddress());
            auto line = range->GetLineNumByAddress(pcAddr);
            stackFrame.line = debugger->GetOption().linesStartAt1 ? line : line - 1;
            source.sourceReference = range.value().GetIndex();
            source.name = range.value().GetSymbolName();
        }
    }
    stackFrame.source = std::move(source);
    vecStackFrame.emplace_back(stackFrame);
}

std::unique_ptr<Response> StackTraceApiHandler::GenerateSuccessResponse(uint32_t totalFrames)
{
    std::unique_ptr<StackTraceResponse> responsePointer = std::make_unique<StackTraceResponse>();
    debugger->InitResponse(*responsePointer,  currentRequest->seq, true);
    responsePointer->stackFrames = std::move(vecStackFrame);
    responsePointer->totalFrames = totalFrames;
    return responsePointer;
}

