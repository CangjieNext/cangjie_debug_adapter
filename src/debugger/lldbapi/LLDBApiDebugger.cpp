// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LLDBApiDebugger.h"
#include <thread>
#include "DapLog.h"
#include "CommonUtils.h"
#include "InitializeApiHandler.h"
#include "LaunchApiHandler.h"
#include "AttachApiHandler.h"
#include "SetBreakpointsApiHandler.h"
#include "SetFunctionBreakpointsApiHandler.h"
#include "SetExceptionBreakpointsApiHandler.h"
#include "StackTraceApiHandler.h"
#include "LLDBApiEventHandler.h"
#include "DataBreakpointInfoApiHandler.h"
#include "SetDataBreakpointsApiHandler.h"
#include "SetInstructionBreakpointsApiHandler.h"
#include "ScopesApiHandler.h"
#include "AddSourceMapApiHandler.h"
#include "ResetSourceMapApiHandler.h"
#include "VariablesApiHandler.h"
#include "EvaluateApiHandler.h"
#include "SetVariableApiHandler.h"
#include "SourceApiHandler.h"
#include "DisassembleApiHandler.h"
#include "GetBitmapApiHandler.h"
#include "BreakpointLocationsApiHandler.h"
#include "VariableHexViewHandler.h"
#include "CommandCompletionApiHandler.h"
#include "TimeTravelDebugManager.h"
#include "EnumUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace std::placeholders;
using namespace lldb;
using namespace Dap;
using namespace Dap::Protocol;

LLDBApiDebugger::LLDBApiDebugger(const LLDBApiDebuggerOption &debuggerOption)
    : Debugger(), option(debuggerOption)
{
    SBDebugger::Initialize();
    sbDebugger = SBDebugger::Create();
    sbDebugger.SetAsync(true);
    if (!AddInternalTypeSummaries()) {
        LOG_INFO("AddInternalTypeSummaries() failed !");
    }
    Configure();
}

void LLDBApiDebugger::Configure()
{
    requestHandlerMap.clear();
    requestHandlerMap = {
        { REQ_RES_INITIALIZE, std::bind(&LLDBApiDebugger::Initialize, this, _1) },
        { REQ_RES_LAUNCH, std::bind(&LLDBApiDebugger::Launch, this, _1) },
        { REQ_RES_ATTACH, std::bind(&LLDBApiDebugger::Attach, this, _1) },
        { REQ_RES_CONTINUE, std::bind(&LLDBApiDebugger::Continue, this, _1) },
        { REQ_RES_PAUSE, std::bind(&LLDBApiDebugger::Pause, this, _1) },
        { REQ_RES_DISCONNECT, std::bind(&LLDBApiDebugger::Disconnect, this, _1) },
        { REQ_RES_CONFIGURATION_DONE, std::bind(&LLDBApiDebugger::ConfigurationDone, this, _1) },
        { REQ_RES_NEXT, std::bind(&LLDBApiDebugger::Next, this, _1) },
        { REQ_RES_STEP_IN, std::bind(&LLDBApiDebugger::StepIn, this, _1) },
        { REQ_RES_STEP_OUT, std::bind(&LLDBApiDebugger::StepOut, this, _1) },
        { REQ_RES_SET_BREAKPOINTS, std::bind(&LLDBApiDebugger::SetBreakpoint, this, _1) },
        { REQ_RES_SET_FUNCTION_BREAKPOINTS, std::bind(&LLDBApiDebugger::SetFunctionBreakpoint, this, _1) },
        { REQ_RES_SET_EXCEPTION_BREAKPOINTS, std::bind(&LLDBApiDebugger::SetExceptionBreakpoint, this, _1) },
        { REQ_RES_DATA_BREAKPOINT_INFO, std::bind(&LLDBApiDebugger::DataBreakpointInfo, this, _1) },
        { REQ_RES_SET_DATA_BREAKPOINTS, std::bind(&LLDBApiDebugger::SetDataBreakpoints, this, _1) },
        { REQ_RES_SET_INSTRUCTION_BREAKPOINTS, std::bind(&LLDBApiDebugger::SetInstructionBreakpoints, this, _1) },
        { REQ_RES_DEBUG_IN_CONSOLE, std::bind(&LLDBApiDebugger::DebugInConsole, this, _1) },
        { REQ_RES_THREADS, std::bind(&LLDBApiDebugger::Threads, this, _1) },
        { REQ_RES_STACK_TRACE, std::bind(&LLDBApiDebugger::StackTrace, this, _1) },
        { REQ_RES_SCOPES, std::bind(&LLDBApiDebugger::Scopes, this, _1) },
        { REQ_RES_VARIABLES, std::bind(&LLDBApiDebugger::Variables, this, _1) },
        { REQ_RES_EVALUATE, std::bind(&LLDBApiDebugger::Evaluate, this, _1) },
        { REQ_RES_SET_VARIABLE, std::bind(&LLDBApiDebugger::SetVariable, this, _1) },
        { REQ_RES_SOURCE, std::bind(&LLDBApiDebugger::Source, this, _1) },
        { REQ_RES_READ_MEMORY, std::bind(&LLDBApiDebugger::ReadMemory, this, _1) },
        { REQ_RES_WRITE_MEMORY, std::bind(&LLDBApiDebugger::WriteMemory, this, _1) },
        { REQ_RES_DISASSEMBLE, std::bind(&LLDBApiDebugger::Disassemble, this, _1) },
        { REQ_RES_REVERSE_STEP, std::bind(&LLDBApiDebugger::ReverseStep, this, _1) },
        { REQ_RES_UNDO, std::bind(&LLDBApiDebugger::Undo, this, _1) },
        { REQ_RES_REVERSE_CONTINUE, std::bind(&LLDBApiDebugger::ReverseContinue, this, _1) },
        { REQ_RES_SET_EXECUTION_POINT, std::bind(&LLDBApiDebugger::SetExecutionPoint, this, _1) },
        { REQ_RES_GOTO_RECORD_POINT, std::bind(&LLDBApiDebugger::GotoRecordPoint, this, _1) },
        { REQ_RES_RECORD_POINTS,  std::bind(&LLDBApiDebugger::RecordPoints, this, _1) },
        { REQ_RES_CONTINUE_IN_REVERSE, std::bind(&LLDBApiDebugger::ContinueInReverse, this, _1) },
        { REQ_RES_STEP_IN_REVERSE, std::bind(&LLDBApiDebugger::StepInReverse, this, _1) },
        { REQ_RES_CACHE_DATA_CONFIG,  std::bind(&LLDBApiDebugger::CacheDataConfig, this, _1) },
        { REQ_RES_RECORD_MESSAGE,  std::bind(&LLDBApiDebugger::RecordMessage, this, _1) },
        { REQ_RES_GET_BITMAP, std::bind(&LLDBApiDebugger::GetBitmap, this, _1) },
        { REQ_RES_LOAD_MODULES, std::bind(&LLDBApiDebugger::LoadModules, this, _1) },
        { REQ_RES_ADD_SOURCE_MAPPING, std::bind(&LLDBApiDebugger::AddSourceMapping, this, _1) },
        { REQ_RES_RESET_SOURCE_MAPPING, std::bind(&LLDBApiDebugger::ResetSourceMapping, this, _1) },
        { REQ_RES_BREAKPOINT_LOCATIONS, std::bind(&LLDBApiDebugger::BreakpointLocations, this, _1) },
        { REQ_RES_ADD_SOURCE_BREAKPOINT, std::bind(&LLDBApiDebugger::AddSourceBreakpoint, this, _1) },
        { REQ_RES_DELETE_BREAKPOINT, std::bind(&LLDBApiDebugger::DeleteBreakpoint, this, _1) },
        { REQ_RES_VARIABLE_HEX_VIEW, std::bind(&LLDBApiDebugger::VariableHexView, this, _1) },
        { REQ_RES_COMMAND_COMPLETION, std::bind(&LLDBApiDebugger::CommandCompletion, this, _1) }
    };
}

static bool CharSummaryProvider(lldb::SBValue value, lldb::SBTypeSummaryOptions, lldb::SBStream &stream)
{
    if (!value.IsValid()) {
        LOG_INFO("SBValue invalid !");
        return false;
    }
    lldb::SBType valueType = value.GetType();
    if (!valueType.IsValid()) {
        LOG_INFO("SBType invalid !");
        return false;
    }
    lldb::BasicType basicType = valueType.GetBasicType();
    const char *withNullAsEmpty = value.GetValue() ? value.GetValue() : "";
    if (basicType == lldb::eBasicTypeUnsignedChar) {
        stream.Printf("%u %s", static_cast<unsigned int>(value.GetValueAsUnsigned()), withNullAsEmpty);
    } else if (basicType == lldb::eBasicTypeSignedChar) {
        stream.Printf("%d %s", static_cast<int>(value.GetValueAsSigned()), withNullAsEmpty);
    } else {
        return false;
    }

    return true;
}

