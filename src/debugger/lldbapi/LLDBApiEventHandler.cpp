// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LLDBApiEventHandler.h"
#include <thread>
#include <climits>
#include "DapLog.h"
#include "LLDB.h"
#include "CommonUtils.h"
#include "TimeTravelDebugManager.h"
#include "EnumUtils.h"

using namespace lldb;
using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

bool LLDBApiEventHandler::StartListener()
{
    if (isListening) {
        errorMsg = "Event listener is already running now.";
        LOG_ERROR(errorMsg);
        return false;
    }
    if (apiDebugger == nullptr) {
        errorMsg = "LLDBApiDebugger isn't initialized!";
        LOG_ERROR(errorMsg);
        return false;
    }
    if (!apiDebugger->GetSbDebugger().GetListener().IsValid()) {
        errorMsg = "SBListener of debugger is invalid!";
        LOG_ERROR(errorMsg);
        return false;
    }
    isListening = true;
    listenEventThread = std::thread([this] { ListenEvent(); });
    return true;
}

LLDBApiEventHandler::~LLDBApiEventHandler()
{
    StopListener();
}

bool LLDBApiEventHandler::InitSBListener(lldb::SBListener &listener)
{
    if (!listener.IsValid()) {
        LOG_INFO("SBListener is invalid !");
        return false;
    }
    unsigned int eventMask = SBTarget::eBroadcastBitBreakpointChanged | SBTarget::eBroadcastBitWatchpointChanged |
                             SBTarget::eBroadcastBitModulesLoaded | SBTarget::eBroadcastBitModulesUnloaded |
                             SBTarget::eBroadcastBitSymbolsLoaded;
    if (lldb::SBTarget::GetBroadcasterClassName() == nullptr) {
        LOG_INFO("unable to get BroadcasterClassName!");
        return false;
    }
    bool result = listener.StartListeningForEventClass(
        apiDebugger->GetSbDebugger(), lldb::SBTarget::GetBroadcasterClassName(), eventMask);
    return result;
}

void LLDBApiEventHandler::ListenEvent()
{
    LOG_INFO("Event listener started.");
    SBListener sbListener = apiDebugger->GetSbDebugger().GetListener();
    if (!InitSBListener(sbListener)) {
        return;
    }
    lldb::SBEvent sbEvent = lldb::SBEvent();
    lldb::SBStream s;
    while (sbListener.IsValid() && isListening) {
        sbEvent.Clear();
        s.Clear();
        if (sbListener.WaitForEvent(1, sbEvent) && sbEvent.IsValid()) {
            sbEvent.GetDescription(s);
            if (s.IsValid() && s.GetData() != nullptr) {
                LOG_INFO("Description of event: ", s.GetData());
            }
            if (lldb::SBProcess::EventIsProcessEvent(sbEvent)) {
                HandleProcessEvent(sbEvent);
            } else if (lldb::SBBreakpoint::EventIsBreakpointEvent(sbEvent)) {
                HandleBreakpointEvent(sbEvent);
            } else if (lldb::SBWatchpoint::EventIsWatchpointEvent(sbEvent)) {
                HandleWatchpointEvent(sbEvent);
            } else if (lldb::SBThread::EventIsThreadEvent(sbEvent)) {
                // ThreadEvent
            } else if (lldb::SBTarget::EventIsTargetEvent(sbEvent)) {
                HandleTargetEvent(sbEvent);
            } else if (lldb::SBCommandInterpreter::EventIsCommandInterpreterEvent(sbEvent)) {
                // InterpreterEvent
            }
        }
    }
    LOG_INFO("Event listener exited.");
}

void LLDBApiEventHandler::StopListener()
{
    if (!isListening) {
        LOG_INFO("Event listener is not running now.");
    }
    isListening = false;
    if (listenEventThread.joinable() && listenEventThread.get_id() != std::this_thread::get_id()) {
        listenEventThread.join();
    }
}

void LLDBApiEventHandler::HandleProcessEvent(const lldb::SBEvent &sbEvent)
{
    LOG_INFO("A process event occurred!");
    string processEventType;
    switch (sbEvent.GetType()) {
        case lldb::SBProcess::eBroadcastBitStateChanged:
            processEventType = "eBroadcastBitStateChanged";
            HandleProcessStateChangedEvent(sbEvent);
            break;
        case lldb::SBProcess::eBroadcastBitInterrupt:
            processEventType = "eBroadcastBitInterrupt";
            break;
        case lldb::SBProcess::eBroadcastBitProfileData:
            processEventType = "eBroadcastBitProfileData";
            break;
        case lldb::SBProcess::eBroadcastBitStructuredData:
            processEventType = "eBroadcastBitStructuredData";
            break;
        case lldb::SBProcess::eBroadcastBitSTDERR:
            // stderr message
            processEventType = "eBroadcastBitSTDERR";
            break;
        case lldb::SBProcess::eBroadcastBitSTDOUT:
            // stdout message
            processEventType = "eBroadcastBitSTDOUT";
            break;
        default: {
            processEventType = "UnKnownProcessEvent";
        }
    }
    LOG_INFO("Process event handle finished, event type is ", processEventType);
}

