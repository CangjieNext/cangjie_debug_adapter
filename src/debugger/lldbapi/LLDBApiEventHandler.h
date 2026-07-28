// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_EVENT_HANDLER_H
#define DAP_LLDBAPI_EVENT_HANDLER_H

#include <iostream>
#include <map>
#include "LLDB.h"
#include "Debugger.h"
#include "LLDBApiDebugger.h"
#include "Protocol.h"

namespace Dap {
class LLDBApiEventHandler {
public:
    explicit LLDBApiEventHandler(std::shared_ptr<LLDBApiDebugger> debugger) : apiDebugger(debugger) {};
    ~LLDBApiEventHandler() = default;

    bool StartListener();
    void StopListener();

    std::string GetErrorMsg()
    {
        return errorMsg;
    }

    Dap::Protocol::StoppedEvent GetCurrentStoppedEvent()
    {
        return currentStoppedEvent;
    }

    int GetPreviousStoppedThreadID()
    {
        return preStoppedThreadIndex;
    }

private:
    void ListenEvent();
    bool InitSBListener(lldb::SBListener &listener);
    // kind of event
    void HandleProcessEvent(const lldb::SBEvent &sbEvent);
    void HandleTargetEvent(const lldb::SBEvent &sbEvent);
    void HandleBreakpointEvent(const lldb::SBEvent &sbEvent);
    void HandleWatchpointEvent(const lldb::SBEvent &sbEvent);
    void HandleBreakpointAddedEvent(const lldb::SBBreakpoint &sbBreakpoint, bool isNeedSendEvent);
    void HandleBreakpointRemoveEvent(const lldb::SBBreakpoint &sbBreakpoint, bool isNeedSendEvent);
    void HandleWatchpointAddedEvent(const lldb::SBWatchpoint &sbWatchpoint, bool isNeedSendEvent);
    void HandleWatchpointRemoveEvent(const lldb::SBWatchpoint &sbWatchpoint, bool isNeedSendEvent);
    static std::string GetBreakpointDescription(lldb::SBBreakpoint &rBreakpoint);
    void GetBreakPointInfo(const lldb::SBBreakpoint &sbBreakpoint,
                           Dap::Protocol::BreakpointEventBody &breakpointEventBody);
    void GetWatchPointInfo(const lldb::SBWatchpoint &sbWatchpoint,
                           Dap::Protocol::BreakpointEventBody &breakpointEventBody);
    BreakpointType GetBreakPointTypeFromDescription(const std::string &breakpointDescription) const;
    Protocol::DataBreakpointAccessType GetWatchPointAccessType(const std::string &watchpointDescription);
    std::string GetKeyWordsFromDescription(const std::string &description, const std::string &stringBeforeKeyWord,
                                           const std::string &stringAfterKeyWord) const;
    int GetRunningCoroutineId();
    int SplitHistoryVersionCoroutineId(const std::string &coroutineInfo);
    int SplitCoroutineId(const std::string &coroutineInfo);

    // kind of process event
    void HandleProcessStateChangedEvent(const lldb::SBEvent &sbEvent);

    // kind of process state changed event
    void HandleStoppedEvent(const lldb::SBEvent &sbEvent);
    void GetStoppedReason(const lldb::SBEvent &sbEvent, lldb::SBThread &thread);
    void TimeTravelPreProcess();
    void HandleLogPoint(const int breakpointId);
    void HandleRunningEvent() const;
    void HandleExitedEvent();
    void SendTerminatedEvent(const bool &restart);
    void UpdateSelectThread() const;
    std::string GetExceptionDescription() const;
    std::string GetSignalDescription(const lldb::SBEvent &sbEvent) const;

    // kind of target event
    void HandleModulesLoadedEvent(const lldb::SBEvent &sbEvent);
    std::string GetModuleInfoStr(Dap::Protocol::Module &module);

    Dap::Protocol::StoppedEvent currentStoppedEvent;
    bool isListening = false;
    int preStoppedThreadIndex = -1;
    std::string errorMsg;
    std::shared_ptr<LLDBApiDebugger> apiDebugger = nullptr;
    const std::string BREAKPOINT_FILE_ADDR = "file =";
    const std::string BREAKPOINT_LINE_NUM = "line =";
    const std::string BREAKPOINT_FUNC_NAME = "name = '";
    const std::string BREAKPOINT_ADDR = "address =";
    const std::string BREAKPOINT_ACCESS_TYPE = "type = ";
    const std::string WATCHPOINT_SPEC = "watchpoint spec = '";
    const std::string EXCP_BREAKPOINT = "Exception breakpoint";
    const std::string LINE_BREAKPOINT = "Breakpoint";
    const std::string FUNC_BREAKPOINT = "Function Breakpoint";
    const std::string INST_BREAKPOINT = "Instruction Breakpoint";
    const std::string DATA_BREAKPOINT = "Data Breakpoint";
};
}
#endif // DAP_LLDBAPI_EVENT_HANDLER_H