static bool AddTypeSummary(lldb::SBTypeCategory category, const char *typeName,
                           lldb::SBTypeSummary::FormatCallback cb, uint32_t typeOptions, bool regex = false)
{
    lldb::SBTypeSummary summary = lldb::SBTypeSummary::CreateWithCallback(cb, typeOptions);
    if (summary.IsValid()) {
        return category.AddTypeSummary(lldb::SBTypeNameSpecifier(typeName, regex), summary);
    } else {
        return false;
    }
}

bool LLDBApiDebugger::LoadInternalSummaries(SBTypeCategory category)
{
    if (!AddTypeSummary(category, "unsigned char", CharSummaryProvider,
                        lldb::eTypeOptionSkipPointers | lldb::eTypeOptionHideValue)) {
        return false;
    }
    if (!AddTypeSummary(category, "signed char", CharSummaryProvider,
                        lldb::eTypeOptionSkipPointers | lldb::eTypeOptionHideValue)) {
        return false;
    }
    if (!AddTypeSummary(category, "char", CharSummaryProvider,
                        lldb::eTypeOptionHideValue | lldb::eTypeOptionSkipPointers)) {
        return false;
    }
    return true;
}

bool LLDBApiDebugger::AddInternalTypeSummaries()
{
    static const char *categoryName = "CPlusPlus";
    SBTypeCategory myCategory = sbDebugger.CreateCategory(categoryName);
    if (!myCategory.IsValid()) {
        LOG_INFO("SBTypeCategory invalid !");
        return false;
    }
    if (!LoadInternalSummaries(myCategory)) {
        sbDebugger.DeleteCategory(categoryName);
        return false;
    }
    myCategory.SetEnabled(true);
    return true;
}

int LLDBApiDebugger::OnDapRequest(UniqueRequestPtr request)
{
    int preHandleResult = PreHandleRequest(request.get());
    if (preHandleResult != 0) {
        return preHandleResult;
    }
    string command = request->command;
    unsigned int seq = request->seq;
    if (request->command == REQ_RES_SET_VARIABLE) {
        auto reqPtr = dynamic_cast<SetVariableRequest *>(request.get());
        changedVar.parentRef = reqPtr->arguments.variablesReference;
        changedVar.varName = reqPtr->arguments.name;
    }
    UniqueResponsePtr response;
    RequestArgumentsContent content = timeTravelManager->GetArgumentsContent(dynamic_cast<Request*>(request.get()));
    if (timeTravelManager->IsUndoEnable()) { // time travel debug response
        std::shared_ptr<Response> undoResponse = timeTravelManager->GetDebugInfoResponseInUndo(content, command, seq);
        if (undoResponse != nullptr) {
            SendResponse(*undoResponse);
            LOG_INFO("handle reverse request end!");
            return 0;
        }
        response = OnDapRequestInUndo(std::move(request), command, seq);
        SendResponse(*response);
        if (timeTravelManager->IsTriggeredUndoEvent(command) && response->success) {
            SendStoppedEventInReverseDebug();
        }
    } else { // normal response
        response = OnDapRequestInNormal(std::move(request), command, seq);
        if (response == nullptr) {
            LOG_INFO("do not send response when process ", command, " request");
            return 0;
        }
        if (IsInternalRequest(response->request_seq)) {
            debugCacheHelper->HandleResponse(*response);
        } else {
            SendResponse(*response);
        }
        if (response->command == REQ_RES_SET_VARIABLE && response->success &&
            timeTravelManager->GetRecodeNodeQueueSize() > 0) {
            // update variables cache when set variable to display the modified value in the reverse mode.
            auto res = dynamic_cast<SetVariableResponse*>(response.get());
            timeTravelManager->UpdateVariablesCache(changedVar.varName, changedVar.parentRef, res->value);
        } else if (response->command == REQ_RES_SET_EXECUTION_POINT && response->success) {
            StoppedEvent stoppedApiEvent;
            stoppedApiEvent.seq = GetAndIncCurrentMessageSeq();
            stoppedApiEvent.body.reason = StoppedReason::SET_EXECUTIONPOINT;
            stoppedApiEvent.body.threadId = GetEventHandler()->GetPreviousStoppedThreadID();
            stoppedApiEvent.body.allThreadsStopped = true;
            timeTravelManager->CreateRecordNode(stoppedApiEvent.body);
            std::thread t = std::thread([stoppedApiEvent, this] {
                debugCacheHelper->StartCaching(stoppedApiEvent);
                SendEvent(stoppedApiEvent);
                RecordPointsUpdateEvent event;
                event.body = timeTravelManager->GetRecordPointsUpdateEventBody();
                event.seq = GetAndIncCurrentMessageSeq();
                SendEvent(event);
            });
            t.join();
            return 0;
        }
        // Record command
        bool isRecordPointSupported = timeTravelManager != nullptr ?
            timeTravelManager->IsSupportedRecordPointType(eventHandler->GetCurrentStoppedEvent().body.reason) : false;
        if (timeTravelManager != nullptr && GetDebugCacheHelper()->CacheInServer() && isRecordPointSupported) {
            timeTravelManager->RecordResponse(content, command, std::shared_ptr<Response>(std::move(response)));
        }
    }
    LOG_INFO("handle request end!");
    return 0;
}

int LLDBApiDebugger::PreHandleRequest(Protocol::Request *request)
{
    if (request == nullptr) {
        LOG_ERROR("request is nullptr in OnDapRequest method!");
        return -1;
    }
    LOG_INFO("request = ", request->command, ", seq = ", request->seq);
    if (requestHandlerMap.count(request->command) < 1) {
        LOG_ERROR("command not supportted: ", request->command);
        return -1;
    }
    if (IsDapInitialized() == false && request->command != "initialize") {
        LOG_ERROR("Error: first command is not initialize request !");
        return NO_INITIALIZE_REQUEST;
    }
    return 0;
}

void LLDBApiDebugger::OnDapResponse(UniqueResponsePtr response)
{
    if (response == nullptr) {
        LOG_ERROR("response is nullptr in OnDapResponse method!");
        return;
    }
    LOG_INFO("response = ", response->command, ", success = ", response->success);
    RunInTerminalResponse *runInTerminalRes;
    if (response->command == "runInTerminal") {
        runInTerminalRes = dynamic_cast<RunInTerminalResponse *>(response.get());
    } else {
        LOG_ERROR("ERROR: this response is not supported, command = ", response->command);
        return;
    }
    if (runInTerminalRes == nullptr) {
        LOG_ERROR("ERROR: RunInTerminalResponse Pointer conversion failed.");
        return;
    }
    this->runInTerminalResponse = *runInTerminalRes;
}

// only when exit reverse mode in undo, the value of isReverseMode is false.
void LLDBApiDebugger::SendStoppedEventInReverseDebug(bool isReverseMode)
{
    StoppedEvent stoppedApiEvent;
    stoppedApiEvent.seq = GetAndIncCurrentMessageSeq();
    if (timeTravelManager->GetCurrentRecordNode()) {
        stoppedApiEvent.body = timeTravelManager->GetCurrentRecordNode()->stoppedEventBody;
        if (stoppedApiEvent.body.reason == StoppedReason::STEP) {
            // Determine whether the current stop point is in source code or disassemble by steppingGranularity
            if (timeTravelManager->GetCurrentRecordNode()->steppingGranularity == SteppingGranularity::INSTRUCTION) {
                stoppedApiEvent.body.reason = StoppedReason::INSTRUCTION_BREAKPOINT;
            } else if (timeTravelManager->GetCurrentRecordNode()->steppingGranularity == SteppingGranularity::LINE) {
                stoppedApiEvent.body.reason = StoppedReason::BREAKPOINT;
            }
        }
    } else {
        LOG_INFO("can not get current RecordNode.");
        stoppedApiEvent.body.reason = StoppedReason::UNKNOWN;
        stoppedApiEvent.body.threadId = 1;
        stoppedApiEvent.body.allThreadsStopped = true;
    }
    if (isReverseMode) {
        stoppedApiEvent.body.isReverseMode = true;
    }
    SendEvent(stoppedApiEvent);
}

void LLDBApiDebugger::Start()
{
}