void LLDBApiEventHandler::HandleProcessStateChangedEvent(const lldb::SBEvent &sbEvent)
{
    LOG_INFO("Process state changed!");
    const lldb::StateType stateType = lldb::SBProcess::GetStateFromEvent(sbEvent);
    string processState;
    switch (stateType) {
        case lldb::eStateStopped:
            processState = "eStateStopped";
            HandleStoppedEvent(sbEvent);
            break;
        case lldb::eStateRunning:
            processState = "eStateRunning";
            HandleRunningEvent();
            break;
        case lldb::eStateExited:
            processState = "eStateExited";
            HandleExitedEvent();
            break;
        case lldb::eStateInvalid:
            processState = "eStateInvalid";
            break;
        case lldb::eStateUnloaded:
            processState = "eStateUnloaded";
            break;
        case lldb::eStateConnected:
            processState = "eStateConnected";
            break;
        case lldb::eStateAttaching:
            processState = "eStateAttaching";
            break;
        case lldb::eStateLaunching:
            processState = "eStateLaunching";
            break;
        case lldb::eStateCrashed:
            processState = "eStateCrashed";
            break;
        case lldb::eStateSuspended:
            processState = "eStateSuspended";
            break;
        case lldb::eStateStepping:
            processState = "eStateStepping";
            break;
        case lldb::eStateDetached:
            processState = "eStateDetached";
            break;
        default:
            processState = "Unknown";
    }
    LOG_INFO("Process state changed event handler finished, process state changed to", processState);
}

void LLDBApiEventHandler::HandleStoppedEvent(const lldb::SBEvent &sbEvent)
{
    lldb::SBProcess process = apiDebugger->GetSbTarget().GetProcess();
    if (!process.IsValid()) {
        LOG_INFO("Process is invalid!");
        return;
    }
    lldb::StateType state = process.GetState();
    if (state != lldb::eStateStopped) {
        LOG_INFO("Process state is ", lldb::SBDebugger::StateAsCString(state), " now.");
        return;
    }
    if (lldb::SBProcess::GetRestartedFromEvent(sbEvent)) {
        LOG_INFO("Process has been restarted.");
        return;
    }
    apiDebugger->GetContainerManager().Reset();
    apiDebugger->SetDebuggeeState(DebuggeeState::STOPPED);
    currentStoppedEvent = {};
    currentStoppedEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    currentStoppedEvent.body.allThreadsStopped = true;
    UpdateSelectThread();
    lldb::SBThread thread = process.GetSelectedThread();
    if (!thread.IsValid()) {
        currentStoppedEvent.body.threadId = -1;
        currentStoppedEvent.body.reason = StoppedReason::UNKNOWN;
        apiDebugger->SendEvent(currentStoppedEvent);
        return;
    }
    GetStoppedReason(sbEvent, thread);
    apiDebugger->SetPauseOperation(false);
    SBThread preStoppedThread;
    int threadId;
    if (apiDebugger->GetOption().OhosCangjieDebug()) {
        SBCJThread cjthread = apiDebugger->GetSbTarget().GetProcess().GetSelectedCJThread();
        preStoppedThread = apiDebugger->GetSbTarget().GetProcess().GetThreadByID(cjthread.GetHostThreadID());
        threadId = cjthread.GetCJThreadID();
    } else {
        preStoppedThread = apiDebugger->GetSbTarget().GetProcess().GetThreadByIndexID(preStoppedThreadIndex);
        threadId = thread.GetIndexID();
    }
    if (thread.GetStopReason() == lldb::eStopReasonSignal && preStoppedThread.IsValid()) {
        // when received signal, lldb will catch the thread at index 1, e.g. interrupt.
        // we try parse the previous stopped thread in this case.
        currentStoppedEvent.body.threadId = preStoppedThreadIndex;
    } else {
        currentStoppedEvent.body.threadId = threadId;
        preStoppedThreadIndex = threadId;
    }
    if (apiDebugger->GetEntryBreakpointID() != -1) {
        apiDebugger->GetSbTarget().BreakpointDelete(apiDebugger->GetEntryBreakpointID());
        apiDebugger->SetEntryBreakpointID(-1);
    }
    TimeTravelPreProcess();
}

void LLDBApiEventHandler::GetStoppedReason(const SBEvent &sbEvent, SBThread &thread)
{
    switch (thread.GetStopReason()) {
        case eStopReasonBreakpoint: {
            int bkptno = thread.GetStopReasonDataAtIndex(0);
            vector<int64_t> hitBreakpointIds = { bkptno };
            currentStoppedEvent.body.hitBreakpointIds = hitBreakpointIds;
            currentStoppedEvent.body.reason = apiDebugger->GetSpecificBreakpointStopReason(bkptno);
            break;
        }
        case eStopReasonWatchpoint: {
            int bkptno = thread.GetStopReasonDataAtIndex(0);
            vector<int64_t> hitBreakpointIds = { bkptno };
            currentStoppedEvent.body.hitBreakpointIds = hitBreakpointIds;
            currentStoppedEvent.body.reason = StoppedReason::DATA_BREAKPOINT;
            break;
        }
        case eStopReasonTrace:
        case eStopReasonPlanComplete:
            currentStoppedEvent.body.reason = StoppedReason::STEP;
            break;
        case eStopReasonSignal:
            if (apiDebugger->IsPauseOperation()) {
                currentStoppedEvent.body.reason = StoppedReason::PAUSE;
                break;
            }
            currentStoppedEvent.body.reason = StoppedReason::EXCEPTION;
            currentStoppedEvent.body.description = GetSignalDescription(sbEvent);
            break;
        case eStopReasonException:
            if (apiDebugger->IsPauseOperation()) {
                currentStoppedEvent.body.reason = StoppedReason::PAUSE;
                break;
            }
            currentStoppedEvent.body.reason = StoppedReason::EXCEPTION;
            currentStoppedEvent.body.description = GetExceptionDescription();
            break;
        default:
            currentStoppedEvent.body.reason = StoppedReason::UNKNOWN;
    }
}