void LLDBApiDebugger::Stop()
{
    if (eventHandler != nullptr) {
        eventHandler->StopListener();
    }
}

UniqueResponsePtr LLDBApiDebugger::Initialize(UniqueRequestPtr request)
{
    InitializeApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Launch(UniqueRequestPtr request)
{
    // handler will be used in another thread;
    std::shared_ptr<LaunchApiHandler> handler = std::make_shared<LaunchApiHandler>(shared_from_this());
    return handler->HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Attach(UniqueRequestPtr request)
{
    AttachApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Continue(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ContinueRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ContinueRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ContinueResponse>(), error, request->seq);
    }

    if (GetDebuggeeState() == DebuggeeState::RUNNING) {
        string error = "ERROR: the debuggee is already running!";
        return CompleteFailureResponse(std::make_unique<ContinueResponse>(), error, request->seq);
    }

    PreResume();
    SBError sbError = sbTarget.GetProcess().Continue();
    if (sbError.GetCString() != nullptr) {
        LOG_ERROR("failed to continue: ", sbError.GetCString());
        return CompleteFailureResponse(std::make_unique<ContinueResponse>(), sbError.GetCString(), request->seq);
    }
    std::unique_ptr<ContinueResponse> responsePointer = std::make_unique<ContinueResponse>();
    InitResponse(*responsePointer, request->seq, true);
    responsePointer->allThreadsContinued = true;
    return responsePointer;
}

UniqueResponsePtr LLDBApiDebugger::Pause(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<PauseRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: PauseRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<PauseResponse>(), error, request->seq);
    }
    if (GetDebuggeeState() == DebuggeeState::STOPPED) {
        string error = "ERROR: Debuggee is already in paused state, return directly!";
        return CompleteFailureResponse(std::make_unique<PauseResponse>(), error, request->seq);
    }
    SetPauseOperation(true);
    SBError sbError = sbTarget.GetProcess().Stop();
    if (sbError.GetCString() != nullptr) {
        LOG_ERROR("failed to pause: ", sbError.GetCString());
        SetPauseOperation(false);
        return CompleteFailureResponse(std::make_unique<PauseResponse>(), sbError.GetCString(), request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<PauseResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::Disconnect(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<DisconnectRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: DisconnectRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<DisconnectResponse>(), error, request->seq);
    }
    SBError sbError;
    if (GetOption().startMode == StartMode::ATTACH_MODE) {
        if (reqPtr->arguments.has_value() && reqPtr->arguments->isDebuggeeCrashed.has_value()
            && reqPtr->arguments->isDebuggeeCrashed.value()) {
            LOG_INFO("debuggee has already crashed, disconnect directly.");
            sbDebugger.GetSelectedPlatform().DisconnectRemote();
            return CompleteSuccessResponse(std::make_unique<DisconnectResponse>(), request->seq);
        }
        // if process is stopped when disconnect, resume process
        if (GetDebuggeeState() == DebuggeeState::STOPPED) {
            // disable all breakpoints to avoid continue to bkpt
            sbTarget.DisableAllBreakpoints();
            sbTarget.DisableAllWatchpoints();
            sbError = sbTarget.GetProcess().Continue();
            if (sbError.GetCString() != nullptr) {
                LOG_ERROR("failed to continue: ", sbError.GetCString());
            }
        }
        sbError = sbTarget.GetProcess().Detach(false);
        if (sbError.GetCString() != nullptr) {
            LOG_ERROR("failed to detach: ", sbError.GetCString());
            this->OnDebuggerExit(-1);
            sbDebugger.GetSelectedPlatform().DisconnectRemote();
            return CompleteFailureResponse(std::make_unique<DisconnectResponse>(), sbError.GetCString(), request->seq);
        }
    } else if (GetOption().startMode == StartMode::LAUNCH_MODE) {
#ifdef _WIN32
        if (!GetOption().externalConsole) {
            // AttachConsole in launch handler
            FreeConsole();
        }
#endif
    }
    if (option.remote == RemoteClass::DEBUGGER_SERVER_REMOTE) {
        sbDebugger.GetSelectedPlatform().DisconnectRemote();
        LOG_INFO("Disconnect from ", option.remotePlatform);
    }
    Stop();
    eventHandler.reset();
    lldb::SBDebugger::Destroy(sbDebugger);
    this->OnDebuggerExit(0);
    return CompleteSuccessResponse(std::make_unique<DisconnectResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::ConfigurationDone(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ConfigurationDoneRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ConfigurationDoneRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ConfigurationDoneResponse>(), error, request->seq);
    }
    SBError sbError = SBError();
    if (option.startMode == StartMode::ATTACH_MODE || option.launchParams.debuggeeStarted) {
        sbError = sbTarget.GetProcess().Continue();
    } else if (option.startMode == StartMode::LAUNCH_MODE) {
        lldb::SBLaunchInfo sbLaunchInfo = sbTarget.GetLaunchInfo();
        SBBreakpoint sbBreakpoint;
        if (option.stopAtEntry && !GetFunctionBreakpointMap().HasFunction("main")) {
            sbBreakpoint = sbTarget.BreakpointCreateByName("main");
            if (sbBreakpoint.IsValid()) {
                GetFunctionBreakpointMap().AddBreakPoint("main", sbBreakpoint.GetID());
                entryBreakpointID = sbBreakpoint.GetID();
            }
        }
        SBProcess process = sbTarget.Launch(sbLaunchInfo, sbError);
    }
    if (sbError.GetCString() != nullptr) {
        LOG_ERROR("Failed: ", sbError.GetCString());
        return CompleteFailureResponse(std::make_unique<ConfigurationDoneResponse>(),
            sbError.GetCString(), request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<ConfigurationDoneResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::Next(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<NextRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: NextRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<NextResponse>(), error, request->seq);
    }
    auto nextAction = [&](auto&& thread) -> UniqueResponsePtr {
        if (!thread.IsValid()) {
            string error = "ERROR: Invalid thread id.";
            return CompleteFailureResponse(std::make_unique<NextResponse>(), error, request->seq);
        }

        PreResume();
        SBError sbError;
        if (reqPtr->arguments.granularity.has_value()) {
            SetCurrentStepGranularity(reqPtr->arguments.granularity.value());
            if (reqPtr->arguments.granularity.value() == SteppingGranularity::INSTRUCTION) {
                thread.StepInstruction(true, sbError);
            } else {
                thread.StepOver(lldb::eOnlyDuringStepping, sbError);
            }
        } else {
            SetCurrentStepGranularity(SteppingGranularity::LINE);
            SBFrame sbFrame = thread.GetFrameAtIndex(0);
            if (IsInDisassembly(sbFrame)) {
                thread.StepInstruction(true, sbError);
            } else {
                thread.StepOver(lldb::eOnlyDuringStepping, sbError);
            }
        }
        if (sbError.GetCString() != nullptr) {
            LOG_ERROR("failed to next: ", sbError.GetCString());
            return CompleteFailureResponse(std::make_unique<NextResponse>(), sbError.GetCString(), request->seq);
        }
        return CompleteSuccessResponse(std::make_unique<NextResponse>(), request->seq);
    };
    if (GetOption().OhosCangjieDebug()) {
        SBThread thread;
        if (GetCjThreadIndexById(reqPtr->arguments.threadId).has_value()) {
            SBCJThread sbCjThread = GetSbTarget().GetProcess().
                GetCJThreadAtIndex(GetCjThreadIndexById(reqPtr->arguments.threadId).value());
            thread = GetSbTarget().GetProcess().GetThreadByID(sbCjThread.GetHostThreadID());
        } else {
            thread = GetSbTarget().GetProcess().GetSelectedThread();
        }
        return nextAction(thread);
    }
    return nextAction(GetSbTarget().GetProcess().GetThreadByIndexID(reqPtr->arguments.threadId));
}

UniqueResponsePtr LLDBApiDebugger::StepIn(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<StepInRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: StepInRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<StepInResponse>(), error, request->seq);
    }
    auto stepInAction = [&](auto&& thread) -> UniqueResponsePtr {
        if (!thread.IsValid()) {
            string error = "ERROR: Invalid thread id.";
            return CompleteFailureResponse(std::make_unique<StepInResponse>(), error, request->seq);
        }

        PreResume();
        SBError sbError;
        if (reqPtr->arguments.granularity.has_value()) {
            SetCurrentStepGranularity(reqPtr->arguments.granularity.value());
            if (reqPtr->arguments.granularity.value() == SteppingGranularity::INSTRUCTION) {
                thread.StepInstruction(false, sbError);
            } else {
                thread.StepInto(nullptr, LLDB_INVALID_LINE_NUMBER, sbError, lldb::eOnlyDuringStepping);
            }
        } else {
            SetCurrentStepGranularity(SteppingGranularity::LINE);
            SBFrame sbFrame = thread.GetFrameAtIndex(0);
            if (IsInDisassembly(sbFrame)) {
                thread.StepInstruction(false, sbError);
            } else {
                thread.StepInto(nullptr, LLDB_INVALID_LINE_NUMBER, sbError, lldb::eOnlyDuringStepping);
            }
        }
        if (sbError.GetCString() != nullptr) {
            LOG_ERROR("failed to next: ", sbError.GetCString());
            return CompleteFailureResponse(std::make_unique<StepInResponse>(), sbError.GetCString(), request->seq);
        }
        return CompleteSuccessResponse(std::make_unique<StepInResponse>(), request->seq);
    };
    if (GetOption().OhosCangjieDebug()) {
        SBThread thread;
        if (GetCjThreadIndexById(reqPtr->arguments.threadId).has_value()) {
            SBCJThread sbCjThread = GetSbTarget().GetProcess().
                GetCJThreadAtIndex(GetCjThreadIndexById(reqPtr->arguments.threadId).value());
            thread = GetSbTarget().GetProcess().GetThreadByID(sbCjThread.GetHostThreadID());
        } else {
            thread = GetSbTarget().GetProcess().GetSelectedThread();
        }
        return stepInAction(thread);
    }
    return stepInAction(sbTarget.GetProcess().GetThreadByIndexID(reqPtr->arguments.threadId));
}