void LLDBApiEventHandler::TimeTravelPreProcess()
{
    int bkptno = apiDebugger->GetSbTarget().GetProcess().GetSelectedThread().GetStopReasonDataAtIndex(0);
    bool isLogPoint = currentStoppedEvent.body.reason == StoppedReason::BREAKPOINT &&
        apiDebugger->HasBreakPointLogMsg(bkptno);
    if (apiDebugger->GetTimeTravelManager() != nullptr) {
        if (apiDebugger->GetDebugCacheHelper()->CacheInServer() &&
            apiDebugger->GetTimeTravelManager()->IsSupportedRecordPointType(currentStoppedEvent.body.reason)) {
            int coroutineId = GetRunningCoroutineId();
            apiDebugger->GetTimeTravelManager()->CreateRecordNode(currentStoppedEvent.body, coroutineId);
            apiDebugger->GetDebugCacheHelper()->StartCaching(currentStoppedEvent);
        }
        if (!isLogPoint) {
            auto body = apiDebugger->GetTimeTravelManager()->GetRecordPointsUpdateEventBody();
            if (body.newRecordPoints.has_value() || body.removedRecordPointIds.has_value()) {
                RecordPointsUpdateEvent event;
                event.body = body;
                event.seq = apiDebugger->GetAndIncCurrentMessageSeq();
                apiDebugger->SendEvent(event);
            }
        }
    }
    if (isLogPoint) {
        HandleLogPoint(bkptno);
    } else {
        apiDebugger->SendEvent(currentStoppedEvent);
    }
}

void LLDBApiEventHandler::HandleLogPoint(const int breakpointId)
{
    OutputEvent outputEvent;
    outputEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    outputEvent.body.category = OutputEventBodyCategory::STDOUT;
    outputEvent.body.output = apiDebugger->GetBreakpointLogMsg(breakpointId);
    outputEvent.body.output += "\n";
    apiDebugger->SendEvent(outputEvent);
    apiDebugger->GetSbTarget().GetProcess().Continue();
}

void LLDBApiEventHandler::HandleRunningEvent() const
{
    apiDebugger->SetDebuggeeState(DebuggeeState::RUNNING);
    ContinuedEvent continuedEvent;
    continuedEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    continuedEvent.body.allThreadsContinued = true;
    continuedEvent.body.threadId = 0;
    apiDebugger->SendEvent(continuedEvent);
}


void LLDBApiEventHandler::HandleExitedEvent()
{
    apiDebugger->SetDebuggeeState(DebuggeeState::EXIT);

    ExitedEvent exitedEvent;
    exitedEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    exitedEvent.body.exitCode = apiDebugger->GetSbTarget().GetProcess().GetExitStatus();
    apiDebugger->SendEvent(exitedEvent);
    SendTerminatedEvent(false);
    this->StopListener();
}

void LLDBApiEventHandler::SendTerminatedEvent(const bool &restart)
{
    TerminatedEvent terminatedEvent;
    TerminatedEventBody terminatedEventBody;
    terminatedEventBody.restart = restart;
    terminatedEvent.body = terminatedEventBody;
    terminatedEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    apiDebugger->SendEvent(terminatedEvent);
}

void LLDBApiEventHandler::UpdateSelectThread() const
{
    // find which thread stopped and switch to stopped thread
    SBProcess process = apiDebugger->GetSbTarget().GetProcess();
    lldb::SBThread thread;
    lldb::SBThread planThread;
    lldb::SBThread breakpointThread;
    lldb::SBThread exceptionThread;
    const size_t threadsNum = process.GetNumThreads();
    for (unsigned int i = 0; i < threadsNum; i++) {
        thread = process.GetThreadAtIndex(i);
        const lldb::StopReason stopReason = thread.GetStopReason();
        if ((stopReason == lldb::eStopReasonBreakpoint || stopReason == lldb::eStopReasonWatchpoint) &&
            (!breakpointThread.IsValid() || (int)i == preStoppedThreadIndex)) {
            breakpointThread = thread;
        }
        if ((stopReason == lldb::eStopReasonSignal || stopReason == lldb::eStopReasonException) &&
            (!exceptionThread.IsValid() || (int)i == preStoppedThreadIndex)) {
            exceptionThread = thread;
        }
        if (stopReason == lldb::eStopReasonTrace || (stopReason == lldb::eStopReasonPlanComplete &&
            (!planThread.IsValid() || (int)i == preStoppedThreadIndex))) {
            planThread = thread;
        }
    }
    SBThread preStoppedThread = process.GetThreadByIndexID(preStoppedThreadIndex);
    lldb::SBThread currentThread = process.GetSelectedThread();
    if (planThread.IsValid()) {
        process.SetSelectedThread(planThread);
    } else if (breakpointThread.IsValid()) {
        process.SetSelectedThread(breakpointThread);
    } else if (exceptionThread.IsValid()) {
        process.SetSelectedThread(exceptionThread);
    } else if (preStoppedThread.IsValid()) {
        process.SetSelectedThread(preStoppedThread);
    } else if (currentThread.IsValid()) {
        process.SetSelectedThread(currentThread);
    } else if (process.GetThreadAtIndex(0).IsValid()) {
        process.SetSelectedThread(process.GetThreadAtIndex(0));
    }
}

string LLDBApiEventHandler::GetExceptionDescription() const
{
    lldb::SBThread thread = apiDebugger->GetSbTarget().GetProcess().GetSelectedThread();
    if (!thread.IsValid()) {
        return "Unknown";
    }
    const size_t descriptionLen = thread.GetStopDescription(nullptr, 0);
    char stopDescription[descriptionLen];
    thread.GetStopDescription(stopDescription, descriptionLen);
    return stopDescription;
}

std::string LLDBApiEventHandler::GetSignalDescription(const lldb::SBEvent &sbEvent) const
{
    string description;
    lldb::SBProcess process = apiDebugger->GetSbTarget().GetProcess();
    const unsigned int signal = process.GetSelectedThread().GetStopReasonDataAtIndex(0);
    lldb::SBUnixSignals unixSignals = process.GetUnixSignals();
    const unsigned int sigint = unixSignals.GetSignalNumberFromName("SIGINT");
    const unsigned int sigstop = unixSignals.GetSignalNumberFromName("SIGSTOP");
    const unsigned int sigsegv = unixSignals.GetSignalNumberFromName("SIGSEGV");
    const unsigned int sigtrap = unixSignals.GetSignalNumberFromName("SIGTRAP");
    const bool interrupted = lldb::SBProcess::GetInterruptedFromEvent(sbEvent);
    if (signal == sigint || (signal == sigstop && interrupted)) {
        description = "SIGINT: Interrupt.";
    } else if (signal == sigstop) {
        description = "SIGSTOP: Stop.";
    } else if (signal == sigsegv) {
        description = "SIGSEGV: Segmentation fault.";
    } else if (signal == sigtrap) {
        lldb::SBThread thread = process.GetSelectedThread();
        if (thread.GetNumFrames() > 0) {
            lldb::SBFrame frame = thread.GetFrameAtIndex(0);
            const char* pFnName = frame.GetFunctionName();
            if (pFnName != nullptr && string(pFnName) == "__pthread_clone" && process.IsValid()) {
                // ignore stopped event from debugger signal
                process.Continue();
            }
        }
        description = "SIGTRAP";
    } else {
        unixSignals = process.GetUnixSignals();
        const char* pSignal = unixSignals.GetSignalAsCString(signal);
        if (pSignal) {
            description = pSignal;
        } else {
            description = std::to_string(signal);
        }
    }
    LOG_INFO("signal-received, ", description);
    return description;
}

void LLDBApiEventHandler::HandleTargetEvent(const SBEvent &sbEvent)
{
    LOG_INFO("A target event occurred!");
    string targetEventType;
    switch (sbEvent.GetType()) {
        case SBTarget::eBroadcastBitModulesLoaded:
            targetEventType = "eBroadcastBitModulesLoaded";
            HandleModulesLoadedEvent(sbEvent);
            break;
        case SBTarget::eBroadcastBitModulesUnloaded:
            targetEventType = "eBroadcastBitModulesUnloaded";
            break;
        case SBTarget::eBroadcastBitSymbolsLoaded:
            targetEventType = "eBroadcastBitSymbolsLoaded";
            break;
        case SBTarget::eBroadcastBitBreakpointChanged:
            targetEventType = "eBroadcastBitBreakpointChanged";
            break;
        case SBTarget::eBroadcastBitWatchpointChanged:
            targetEventType = "eBroadcastBitWatchpointChanged";
            break;
        default: {
            targetEventType = "UnKnownTargetEvent";
        }
    }
    LOG_INFO("Target event handle finished, event type is ", targetEventType);
}

void LLDBApiEventHandler::HandleModulesLoadedEvent(const SBEvent &sbEvent)
{
    const unsigned number = SBTarget::GetNumModulesFromEvent(sbEvent);
    OutputEvent outputEvent;
    outputEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    outputEvent.body.category = OutputEventBodyCategory::CONSOLE;
    // send modulesEvent to client
    ModulesEvent modulesEvent;
    modulesEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    std::vector<Module> moduleInfoList;
    for (unsigned i = 0; i < number; ++i) {
        SBModule sbModule = SBTarget::GetModuleAtIndexFromEvent(i, sbEvent);
        Module module;
        apiDebugger->GetModuleInfo(sbModule, module);
        std::string strModuleInfo = GetModuleInfoStr(module);
        outputEvent.body.output += module.dateTimeStamp.value();
        outputEvent.body.output += ": library-loaded,";
        outputEvent.body.output += strModuleInfo;
        outputEvent.body.output += "\n";
        moduleInfoList.emplace_back(module);
    }
    modulesEvent.body.moduleList = moduleInfoList;
    apiDebugger->SendEvent(outputEvent);
    apiDebugger->SendEvent(modulesEvent);
}

std::string LLDBApiEventHandler::GetModuleInfoStr(Module &module)
{
    // "target-name" field
    std::string strModuleInfo;
    strModuleInfo += "target-name=";
    strModuleInfo += "\"" + module.path.value() + "\"";
    strModuleInfo += ",";
    // "host-name" field
    strModuleInfo += "host-name=";
    strModuleInfo += "\"" + module.platformFilePath.value() + "\"";
    strModuleInfo += ",";
    // "loaded_addr" field
    strModuleInfo += "loaded_addr=";
    strModuleInfo += "\"" + module.loadedAddress.value() + "\"";
    strModuleInfo += ",";
    // "codeRange" field
    strModuleInfo += "codeRange=";
    strModuleInfo += "\"" + module.codeRange.value() + "\"";
    strModuleInfo += ",";
    // "moduleAddressRange" field
    strModuleInfo += "moduleAddressRange=";
    strModuleInfo += "\"" + module.addressRange.value() + "\"";
    return strModuleInfo;
}