UniqueResponsePtr LLDBApiDebugger::StepOut(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<StepOutRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: StepOutRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<StepOutResponse>(), error, request->seq);
    }
    auto stepOutAction = [&](auto&& thread) -> UniqueResponsePtr {
        if (!thread.IsValid()) {
            string error = "ERROR: Invalid thread id.";
            return CompleteFailureResponse(std::make_unique<StepOutResponse>(), error, request->seq);
        }

        PreResume();
        SBError sbError;
        if (reqPtr->arguments.granularity.has_value()) {
            SetCurrentStepGranularity(reqPtr->arguments.granularity.value());
        } else {
            SetCurrentStepGranularity(SteppingGranularity::LINE);
        }
        thread.StepOut(sbError);
        if (sbError.GetCString() != nullptr) {
            LOG_ERROR("failed to next: ", sbError.GetCString());
            return CompleteFailureResponse(std::make_unique<StepOutResponse>(), sbError.GetCString(), request->seq);
        }
        return CompleteSuccessResponse(std::make_unique<StepOutResponse>(), request->seq);
    };
    if (GetOption().OhosCangjieDebug()) {
        SBThread thread;
        if (GetCjThreadIndexById(reqPtr->arguments.threadId).has_value()) {
            SBCJThread sbCjThread = GetSbTarget().GetProcess().
                GetCJThreadAtIndex(GetCjThreadIndexById(reqPtr->arguments.threadId).value());
            thread = GetSbTarget().GetProcess().GetThreadByID(sbCjThread.GetHostThreadID());
        } else {
            thread = GetSbTarget().GetProcess().GetSelectedThread();
        }
        return stepOutAction(thread);
    }
    return stepOutAction(sbTarget.GetProcess().GetThreadByIndexID(reqPtr->arguments.threadId));
}

std::string LLDBApiDebugger::GetAddressRange(lldb::SBModule &sbModule)
{
    std::string addressRangeInfo;
    SBAddress sbAddress = sbModule.GetObjectFileHeaderAddress();
    const lldb::addr_t loadAddress = sbAddress.GetLoadAddress(sbTarget);
    std::string startAddr = loadAddress != LLDB_INVALID_ADDRESS ?
            CommonUtils::Dec2HexAddress(loadAddress, CommonUtils::hexAddressWidth64BitsCpu) : "-";
    const lldb::addr_t moduleSize = sbModule.GetAddressByteSize();
    const lldb::addr_t endAddress = loadAddress + moduleSize;
    std::string endAddr = endAddress != LLDB_INVALID_ADDRESS ?
            CommonUtils::Dec2HexAddress(endAddress, CommonUtils::hexAddressWidth64BitsCpu) : "-";
    addressRangeInfo = startAddr + "~" + endAddr;
    return addressRangeInfo;
}

std::string LLDBApiDebugger::GetCodeRange(lldb::SBModule &sbModule)
{
    std::string codeRangeInfo;
    SBAddress sbAddress = sbModule.GetObjectFileHeaderAddress();
    const lldb::addr_t loadAddress = sbAddress.GetLoadAddress(sbTarget);
    size_t sectionNum = sbModule.GetNumSections();
    for (size_t i = 0; i < sectionNum; i++) {
        SBSection section = sbModule.GetSectionAtIndex(i);
        if (section.GetPermissions() == (ePermissionsExecutable | ePermissionsReadable)) {
            lldb::addr_t loadAddr = section.GetLoadAddress(sbTarget);
            std::string startAddr = loadAddress != LLDB_INVALID_ADDRESS ?
                        CommonUtils::Dec2HexAddress(loadAddr, CommonUtils::hexAddressWidth64BitsCpu) : "-";
            codeRangeInfo += startAddr;
            codeRangeInfo += "~";
            loadAddr += section.GetByteSize();
            std::string endAddr = loadAddress != LLDB_INVALID_ADDRESS ?
                        CommonUtils::Dec2HexAddress(loadAddr, CommonUtils::hexAddressWidth64BitsCpu) : "-";
            codeRangeInfo += endAddr;
        }
    }
    return codeRangeInfo;
}

void LLDBApiDebugger::GetModuleInfo(SBModule &sbModule, Module &module)
{
    const char *sModuleUUID = sbModule.GetUUIDString();
    const char *sModuleFileName = sbModule.GetFileSpec().GetFilename();
    std::string uuidStr = (sModuleUUID != nullptr) ? std::string(sModuleUUID) : "";
    module.id = GetIndexByUUID(uuidStr);
    module.name = (sModuleFileName != nullptr) ? std::string(sModuleFileName) : "";
    module.uuid = uuidStr;
    std::unique_ptr<char[]> path = make_unique<char[]>(PATH_MAX);
    sbModule.GetPlatformFileSpec().GetPath(path.get(), PATH_MAX);
    module.platformFilePath = string(path.get());
    sbModule.GetFileSpec().GetPath(path.get(), PATH_MAX);
    module.path = string(path.get());
    // isOptimized not supported yet
    module.isOptimized = true;
    module.isUserCode = false;
    GetModuleVersion(sbModule, module);
    // Checking if a module has any compile units (source files) to judge if there is debug information in a module
    if (sbModule.GetNumCompileUnits() == 0) {
        module.symbolStatus = "Symbols not Found";
    } else {
        module.symbolStatus = "Symbols Loaded";
    }
    sbModule.GetSymbolFileSpec().GetPath(path.get(), PATH_MAX);
    module.symbolFilePath = string(path.get());
    module.dateTimeStamp = Dap::TimeUtils::Instance().NowStr(Dap::TimeStyle::DATE_TIME_4);
    module.codeRange = GetCodeRange(sbModule);
    module.addressRange = GetAddressRange(sbModule);
    SBAddress sbAddress = sbModule.GetObjectFileHeaderAddress();
    const lldb::addr_t loadAddress = sbAddress.GetLoadAddress(sbTarget);
    module.loadedAddress = loadAddress != LLDB_INVALID_ADDRESS ?
            CommonUtils::Dec2HexAddress(loadAddress, CommonUtils::hexAddressWidth64BitsCpu) : "-";
}

void LLDBApiDebugger::GetModuleVersion(SBModule &sbModule, Module &module)
{
    int numVersions = sbModule.GetVersion(nullptr, 0);
    // if binary file of a module has no version information, we extract version from the name of module file
    // for example: libxxx.1.1.2.z.so, the version is 1.1.2
    if (numVersions != 0) {
        uint32_t* version = nullptr;
        sbModule.GetVersion(version, numVersions);
        if (version == nullptr) {
            return;
        }
        for (int i = 0; i < numVersions; i++) {
            module.version->append(std::to_string(*(version + i)));
            if (i < numVersions - 1) {
                module.version->append(".");
            }
        }
    } else {
        std::smatch results;
        if (CommonUtils::RegexSearch(module.name, results, "(\\d+\\.){2,10}")) {
            module.version = results[0].str().substr(0, results[0].length() - 1);
        }
    }
}