void LLDBApiEventHandler::HandleBreakpointEvent(const lldb::SBEvent &sbEvent)
{
    LOG_INFO("A breakpoint event occurred!");
    string bpEventType;
    const BreakpointEventType breakpointEventType = SBBreakpoint::GetBreakpointEventTypeFromEvent(sbEvent);
    SBBreakpoint sbBreakpoint = SBBreakpoint::GetBreakpointFromEvent(sbEvent);
    switch (breakpointEventType) {
        case lldb::eBreakpointEventTypeAdded:
            bpEventType = "eBreakpointEventTypeAdded";
            HandleBreakpointAddedEvent(sbBreakpoint, false);
            break;
        case lldb::eBreakpointEventTypeLocationsResolved:
            bpEventType = "eBreakpointEventTypeLocationsResolved";
            break;
        case lldb::eBreakpointEventTypeEnabled:
            bpEventType = "eBreakpointEventTypeEnabled";
            break;
        case lldb::eBreakpointEventTypeDisabled:
            bpEventType = "eBreakpointEventTypeDisabled";
            break;
        case lldb::eBreakpointEventTypeCommandChanged:
            bpEventType = "eBreakpointEventTypeCommandChanged";
            break;
        case lldb::eBreakpointEventTypeConditionChanged:
            bpEventType = "eBreakpointEventTypeConditionChanged";
            break;
        case lldb::eBreakpointEventTypeIgnoreChanged:
            bpEventType = "eBreakpointEventTypeIgnoreChanged";
            break;
        case lldb::eBreakpointEventTypeAutoContinueChanged:
            bpEventType = "eBreakpointEventTypeAutoContinueChanged";
            break;
        case lldb::eBreakpointEventTypeLocationsAdded:
            bpEventType = "eBreakpointEventTypeLocationsAdded";
            break;
        case lldb::eBreakpointEventTypeRemoved:
            bpEventType = "eBreakpointEventTypeRemoved";
            HandleBreakpointRemoveEvent(sbBreakpoint, false);
            break;
        case lldb::eBreakpointEventTypeThreadChanged:
            bpEventType = "eBreakpointEventTypeThreadChanged";
            break;
        case lldb::eBreakpointEventTypeLocationsRemoved:
            bpEventType = "eBreakpointEventTypeLocationsRemoved";
            break;
        case lldb::eBreakpointEventTypeInvalidType:
            bpEventType = "eBreakpointEventTypeInvalidType";
            break;
        default: {
            bpEventType = "UnKnownBreakpointEvent";
        }
    }
    LOG_INFO("Breakpoint event handle finished, event type is ", bpEventType);
}

void LLDBApiEventHandler::HandleWatchpointEvent(const lldb::SBEvent &sbEvent)
{
    LOG_INFO("A Watchpoint event occurred!");
    string wpEventType;
    const WatchpointEventType watchpointEventType = SBWatchpoint::GetWatchpointEventTypeFromEvent(sbEvent);
    SBWatchpoint sbWatchpoint = SBWatchpoint::GetWatchpointFromEvent(sbEvent);
    switch (watchpointEventType) {
        case lldb::eWatchpointEventTypeInvalidType:
            wpEventType = "eWatchpointEventTypeInvalidType";
            break;
        case lldb::eWatchpointEventTypeAdded:
            wpEventType = "eWatchpointEventTypeAdded";
            HandleWatchpointAddedEvent(sbWatchpoint, false);
            break;
        case lldb::eWatchpointEventTypeRemoved:
            wpEventType = "eWatchpointEventTypeRemoved";
            HandleWatchpointRemoveEvent(sbWatchpoint, false);
            break;
        case lldb::eWatchpointEventTypeEnabled:
            wpEventType = "eWatchpointEventTypeEnabled";
            break;
        case lldb::eWatchpointEventTypeDisabled:
            wpEventType = "eWatchpointEventTypeDisabled";
            break;
        case lldb::eWatchpointEventTypeCommandChanged:
            wpEventType = "eWatchpointEventTypeCommandChanged";
            break;
        case lldb::eWatchpointEventTypeConditionChanged:
            wpEventType = "eWatchpointEventTypeConditionChanged";
            break;
        case lldb::eWatchpointEventTypeIgnoreChanged:
            wpEventType = "eWatchpointEventTypeIgnoreChanged";
            break;
        case lldb::eWatchpointEventTypeThreadChanged:
            wpEventType = "eWatchpointEventTypeThreadChanged";
            break;
        case lldb::eWatchpointEventTypeTypeChanged:
            wpEventType = "eWatchpointEventTypeTypeChanged";
            break;
        default: {
            wpEventType = "UnKnownBreakpointEvent";
        }
    }
    LOG_INFO("Watchpoint event handle finished, event type is ", wpEventType);
}

std::string LLDBApiEventHandler::GetBreakpointDescription(SBBreakpoint &rBreakpoint)
{
    SBStream sbStream;
    rBreakpoint.GetDescription(sbStream, true);
    return sbStream.GetData() == nullptr ? "" : sbStream.GetData();
}

// Events handled by this function include Breakpoint,InstructionBreakPoint,FunctionBreakPoint and ExceptionBreakpoints.
void LLDBApiEventHandler::HandleBreakpointAddedEvent(const SBBreakpoint &sbBreakpoint, bool isNeedSendEvent)
{
    SBBreakpoint &rBreakpoint = const_cast<SBBreakpoint &>(sbBreakpoint);
    BreakpointEventBody breakpointEventBody;
    GetBreakPointInfo(sbBreakpoint, breakpointEventBody);
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    breakpointEvent.body = breakpointEventBody;

    bool notExist = apiDebugger->GetBreakpointType(rBreakpoint.GetID()) == BreakpointType::UNKNOWN ? true : false;
    string bpDescription = GetBreakpointDescription(rBreakpoint);
    BreakpointType breakpointType = GetBreakPointTypeFromDescription(bpDescription);
    switch (breakpointType) {
        case BreakpointType::FUNCTION_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = FUNC_BREAKPOINT;
            breakpointEvent.body.breakpoint.source->name =
                    GetKeyWordsFromDescription(bpDescription, BREAKPOINT_FUNC_NAME, "'");
            if (notExist && breakpointEvent.body.breakpoint.source->name.has_value()) {
                apiDebugger->GetFunctionBreakpointMap().
                        AddBreakPoint(breakpointEvent.body.breakpoint.source->name.value(),
                                      breakpointEvent.body.breakpoint.id.value());
            }
            break;
        case BreakpointType::INSTRUCTION_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = INST_BREAKPOINT;
            if (notExist && breakpointEvent.body.breakpoint.instructionReference.has_value()) {
                apiDebugger->GetInstructionBreakpointMap().
                        AddBreakPoint(breakpointEvent.body.breakpoint.id.value(),
                                      breakpointEvent.body.breakpoint.instructionReference.value());
            }
            break;
        case BreakpointType::BREAKPOINT:
            breakpointEvent.body.breakpoint.message = LINE_BREAKPOINT;
            if (notExist && breakpointEvent.body.breakpoint.source->path.has_value()) {
                apiDebugger->GetBreakpointMap().AddBreakPoint(breakpointEvent.body.breakpoint.source->path.value(),
                                                              breakpointEvent.body.breakpoint.id.value());
            }
            break;
        case BreakpointType::EXCEPTIOM_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = EXCP_BREAKPOINT;
            if (notExist) {
                apiDebugger->GetExceptionBreakpoints().
                        AddExceptionBreakPoint(breakpointEvent.body.breakpoint.id.value());
            }
            break;
        default: // breakpoint not exist
            break;
    }
    if (isNeedSendEvent) {
        apiDebugger->SendEvent(breakpointEvent);
    }
}

// Events handled by this function include Breakpoint,InstructionBreakPoint,FunctionBreakPoint and ExceptionBreakpoints.
void LLDBApiEventHandler::HandleBreakpointRemoveEvent(const SBBreakpoint &sbBreakpoint, bool isNeedSendEvent)
{
    SBBreakpoint &rBreakpoint = const_cast<SBBreakpoint &>(sbBreakpoint);
    BreakpointEventBody breakpointEventBody;
    GetBreakPointInfo(sbBreakpoint, breakpointEventBody);
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    breakpointEvent.body = breakpointEventBody;
    breakpointEvent.body.reason = Reason::REMOVED;

    bool isExist = apiDebugger->GetBreakpointType(sbBreakpoint.GetID()) != BreakpointType::UNKNOWN;
    string bpDescription = GetBreakpointDescription(rBreakpoint);
    BreakpointType breakpointType = GetBreakPointTypeFromDescription(bpDescription);
    switch (breakpointType) {
        case BreakpointType::FUNCTION_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = FUNC_BREAKPOINT;
            breakpointEvent.body.breakpoint.source->name =
                    GetKeyWordsFromDescription(bpDescription, BREAKPOINT_FUNC_NAME, "'");
            if (isExist && breakpointEvent.body.breakpoint.source->name.has_value()) {
                apiDebugger->GetFunctionBreakpointMap().
                        RemoveBreakPoint(breakpointEvent.body.breakpoint.source->name.value(),
                                         breakpointEvent.body.breakpoint.id.value());
            }
            break;
        case BreakpointType::INSTRUCTION_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = INST_BREAKPOINT;
            if (isExist) {
                apiDebugger->GetInstructionBreakpointMap().RemoveBreakPoint(breakpointEvent.body.breakpoint.id.value());
            }
            break;
        case BreakpointType::BREAKPOINT:
            breakpointEvent.body.breakpoint.message = LINE_BREAKPOINT;
            if (isExist && breakpointEvent.body.breakpoint.source->path.has_value()) {
                apiDebugger->GetBreakpointMap().RemoveBreakPoint(breakpointEvent.body.breakpoint.source->path.value(),
                                                                 breakpointEvent.body.breakpoint.id.value());
            }
            break;
        case BreakpointType::EXCEPTIOM_BREAKPOINT:
            breakpointEvent.body.breakpoint.message = EXCP_BREAKPOINT;
            if (isExist) {
                apiDebugger->GetExceptionBreakpoints().
                        RemoveExceptionBreakPoint(breakpointEvent.body.breakpoint.id.value());
            }
            break;
        default: // breakpoint not exist
            break;
    }
    if (isNeedSendEvent) {
        apiDebugger->SendEvent(breakpointEvent);
    }
}

void LLDBApiEventHandler::HandleWatchpointAddedEvent(const SBWatchpoint &sbWatchpoint, bool isNeedSendEvent)
{
    SBWatchpoint &rWatchpoint = const_cast<SBWatchpoint &>(sbWatchpoint);
    BreakpointEventBody breakpointEventBody;
    GetWatchPointInfo(sbWatchpoint, breakpointEventBody);

    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    BreakpointType breakpointType =  apiDebugger->GetBreakpointType(rWatchpoint.GetID());
    if (breakpointType == BreakpointType::UNKNOWN) {
        apiDebugger->GetDataBreakpoints().AddDataBreakPoint(rWatchpoint.GetID());
    }
    breakpointEvent.body = breakpointEventBody;
    if (isNeedSendEvent) {
        apiDebugger->SendEvent(breakpointEvent);
    }
}