int LLDBApiDebugger::GetIndexByUUID(const std::string &uuidStr)
{
    if (!uuidStr.empty()) {
        const unsigned totalNumber = sbTarget.GetNumModules();
        for (unsigned index = 0; index < totalNumber; index++) {
            SBModule module = sbTarget.GetModuleAtIndex(index);
            const char *sModuleUUID = module.GetUUIDString();
            if ((sModuleUUID != nullptr) && (uuidStr == std::string(sModuleUUID))) {
                return index;
            }
        }
    }
    return -1;
}

std::optional<SBModule> LLDBApiDebugger::GetModuleByUUID(const std::string &uuidStr)
{
    if (!uuidStr.empty()) {
        const unsigned totalNumber = sbTarget.GetNumModules();
        for (unsigned index = 0; index < totalNumber; index++) {
            SBModule module = sbTarget.GetModuleAtIndex(index);
            const char *sModuleUUID = module.GetUUIDString();
            if ((sModuleUUID != nullptr) && (uuidStr == std::string(sModuleUUID))) {
                return module;
            }
        }
    }
    return std::nullopt;
}

std::string LLDBApiDebugger::ExtractModuleFilePath(const std::string &commandResult)
{
    // command result: symbol file '{symbol file path}' has been added to '{module file path}'\n
    int end = commandResult.find_last_of("'");
    int start = commandResult.find_last_of("'", end - 1) + 1;
    return commandResult.substr(start, end - start);
}

UniqueResponsePtr LLDBApiDebugger::LoadModules(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<LoadModuleRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: LoadModulesRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<LoadModulesResponse>(), error, request->seq);
    }
    if (reqPtr->arguments.pathList.size() == 0 || reqPtr->arguments.uuidList.size() == 0
            || reqPtr->arguments.pathList.size() != reqPtr->arguments.uuidList.size()) {
        string error = "ERROR: Size of moduleIdList and pathList should be same (not 0)";
        return CompleteFailureResponse(std::make_unique<LoadModulesResponse>(), error, request->seq);
    }

    std::unique_ptr<LoadModulesResponse> responsePointer = std::make_unique<LoadModulesResponse>();
    InitResponse(*responsePointer,  request->seq, true);
    std::string loadModulesFailedInfo;
    for (unsigned i = 0; i < reqPtr->arguments.pathList.size(); i++) {
        SBFileSpec symbolFileSpec(reqPtr->arguments.pathList[i].c_str());
        // check if symbol file is valid
        if (!symbolFileSpec.IsValid()) {
            loadModulesFailedInfo += "ERROR: Cannot find symbol file: " + reqPtr->arguments.pathList[i] + "\n";
            continue;
        }
        // check if UUID is valid
        std::optional<SBModule> sbModule = GetModuleByUUID(reqPtr->arguments.uuidList[i]);
        if (!sbModule.has_value() || !sbModule.value().IsValid()) {
            loadModulesFailedInfo += "ERROR: Cannot find module with UUID: " + reqPtr->arguments.uuidList[i] + "\n";
            continue;
        }
        /*
            lldb api SBTarget::AddModule does not support add symbol file, so we execute command:
            "target symbols add {symbol file path}" to load symbol file
        */
        std::string command = "target symbols add \"" + reqPtr->arguments.pathList[i] + "\"";
        std::string result;
        bool success;
        bool execStatus = ExecCommand(command, result, success);
        if (!execStatus || !success) {
            loadModulesFailedInfo += "ERROR: Cannot load symbol file for module: " +
                    std::string(sbModule.value().GetFileSpec().GetFilename());
            if (execStatus) {
                loadModulesFailedInfo += " reason: " + result + "\n";
            } else {
                loadModulesFailedInfo += " reason: command invalid!\n";
            }
            continue;
        }
        Module module = GetRefreshedModule(result);
        responsePointer->modules.emplace_back(module);
    }
    if (!loadModulesFailedInfo.empty()) {
        responsePointer->error = loadModulesFailedInfo;
    }
    responsePointer->totalModules = responsePointer->modules.size();
    return responsePointer;
}

bool LLDBApiDebugger::GetSourceMap(std::string& sourceMapStr, std::string& error)
{
    // We execute "settings show target.source-map" to view existing source map settings
    bool success;
    bool execStatus = ExecCommand(SHOW_SOURCE_MAP_COMMAND, sourceMapStr, success);
    if (!execStatus || !success) {
        error = "ERROR: LLDB cannot show target.source-map\n";
        if (execStatus) {
            error.append("reason: " + sourceMapStr);
        } else {
            error.append("reason: command invalid!");
        }
        return false;
    }
    return true;
}

std::unordered_map<string, string> LLDBApiDebugger::ParseSourceMap(const string& sourceMapStr)
{
    // We parse value of target.source-map
    vector<string> mapLines = CommonUtils::SplitString(sourceMapStr, "\n", false);
    std::unordered_map<string, string> sourceMap;
    for (const string& mapLine: mapLines) {
        if (mapLine.find('\"') == string::npos) {
            continue;
        }
        size_t srcDirStart = mapLine.find_first_of('\"') + 1;
        size_t srcDirEnd = mapLine.find_first_of('\"', srcDirStart);
        size_t destDirStart = mapLine.find_first_of('\"', srcDirEnd + 1) + 1;
        size_t destDirEnd = mapLine.find_first_of('\"', destDirStart);
        string key = mapLine.substr(srcDirStart, srcDirEnd - srcDirStart);
        string value = mapLine.substr(destDirStart, destDirEnd - destDirStart);
        if (!sourceMap.count(key)) {
            sourceMap[key] = value;
        }
    }
    return sourceMap;
}

bool LLDBApiDebugger::ExecuteSourceMapCommand(std::string& command, std::string& error)
{
    bool success;
    string result;
    bool execStatus = ExecCommand(command, result, success);
    if (!execStatus || !success) {
        error = "ERROR: Cannot do settings set target.source-mapping\n";
        if (execStatus) {
            error.append("reason: " + result);
        } else {
            error.append("reason: command invalid!");
        }
        return false;
    }
    return true;
}

lldb::SBFrame LLDBApiDebugger::GetLiveFrame(int64_t containerId)
{
    ContainerInfo info = GetContainerManager().GetContainerInfo(containerId);
    auto *framePtr = std::get_if<SBFrame>(&info.container);
    SBFrame stored = (framePtr != nullptr) ? *framePtr : SBFrame();
    if (GetOption().OhosCangjieDebug() && info.threadId != -1) {
        auto cjIndex = GetCjThreadIndexById(info.threadId);
        if (cjIndex.has_value()) {
            SBProcess process = sbTarget.GetProcess();
            SBCJThread cjThread = process.GetCJThreadAtIndex(cjIndex.value());
            addr_t pc = cjThread.GetFrameAtIndex(0).GetPC();
            SBFrame fresh = (pc == 0 || pc == LLDB_INVALID_ADDRESS)
                ? process.GetThreadByID(cjThread.GetHostThreadID()).GetFrameAtIndex(info.frameIndex)
                : cjThread.GetFrameAtIndex(info.frameIndex);
            if (fresh.IsValid()) {
                return fresh;
            }
        }
    }
    return stored;
}