void LLDBApiEventHandler::HandleWatchpointRemoveEvent(const SBWatchpoint &sbWatchpoint, bool isNeedSendEvent)
{
    SBWatchpoint &rWatchpoint = const_cast<SBWatchpoint &>(sbWatchpoint);
    BreakpointEventBody breakpointEventBody;
    GetWatchPointInfo(sbWatchpoint, breakpointEventBody);

    BreakpointEvent breakpointEvent;
    breakpointEvent.body = breakpointEventBody;
    breakpointEvent.seq = apiDebugger->GetAndIncCurrentMessageSeq();
    breakpointEvent.body.reason = Reason::REMOVED;
    BreakpointType breakpointType =  apiDebugger->GetBreakpointType(rWatchpoint.GetID());
    if (breakpointType == BreakpointType::DATA_BREAKPOINT) {
        apiDebugger->GetDataBreakpoints().RemoveDataBreakPoint(rWatchpoint.GetID());
    }
    if (isNeedSendEvent) {
        apiDebugger->SendEvent(breakpointEvent);
    }
}

void LLDBApiEventHandler::GetBreakPointInfo(const lldb::SBBreakpoint &sbBreakpoint,
                                            Dap::Protocol::BreakpointEventBody &breakpointEventBody)
{
    SBBreakpoint &rBreakpoint = const_cast<SBBreakpoint &>(sbBreakpoint);
    SBBreakpointLocation sbBreakpointLocation = rBreakpoint.GetLocationAtIndex(0);
    SBAddress sbAddress = sbBreakpointLocation.GetAddress();
    lldb::addr_t loadAddress = sbAddress.GetLoadAddress(apiDebugger->GetSbTarget());
    if (loadAddress == LLDB_INVALID_ADDRESS) {
        loadAddress = sbAddress.GetFileAddress();
    }
    string addr = CommonUtils::Dec2HexAddress(loadAddress, CommonUtils::hexAddressWidth64BitsCpu);

    string unKnown = "??";
    string filePath = unKnown;
    string filename = unKnown;
    string functionName = unKnown;
    int lineNum = -1;
    int column = -1;
    SBSymbolContext sbSymbolContext = sbAddress.GetSymbolContext(eSymbolContextEverything);
    SBCompileUnit sbCompileUnit = sbSymbolContext.GetCompileUnit();
    if (sbCompileUnit.IsValid()) {
        // include Breakpoint\InstructionBreakPoint\FunctionBreakPoint event
        SBFileSpec sbFileSpec = sbCompileUnit.GetFileSpec();
        filename = sbFileSpec.GetFilename() == nullptr ? unKnown : sbFileSpec.GetFilename();
        filePath = sbFileSpec.GetDirectory() == nullptr ? unKnown : sbFileSpec.GetFilename();
        filePath = CommonUtils::ReplaceString(filePath + "/" + filename, "\\", "/");
        SBFunction sbFunction = sbSymbolContext.GetFunction();
        if (sbFunction.IsValid()) {
            functionName = sbFunction.GetName();
        }
        SBLineEntry sbLineEntry = sbSymbolContext.GetLineEntry();
        if (sbLineEntry.GetLine() > 0) {
            lineNum = sbLineEntry.GetLine();
        }
        column = sbLineEntry.GetColumn();
    }
    string condition = rBreakpoint.GetCondition() != nullptr ? rBreakpoint.GetCondition() : "";
    if (!condition.empty()) {
        breakpointEventBody.breakpoint.condition = condition;
    }
    breakpointEventBody.breakpoint.verified = rBreakpoint.IsValid();
    breakpointEventBody.breakpoint.id = rBreakpoint.GetID();
    breakpointEventBody.breakpoint.instructionReference = addr;
    breakpointEventBody.breakpoint.source = Source();
    breakpointEventBody.breakpoint.source->name = functionName;
    breakpointEventBody.breakpoint.source->path = filePath;
    breakpointEventBody.breakpoint.line = lineNum;
    breakpointEventBody.breakpoint.column = column;
    breakpointEventBody.hitCount = sbBreakpointLocation.GetHitCount();
    breakpointEventBody.ignoreCount = sbBreakpointLocation.GetIgnoreCount();
}

void LLDBApiEventHandler::GetWatchPointInfo(const lldb::SBWatchpoint &sbWatchpoint,
                                            Dap::Protocol::BreakpointEventBody &breakpointEventBody)
{
    SBWatchpoint &rWatchpoint = const_cast<SBWatchpoint &>(sbWatchpoint);
    breakpointEventBody.breakpoint.verified = rWatchpoint.IsValid();
    breakpointEventBody.breakpoint.id = rWatchpoint.GetID();
    breakpointEventBody.breakpoint.message = DATA_BREAKPOINT;
    lldb::addr_t watchAddress = rWatchpoint.GetWatchAddress();
    string addr = watchAddress != LLDB_INVALID_ADDRESS ?
                  CommonUtils::Dec2HexAddress(watchAddress, CommonUtils::hexAddressWidth64BitsCpu) : "-";
    breakpointEventBody.breakpoint.instructionReference = addr;
    int size = rWatchpoint.GetWatchSize();
    string dataId = std::to_string(watchAddress) + "/" + std::to_string(size);
    breakpointEventBody.dataBreakpoint = DataBreakpoint();
    breakpointEventBody.dataBreakpoint->dataId = dataId;
    std::string condition = rWatchpoint.GetCondition() != nullptr ? rWatchpoint.GetCondition() : "";
    if (!condition.empty()) {
        breakpointEventBody.dataBreakpoint->condition = condition;
    }
    SBStream sbStream;
    bool ret = rWatchpoint.GetDescription(sbStream, lldb::eDescriptionLevelVerbose);
    string bpDescription = sbStream.GetData() == nullptr ? "" : sbStream.GetData();
    if (ret && !bpDescription.empty()) {
        breakpointEventBody.dataBreakpoint->accessType = GetWatchPointAccessType(bpDescription);
    }
    breakpointEventBody.hitCount = rWatchpoint.GetHitCount();
    breakpointEventBody.ignoreCount = rWatchpoint.GetIgnoreCount();
    string valueName = GetKeyWordsFromDescription(bpDescription, WATCHPOINT_SPEC, "'");
    if (!valueName.empty()) {
        breakpointEventBody.description = valueName;
    }
}

BreakpointType LLDBApiEventHandler::GetBreakPointTypeFromDescription(const std::string &breakpointDescription) const
{
    if (breakpointDescription.find(BREAKPOINT_FILE_ADDR) != string::npos &&
        breakpointDescription.find(BREAKPOINT_LINE_NUM) != string::npos) {
        return BreakpointType::BREAKPOINT;
    } else if (breakpointDescription.find(BREAKPOINT_FUNC_NAME) != string::npos &&
               breakpointDescription.find(EXCP_BREAKPOINT) == string::npos) {
        return BreakpointType::FUNCTION_BREAKPOINT;
    } else if (breakpointDescription.find(BREAKPOINT_ADDR) != string::npos) {
        return BreakpointType::INSTRUCTION_BREAKPOINT;
    } else if (breakpointDescription.find(EXCP_BREAKPOINT) != string::npos) {
        return BreakpointType::EXCEPTIOM_BREAKPOINT;
    } else {
        return BreakpointType::UNKNOWN;
    }
}

DataBreakpointAccessType LLDBApiEventHandler::GetWatchPointAccessType(const std::string &watchpointDescription)
{
    int position = watchpointDescription.find(BREAKPOINT_ACCESS_TYPE);
    if (watchpointDescription.at(position + BREAKPOINT_ACCESS_TYPE.size()) == 'r') {
        return DataBreakpointAccessType::READ;
    } else if (watchpointDescription.at(position + BREAKPOINT_ACCESS_TYPE.size()) == 'w') {
        return DataBreakpointAccessType::WRITE;
    } else {
        return DataBreakpointAccessType::READ_WRITE;
    }
}

string LLDBApiEventHandler::GetKeyWordsFromDescription(const std::string &description,
                                                       const std::string &stringBeforeKeyWord,
                                                       const std::string &stringAfterKeyWord) const
{
    size_t position = description.find(stringBeforeKeyWord);
    if (position != string::npos) {
        string temp = description.substr(position + stringBeforeKeyWord.size());
        return temp.substr(0, temp.find(stringAfterKeyWord));
    }
    return "";
}

int LLDBApiEventHandler::GetRunningCoroutineId()
{
    if (apiDebugger->GetDebuggeeLanguageType() != DebuggeeLanguageType::CANGJIE) {
        return -1;
    }
    std::string coroutineInfo;
    apiDebugger->ExecCommand("cjthread list", coroutineInfo);
    // Compatible with cjdb earlier versions
    if (coroutineInfo.find("'cjthread' doesn't take any arguments") != std::string::npos) {
        apiDebugger->ExecCommand("cjthread", coroutineInfo);
        return SplitHistoryVersionCoroutineId(coroutineInfo);
    } else if (coroutineInfo.find("'cjthread' is not a valid command") != std::string::npos) {
        apiDebugger->ExecCommand("coroutine", coroutineInfo);
        return SplitHistoryVersionCoroutineId(coroutineInfo);
    }
    return SplitCoroutineId(coroutineInfo);
}

int LLDBApiEventHandler::SplitHistoryVersionCoroutineId(const std::string &coroutineInfo)
{
    if (coroutineInfo.empty()) {
        return -1;
    }
    /* eg:
     *      cjthread id: 1, state: pending
     *      cjthread id: 2, state: running
     *      cjthread id: 3, state: ready
     */
    auto coroutineInfos = CommonUtils::SplitString(coroutineInfo, "\n");
    for (const auto &line: coroutineInfos) {
        if (line.find("running") != std::string::npos) {
            size_t idPosition = line.find("id:");
            size_t statePosition = line.find(", state:");
            if (idPosition == std::string::npos || statePosition == std::string::npos || idPosition >= statePosition) {
                continue;
            }
            size_t start = idPosition + strlen("id: ");
            std::string runningId = line.substr(start, statePosition - start);
            int runningCoroutineId = -1;
            CommonUtils::StringToInt(runningId, runningCoroutineId);
            return runningCoroutineId;
        }
    }
    return -1;
}

int LLDBApiEventHandler::SplitCoroutineId(const std::string &coroutineInfo)
{
    if (coroutineInfo.empty()) {
        return -1;
    }
    /* eg:
     *      cjthread #0 state: running name:
     *        frame #0:  0x00005555555f5af1 main`default::main() at complex.cj:16:11
     *      cjthread #1 state = ready  name = ""
     *        frame #0: 0x00007ffff7f961e2 libcangjie-runtime.so`CJ_TimerSleep + 66
     */
    auto coroutineInfos = CommonUtils::SplitString(coroutineInfo, "\n");
    for (const auto &line: coroutineInfos) {
        if (line.find("state: running") != std::string::npos) {
            size_t idPosition = line.find("cjthread #");
            size_t statePosition = line.find(" state:");
            size_t start = idPosition + strlen("cjthread #");
            std::string runningId = line.substr(start, statePosition - start);
            int runningCoroutineId = -1;
            CommonUtils::StringToInt(runningId, runningCoroutineId);
            return runningCoroutineId;
        }
    }
    return -1;
}