UniqueResponsePtr LLDBApiDebugger::AddSourceMapping(Dap::UniqueRequestPtr request)
{
    AddSourceMapApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::ResetSourceMapping(Dap::UniqueRequestPtr request)
{
    ResetSourceMapApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::VariableHexView(Dap::UniqueRequestPtr request)
{
    VariableHexViewHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::CommandCompletion(Dap::UniqueRequestPtr request)
{
    CommandCompletionApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

Module LLDBApiDebugger::GetRefreshedModule(std::string addSymbolFileResult)
{
    std::string moduleFilePath = ExtractModuleFilePath(addSymbolFileResult);
    SBFileSpec fileSpec(moduleFilePath.c_str());
    SBModule sbModule = sbTarget.FindModule(fileSpec);
    Module module;
    GetModuleInfo(sbModule, module);
    return module;
}

UniqueResponsePtr LLDBApiDebugger::SetBreakpoint(UniqueRequestPtr request)
{
    SetBreakpointsApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetFunctionBreakpoint(UniqueRequestPtr request)
{
    SetFunctionBreakpointsApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetExceptionBreakpoint(UniqueRequestPtr request)
{
    SetExceptionBreakpointsApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::DataBreakpointInfo(UniqueRequestPtr request)
{
    DataBreakpointInfoApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetDataBreakpoints(UniqueRequestPtr request)
{
    SetDataBreakpointsApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetInstructionBreakpoints(UniqueRequestPtr request)
{
    SetInstructionBreakpointsApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

bool LLDBApiDebugger::ExecCommand(const string &command)
{
    string result;
    return ExecCommand(command, result);
}

bool LLDBApiDebugger::ExecCommand(const string &command, string &result)
{
    bool success = false;
    return ExecCommand(command, result, success);
}

bool LLDBApiDebugger::ExecCommand(const string &command, string &result, bool &success, bool isLog)
{
    success = false;
    result.clear();
    lldb::SBCommandReturnObject returnObject;
    if (isLog) {
        LOG_INFO("Send Command to Debugger : ", command);
    }
    if (!sbDebugger.IsValid()) {
        return false;
    }
    // Cangjie expression evaluation need c++14 category
    if (GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE) {
        sbDebugger.GetCategory(GetLanguageType()).SetEnabled(true);
    }
    SBCommandInterpreter sbCommandInterpreter = sbDebugger.GetCommandInterpreter();
    if (!sbCommandInterpreter.IsValid()) {
        return false;
    }
    sbCommandInterpreter.HandleCommand(command.c_str(), returnObject);
    if (!returnObject.IsValid()) {
        return false;
    }
    if (returnObject.GetOutput() != nullptr) {
        result = returnObject.GetOutput();
    }
    if (result.empty()) {
        SBStream sbStream;
        returnObject.GetDescription(sbStream);
        if (sbStream.GetData() != nullptr) {
            result = sbStream.GetData();
        }
        result = CommonUtils::TrimString(result, "Error:  ", TrimType::LEFT);
        result = CommonUtils::TrimString(result, "Success\nError Message:\n", TrimType::LEFT);
    }
    success = returnObject.Succeeded();
    string commandSuccess = success ? "Success" : "Failed";
    if (isLog) {
        LOG_INFO("Result of Command : ", commandSuccess, ", ", result);
    }
    return true;
}

UniqueResponsePtr LLDBApiDebugger::DebugInConsole(UniqueRequestPtr request)
{
    std::unique_ptr<DebugInConsoleResponse> responsePointer = std::make_unique<DebugInConsoleResponse>();
    InitResponse(*responsePointer, request->seq, false);

    auto reqPtr = dynamic_cast<DebugInConsoleRequest *>(request.get());
    if (reqPtr == nullptr) {
        responsePointer->message = "ERROR: DebugInConsoleRequest pointer conversion failed.";
        return responsePointer;
    }

    responsePointer->success = ExecCommand(reqPtr->arguments.debugCommand, responsePointer->output);
    return responsePointer;
}

UniqueResponsePtr LLDBApiDebugger::Threads(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ThreadsRequest*>(request.get());
    if (reqPtr == nullptr) {
        return CompleteFailureResponse(std::make_unique<ThreadsResponse>(),
            "ERROR: ThreadsRequest Pointer conversion failed.", request->seq);
    }
    SBProcess process = sbTarget.GetProcess();
    if (!process.IsValid()) {
        return CompleteSuccessResponse(std::make_unique<ThreadsResponse>(), request->seq);
    }
    auto responsePointer = std::make_unique<ThreadsResponse>();
    InitResponse(*responsePointer, request->seq, true);
    GetThreadMap().clear();
    if (GetOption().OhosCangjieDebug()) {
        FillCJThreads(process, *responsePointer);
    } else {
        FillNativeThreads(process, *responsePointer);
    }
    return responsePointer;
}

void LLDBApiDebugger::FillNativeThreads(SBProcess& process, ThreadsResponse& response)
{
    uint32_t count = process.GetNumThreads();
    for (uint32_t i = 0; i < count; ++i) {
        SBThread sbThread = process.GetThreadAtIndex(i);
        if (!sbThread.IsValid()) {
            continue;
        }
        Thread thread;
        thread.id  = sbThread.GetIndexID();
        thread.tid = sbThread.GetThreadID();

        const char* namePtr = sbThread.GetName();
        if (namePtr && namePtr[0] != '\0') {
            thread.name = namePtr;
        } else {
            thread.name = "Thread-" + std::to_string(thread.id);
        }
        response.threads.emplace_back(thread);
    }
}

void LLDBApiDebugger::FillCJThreads(SBProcess& process, ThreadsResponse& response)
{
    uint32_t count = process.GetNumCJThreads();
    for (uint32_t i = 0; i < count; ++i) {
        SBCJThread sbCjThread = process.GetCJThreadAtIndex(i);
        if (!sbCjThread.IsValid()) {
            continue;
        }
        Thread thread;
        thread.id  = sbCjThread.GetCJThreadID();
        thread.tid = sbCjThread.GetCJThreadID();
        thread.name = "CJThread-" + std::to_string(thread.id);
        const char* namePtr = sbCjThread.GetName();
        if (namePtr && namePtr[0] != '\0') {
            thread.name += "-[" + std::string(namePtr) + "]";
        }
        SetCjThreadIdAndIndex(sbCjThread.GetCJThreadID(), i);
        response.threads.emplace_back(thread);
    }
}

UniqueResponsePtr LLDBApiDebugger::ReadMemory(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ReadMemoryRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ReadMemoryRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ReadMemoryResponse>(), error, request->seq);
    }

    lldb::addr_t address;
    if (!CommonUtils::HexStringToUnsignedLongLong(reqPtr->arguments.memoryReference, address)) {
        string error = "ERROR: Invalid memory reference.";
        return CompleteFailureResponse(std::make_unique<ReadMemoryResponse>(), error, request->seq);
    }
    int64_t offset = reqPtr->arguments.offset.has_value() ? reqPtr->arguments.offset.value() : 0;
    uint64_t count = reqPtr->arguments.count;
    unique_ptr<char[]> buf = make_unique<char[]>(count);
    SBError sbError;
    size_t bytesRead = sbTarget.GetProcess().ReadMemory(address + offset, buf.get(), count, sbError);
    if (sbError.Fail()) {
        LOG_ERROR("failed to ReadMemory: ", sbError.GetCString());
        return CompleteFailureResponse(std::make_unique<ReadMemoryResponse>(), sbError.GetCString(), request->seq);
    } else {
        std::unique_ptr<ReadMemoryResponse> responsePointer = std::make_unique<ReadMemoryResponse>();
        InitResponse(*responsePointer,  request->seq, true);
        responsePointer->address = CommonUtils::Dec2HexAddress(address + offset,
                                                               CommonUtils::hexAddressWidth64BitsCpu);
        responsePointer->unreadableBytes = count - bytesRead;
        responsePointer->data = CommonUtils::Base64Encode(buf.get(), bytesRead);
        return responsePointer;
    }
}

UniqueResponsePtr LLDBApiDebugger::WriteMemory(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<WriteMemoryRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: WriteMemoryRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<WriteMemoryResponse>(), error, request->seq);
    }

    lldb::addr_t address;
    if (!CommonUtils::HexStringToUnsignedLongLong(reqPtr->arguments.memoryReference, address)) {
        string error = "ERROR: Invalid memory reference.";
        return CompleteFailureResponse(std::make_unique<WriteMemoryResponse>(), error, request->seq);
    }
    int64_t offset = reqPtr->arguments.offset.has_value() ? reqPtr->arguments.offset.value() : 0;

    string data = CommonUtils::Base64Decode(reqPtr->arguments.data);
    if (data.empty()) {
        string error = "ERROR: Invalid base64code data.";
        return CompleteFailureResponse(std::make_unique<WriteMemoryResponse>(), error, request->seq);
    }

    size_t count = data.length();
    SBError sbError;
    if (!reqPtr->arguments.allowPartial.has_value() || !reqPtr->arguments.allowPartial.value()) {
        unique_ptr<char[]> buf = make_unique<char[]>(count);
        size_t bytesRead = sbTarget.GetProcess().ReadMemory(address + offset, buf.get(), count, sbError);
        if (sbError.Fail() || bytesRead < count) {
            string error = "ERROR: Unable to write all data to memory";
            return CompleteFailureResponse(std::make_unique<WriteMemoryResponse>(), error, request->seq);
        }
    }

    size_t byteWritten = sbTarget.GetProcess().WriteMemory(address + offset, data.c_str(), count, sbError);
    if (sbError.Fail()) {
        LOG_ERROR("failed to WriteMemory: ", sbError.GetCString());
        return CompleteFailureResponse(std::make_unique<WriteMemoryResponse>(), sbError.GetCString(), request->seq);
    } else {
        std::unique_ptr<WriteMemoryResponse> responsePointer = std::make_unique<WriteMemoryResponse>();
        InitResponse(*responsePointer,  request->seq, true);
        responsePointer->bytesWritten = byteWritten;
        if (byteWritten < count) {
            LOG_WARN("Only part of data is written to the memory: the bytes of the input data is ", count,
                ", but the bytes of the actually written memory is ", byteWritten);
        }
        return responsePointer;
    }
}

UniqueResponsePtr LLDBApiDebugger::StackTrace(UniqueRequestPtr request)
{
    StackTraceApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Scopes(UniqueRequestPtr request)
{
    ScopesApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Variables(UniqueRequestPtr request)
{
    VariablesApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Evaluate(UniqueRequestPtr request)
{
    EvaluateApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetVariable(UniqueRequestPtr request)
{
    SetVariableApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Source(UniqueRequestPtr request)
{
    SourceApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::Disassemble(UniqueRequestPtr request)
{
    DisassembleApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::GetBitmap(Dap::UniqueRequestPtr request)
{
    GetBitmapApiHandler handler(shared_from_this());
    return handler.HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::SetExecutionPoint(UniqueRequestPtr request)
{
    std::unique_ptr<SetExecutionPointResponse> responsePointer = std::make_unique<SetExecutionPointResponse>();
    InitResponse(*responsePointer, request->seq, false);

    auto reqPtr = dynamic_cast<SetExecutionPointRequest *>(request.get());
    if (reqPtr == nullptr) {
        responsePointer->message = "ERROR: SetExecutionPointRequest pointer conversion failed.";
        return responsePointer;
    }

    string command;
    string result;
    if (reqPtr->arguments.instructionReference.has_value()) {
        if (!CommonUtils::IsValidHexInteger(reqPtr->arguments.instructionReference.value(),
                                            CommonUtils::hex64BitsStringLength)) {
            responsePointer->message = "ERROR: Invalid instruction reference.";
            return responsePointer;
        }
        command = "thread jump -r -a " + reqPtr->arguments.instructionReference.value();
    } else if (reqPtr->arguments.sourcePath.has_value() && reqPtr->arguments.line.has_value()) {
        command = "thread jump -r -f \"" +  reqPtr->arguments.sourcePath.value() + "\" -l " +
            std::to_string(reqPtr->arguments.line.value());
    } else {
        responsePointer->message = "ERROR: SetExecutionPointRequest has neither source nor instructionReference.";
        return responsePointer;
    }
    ExecCommand(command, result, responsePointer->success);
    responsePointer->message = result;
    return responsePointer;
}

UniqueResponsePtr LLDBApiDebugger::ReverseStep(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ReverseStepRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ReverseStepRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ReverseStepResponse>(), error, request->seq);
    }
    GetTimeTravelManager()->SelectRecordNode("reverse step");
    if (!GetTimeTravelManager()->IsAvailableOnCurrentContainer()) {
        string error = "History record point boundary reached";
        return CompleteFailureResponse(std::make_unique<ReverseStepResponse>(), error, request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<ReverseStepResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::Undo(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<UndoRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: UndoRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<UndoResponse>(), error, request->seq);
    }
    if (GetTimeTravelManager()->GetRecodeNodeQueueSize() <= 0) {
        return CompleteFailureResponse(std::make_unique<UndoResponse>(), "No record points saved", reqPtr->seq);
    }
    GetTimeTravelManager()->StateSwitchProcess(reqPtr->arguments.undoEnable);
    SendStoppedEventInReverseDebug(reqPtr->arguments.undoEnable);
    return CompleteSuccessResponse(std::make_unique<UndoResponse>(), reqPtr->seq);
}

UniqueResponsePtr LLDBApiDebugger::ReverseContinue(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ReverseContinueRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ReverseContinueRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ReverseContinueResponse>(), error, request->seq);
    }
    timeTravelManager->SelectRecordNode("reverse breakpoint");
    if (!GetTimeTravelManager()->IsAvailableOnCurrentContainer()) {
        string error = "History record point boundary reached";
        return CompleteFailureResponse(std::make_unique<ReverseContinueResponse>(), error, request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<ReverseContinueResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::OnDapRequestInUndo(UniqueRequestPtr request,
                                                      const string& command,
                                                      unsigned int requestSeq)
{
    UniqueResponsePtr response;
    if (!timeTravelManager->IsAllowedMessagesInUndo(command)) {
        string error = "unsupported in time travel debug mode";
        response = CompleteFailureResponse(std::make_unique<ErrorResponse>(command), error, requestSeq);
        LOG_INFO("This command is not supported in time travel debug mode, command = ", command);
    } else {
        response = requestHandlerMap[command](std::move(request));
    }
    return response;
}

UniqueResponsePtr LLDBApiDebugger::OnDapRequestInNormal(UniqueRequestPtr request,
                                                        const string& command,
                                                        unsigned int requestSeq)
{
    UniqueResponsePtr response;
    if (timeTravelManager->IsNotAllowedInNormal(command)) {
        string error = "unsupported in normal debug mode";
        response = CompleteFailureResponse(std::make_unique<ErrorResponse>(command), error, requestSeq);
        LOG_INFO("This command is not supported in normal debug mode, command = ", command);
    } else {
        response = requestHandlerMap[command](std::move(request));
    }
    return response;
}

UniqueResponsePtr LLDBApiDebugger::GotoRecordPoint(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<GotoRecordPointRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: GotoRecordPointRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<GotoRecordPointResponse>(), error, request->seq);
    }
    bool success = GetTimeTravelManager()->GotoRecordPoint(reqPtr->arguments.recordPointId);
    if (!success) {
        string error = "gotoRecordPoint failed, invalid recordPoint id: " + to_string(reqPtr->arguments.recordPointId);
        return CompleteFailureResponse(std::make_unique<GotoRecordPointResponse>(), error, reqPtr->seq);
    }
    return CompleteSuccessResponse(std::make_unique<GotoRecordPointResponse>(), reqPtr->seq);
}

UniqueResponsePtr LLDBApiDebugger::RecordPoints(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<RecordPointsRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: RecordPointsRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<RecordPointsResponse>(), error, request->seq);
    }
    GetTimeTravelManager()->SetFilters(reqPtr->arguments.filters);
    auto resp = std::make_unique<RecordPointsResponse>();
    uint64_t start = 0;
    uint64_t count = 0;
    if (reqPtr->arguments.start.has_value()) {
        start = reqPtr->arguments.start.value();
    }
    if (reqPtr->arguments.count.has_value()) {
        count = reqPtr->arguments.count.value();
    }
    resp->recordPoints = GetTimeTravelManager()->GetSupportedRecordPoints(start, count);
    return CompleteSuccessResponse(std::move(resp), reqPtr->seq);
}

UniqueResponsePtr LLDBApiDebugger::ContinueInReverse(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<ContinueInReverseRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: ContinueInReverseRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<ContinueInReverseResponse>(), error, request->seq);
    }
    timeTravelManager->SelectRecordNode("breakpoint");
    if (!GetTimeTravelManager()->IsAvailableOnCurrentContainer()) {
        string error = "History record point boundary reached";
        return CompleteFailureResponse(std::make_unique<ContinueInReverseResponse>(), error, request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<ContinueInReverseResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::StepInReverse(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<StepInReverseRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: StepInReverseRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<StepInReverseResponse>(), error, request->seq);
    }
    timeTravelManager->SelectRecordNode("step");
    if (!GetTimeTravelManager()->IsAvailableOnCurrentContainer()) {
        string error = "History record point boundary reached";
        return CompleteFailureResponse(std::make_unique<StepInReverseResponse>(), error, request->seq);
    }
    return CompleteSuccessResponse(std::make_unique<StepInReverseResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::CacheDataConfig(UniqueRequestPtr request)
{
    auto response = std::make_unique<CacheDataConfigResponse>();
    auto reqPtr = dynamic_cast<CacheDataConfigRequest *>(request.get());
    if (reqPtr == nullptr) {
        response->message = "ERROR: CacheDataConfigRequest Pointer conversion failed.";
        return response;
    }
    CacheDataConfigArguments arguments = reqPtr->arguments;
    Dap::CacheDataConfig cacheDataConfig;
    cacheDataConfig.startCacheData = arguments.startCacheData;
    cacheDataConfig.saveHistoryCache = arguments.saveHistoryCache;
    if (arguments.threadCount.has_value()) {
        cacheDataConfig.threadCount = arguments.threadCount.value();
    }
    if (arguments.stackFrameCount.has_value()) {
        cacheDataConfig.stackFrameCount = arguments.stackFrameCount.value();
    }
    if (arguments.variableChildrenLayers.has_value()) {
        cacheDataConfig.variableChildrenLayers = arguments.variableChildrenLayers.value();
    }
    if (arguments.variableChildrenCount.has_value()) {
        cacheDataConfig.variableChildrenCount = arguments.variableChildrenCount.value();
        if (cacheDataConfig.variableChildrenCount == 0) {
            cacheDataConfig.variableChildrenLayers = 0;
        }
    }
    if (arguments.stackTracePageSize.has_value()) {
        cacheDataConfig.stackTracePageSize = arguments.stackTracePageSize.value();
    }
    if (arguments.variablePageSize.has_value()) {
        cacheDataConfig.variablePageSize = arguments.variablePageSize.value();
    }
    if (arguments.scopeType.has_value()) {
        cacheDataConfig.scopeType = arguments.scopeType.value();
    }
    debugCacheHelper->UpdateCacheDataConfig(cacheDataConfig);
    return CompleteSuccessResponse(std::make_unique<CacheDataConfigResponse>(), request->seq);
}

UniqueResponsePtr LLDBApiDebugger::RecordMessage(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<RecordMessageRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: RecordMessageRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<RecordMessageResponse>(), error, request->seq);
    }
    std::unique_ptr<RecordMessageResponse> responsePointer = std::make_unique<RecordMessageResponse>();
    for (const auto &id : reqPtr->arguments.recordPointIds) {
        Dap::Protocol::RecordMessage recordMessage;
        recordMessage.recordPointId = id;
        auto recordNode = GetTimeTravelManager()->GetRecordNodeById(id);
        if (recordNode == nullptr) {
            continue;
        }
        if (reqPtr->arguments.requireThread.has_value() && reqPtr->arguments.requireThread.value()) {
            recordMessage.thread = recordNode->GetStoppedThread();
        }
        if (reqPtr->arguments.requireStackFrame.has_value() && reqPtr->arguments.requireStackFrame.value()) {
            recordMessage.stackFrame = recordNode->GetTopFrame();
            if (recordMessage.stackFrame->id == -1) {
                recordMessage.stackFrame = std::nullopt;
            }
        }
        responsePointer->recordMessages.push_back(recordMessage);
    }
    if (responsePointer->recordMessages.empty()) {
        string error = "ERROR: failed to get any message.";
        return CompleteFailureResponse(std::move(responsePointer), error, request->seq);
    }
    return CompleteSuccessResponse(std::move(responsePointer), reqPtr->seq);
}

void LLDBApiDebugger::SetMaxStringSummaryLength(const std::string &maxLength)
{
    lldb::SBDebugger::SetInternalVariable("target.max-string-summary-length", maxLength.c_str(),
                                          sbDebugger.GetInstanceName());
}

std::string LLDBApiDebugger::GetMaxStringSummaryLength()
{
    string length;
    auto sbStringList = lldb::SBDebugger::GetInternalVariableValue("target.max-string-summary-length",
                                                                   sbDebugger.GetInstanceName());
    if (sbStringList.IsValid() && sbStringList.GetSize() > 0) {
        length = sbStringList.GetStringAtIndex(0);
    }
    return length;
}

lldb::SBValue LLDBApiDebugger::FindExprValue(lldb::SBFrame &frame, const std::string &expression)
{
    lldb::SBValue value = frame.FindValue(expression.c_str(), lldb::eValueTypeVariableLocal);
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindValue(expression.c_str(), lldb::eValueTypeVariableArgument);
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindValue(expression.c_str(), lldb::eValueTypeVariableGlobal);
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindValue(expression.c_str(), lldb::eValueTypeVariableStatic);
    if (value.IsValid()) {
        return value;
    }
    SBValueList valueList = frame.GetVariables(false, false, true, true);
    value = valueList.GetFirstValueByName(expression.c_str());
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindValue(expression.c_str(), lldb::eValueTypeConstResult);
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindValue(expression.c_str(), lldb::eValueTypeVariableThreadLocal);
    if (value.IsValid()) {
        return value;
    }
    value = frame.FindVariable(expression.c_str());
    if (value.IsValid()) {
        return value;
    }
#ifdef _DARWIN
    if (GetDebuggeeLanguageType() != DebuggeeLanguageType::CANGJIE) {
        value = frame.EvaluateExpression(expression.c_str());
        if (value.IsValid()) {
            return value;
        }
    }
#else
    value = frame.EvaluateExpression(expression.c_str());
    if (value.IsValid()) {
        return value;
    }
#endif
    return value;
}

bool LLDBApiDebugger::JustExpr(lldb::SBFrame &frame, const std::string &expression, bool isUpdateCategory)
{
    if (isUpdateCategory) {
        UpdateCategory(true);
    }
    SBValue value = frame.EvaluateExpression(expression.c_str());
    if (isUpdateCategory) {
        UpdateCategory();
    }
    return value.IsValid();
}

std::string LLDBApiDebugger::GetExprValueResult(lldb::SBFrame &frame, const std::string &expression)
{
    SBValue value = frame.EvaluateExpression(expression.c_str());
    if (value.IsValid()) {
        ValueParser valueParser(value, shared_from_this());
        std::string result;
        if (valueParser.GetValue(result)) {
            return result;
        }
    }
    return "";
}

std::string LLDBApiDebugger::GetMemoryData(const std::string &hexAddr, uint64_t count, int64_t offset)
{
    lldb::addr_t addr;
    if (!CommonUtils::HexStringToUnsignedLongLong(hexAddr, addr)) {
        return "";
    }
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(count);
    lldb::SBError sbError;
    size_t bytesRead = sbTarget.GetProcess().ReadMemory(addr + offset, buf.get(), count, sbError);
    if (sbError.Fail()) {
        LOG_ERROR("Read memory addr[", hexAddr, "] count[", count, "] offset[", offset, "] failed.");
        return "";
    }
    return CommonUtils::Base64Encode(buf.get(), bytesRead);
}

UniqueResponsePtr LLDBApiDebugger::BreakpointLocations(UniqueRequestPtr request)
{
    // handler will be used in another thread;
    auto handler = std::make_shared<BreakpointLocationsApiHandler>(shared_from_this());
    return handler->HandleRequest(std::move(request));
}

UniqueResponsePtr LLDBApiDebugger::AddSourceBreakpoint(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<AddSourceBreakpointRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: AddSourceBreakpointRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<AddSourceBreakpointResponse>(), error, request->seq);
    }
    if (!reqPtr->arguments.source.path.has_value()) {
        string error = "ERROR: No file path entered.";
        return CompleteFailureResponse(std::make_unique<AddSourceBreakpointResponse>(), error, request->seq);
    }
    string filePath = reqPtr->arguments.source.path.value();
    int l = option.linesStartAt1 ? 0 : 1;
    int c = option.columnsStartAt1 ? 0 : 1;
    int64_t line = reqPtr->arguments.line + l;
    int64_t column = -1;
    if (reqPtr->arguments.column.has_value()) {
        column = reqPtr->arguments.column.value() + c;
    }
    SBBreakpoint sbBreakpoint;
    if (column < 0) {
        sbBreakpoint = sbTarget.BreakpointCreateByLocation(filePath.c_str(), line);
    } else {
        SBFileSpec sbFileSpec(filePath.c_str());
        SBFileSpecList sbFileSpecList;
        sbBreakpoint = sbTarget.BreakpointCreateByLocation(sbFileSpec, line, column, 0, sbFileSpecList);
    }
    Breakpoint breakpoint;
    breakpoint.id = sbBreakpoint.GetID();
    breakpoint.source = Dap::Protocol::Source();
    breakpoint.source->path = filePath;
    breakpoint.verified = sbBreakpoint.IsValid();
    breakpoint.line = sbBreakpoint.GetLocationAtIndex(0).GetAddress().GetLineEntry().GetLine() - l;
    breakpoint.column = sbBreakpoint.GetLocationAtIndex(0).GetAddress().GetLineEntry().GetColumn() - c;
    auto resp = std::make_unique<AddSourceBreakpointResponse>();
    resp->breakpoint = std::move(breakpoint);
    return CompleteSuccessResponse(std::move(resp), reqPtr->seq);
}

UniqueResponsePtr LLDBApiDebugger::DeleteBreakpoint(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<DeleteBreakpointRequest *>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: DeleteBreakpointRequest Pointer conversion failed.";
        return CompleteFailureResponse(std::make_unique<DeleteBreakpointResponse>(), error, request->seq);
    }
    sbTarget.BreakpointDelete(reqPtr->arguments.id);
    return CompleteSuccessResponse(std::make_unique<DeleteBreakpointResponse>(), reqPtr->seq);
}
