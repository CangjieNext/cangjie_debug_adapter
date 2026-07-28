// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUGGER_H
#define DAP_SERVER_DEBUGGER_H

#include <memory>
#include <mutex>
#include <variant>
#include <unordered_set>
#include "Protocol.h"
#include "ChannelHandler.h"
#include "Config.h"
#include "LaunchHelper.h"
#include "AttachHelper.h"
#include "DebugCacheHelper.h"
#include "BreakPointMap.h"
#include "FunctionBreakPointMap.h"
#include "DataBreakpointSet.h"
#include "ExceptionBreakpointSet.h"
#include "InstructionBreakPointMap.h"

namespace Dap {
class TimeTravelDebugManager;

struct RequestArgumentsContent {
    std::variant<
            Dap::Protocol::StackTraceArguments,
            Dap::Protocol::ScopesArguments,
            Dap::Protocol::VariablesArguments> content;
};

struct DebuggerOption {
    RemoteClass remote = RemoteClass::NO_REMOTE;
    bool linesStartAt1 = true;
    bool columnsStartAt1 = true;
    bool stopAtEntry = false;
    bool externalConsole = true;
    bool supportsRunInTerminalRequest = false;
    bool showStaticGlobalVars = false;
    bool filterInvalidLocals = false;
    // true: render original library data structure (eg, STL)
    bool libraryRenderer = false;
    Dap::StartMode startMode = Dap::StartMode::LAUNCH_MODE;
    std::string logPath;
    std::string clientDebugType;
    std::string debuggeePath;
    std::string adapterID;
    std::string debuggeeRemotePath;
    std::string projectDirectory;
    LaunchParams launchParams;
    AttachParams attachParams;
    std::vector<std::string> args;
    std::map<std::string, std::string> env;

    bool OhosCangjieDebug() const
    {
        return adapterID == "DAP-CANGJIE";
    }
};

enum class DebuggeeState {
    RUNNING,
    STOPPED,
    EXIT,
};

enum class DebuggeeLanguageType {
    CPP,
    CANGJIE
};

enum class RequestHandleMode {
    EXTERNAL,
    INTERNAL
};

enum class BreakpointType {
    BREAKPOINT,
    DATA_BREAKPOINT,
    FUNCTION_BREAKPOINT,
    EXCEPTIOM_BREAKPOINT,
    INSTRUCTION_BREAKPOINT,
    UNKNOWN
};

class DebuggerMessageHandler {
public:
    virtual void OnDebuggerResponse(const Dap::Protocol::Response &response) = 0;
    virtual void OnDebuggerEvent(const Dap::Protocol::Event &event) = 0;
    virtual void OnDebuggerRequest(const Dap::Protocol::Request & /* request */) {} // reverse request, runInTerminal
    virtual void OnDebuggerError(const int &errorCode, const std::string &msg) const = 0;
    virtual void OnDebuggerExit(const int &exitCode) const = 0;
    virtual ~DebuggerMessageHandler() = default;
};

class Debugger {
public:
    explicit Debugger() {}
    virtual ~Debugger() = default;

    const static int NO_INITIALIZE_REQUEST = -2;
    /**
     * return    -1: request not supported
     *           -2: first command is not initialize request
     *            0: request has been sent
     *        other: other error
     */
    virtual int OnDapRequest(std::unique_ptr<Dap::Protocol::Request> request) = 0;
    virtual void OnDapResponse(std::unique_ptr<Dap::Protocol::Response> /* response */) {} // RunInTerminalResponse
    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual void SendResponse(const Dap::Protocol::Response &res)
    {
        msgHandler->OnDebuggerResponse(res);
    }

    void SendFailedResponse(const Dap::Protocol::Request &req, const std::string &errorMsg)
    {
        Dap::Protocol::ErrorResponse response(req.command);
        response.seq = GetAndIncCurrentMessageSeq();
        response.request_seq = req.seq;
        response.success = false;
        response.message = errorMsg;
        msgHandler->OnDebuggerResponse(response);
    }

    virtual void SendEvent(const Dap::Protocol::Event &event) const
    {
        msgHandler->OnDebuggerEvent(event);
    }

    virtual void SendRequest(const Dap::Protocol::Request &request) const
    {
        msgHandler->OnDebuggerRequest(request);
    }

    unsigned int GetAndIncCurrentMessageSeq()
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        currentMessageSeq = (currentMessageSeq) % Protocol::SEQ_MAX;
        return ++currentMessageSeq;
    }

    void SetInitialized(bool init)
    {
        debugSessionInitialized = init;
    }

    bool IsDapInitialized() const
    {
        return debugSessionInitialized;
    }

    void RegisterDebuggerMessageHandler(const std::shared_ptr<DebuggerMessageHandler> handler)
    {
        msgHandler = handler;
    }

    void OnDebuggerExit(const int &exitCode) const
    {
        msgHandler->OnDebuggerExit(exitCode);
    }

    DebuggeeState GetDebuggeeState() const
    {
        return debuggeeState;
    }

    void SetDebuggeeState(DebuggeeState state)
    {
        debuggeeState = state;
    }

    DebuggeeLanguageType &GetDebuggeeLanguageType()
    {
        return debuggeeLanguageType;
    }

    void SetDebuggeeLanguageType(DebuggeeLanguageType type)
    {
        debuggeeLanguageType = type;
    }

    Dap::Protocol::RunInTerminalResponse GetRunInTerminalResponse() const
    {
        return runInTerminalResponse;
    }

    std::shared_ptr<TimeTravelDebugManager> GetTimeTravelManager() const
    {
        return timeTravelManager;
    }

    void SetTimeTravelManager(std::shared_ptr<TimeTravelDebugManager> manager)
    {
        timeTravelManager = std::move(manager);
    }

    void SetRequestArgumentsCache(unsigned int requestSeq, RequestArgumentsContent requestArguments)
    {
        requestArgumentsCache[requestSeq] = requestArguments;
    }

    bool GetRequestArgumentsFromCache(unsigned int requestSeq, RequestArgumentsContent &content)
    {
        if (requestArgumentsCache.count(requestSeq) < 1) {
            return false;
        }
        content = requestArgumentsCache[requestSeq];
        return true;
    }

    void DeleteRequestArgumentsFromCache(unsigned int requestSeq)
    {
        if (requestArgumentsCache.count(requestSeq) == 1) {
            requestArgumentsCache.erase(requestSeq);
        }
    }

    void ClearRequestArgumentsCache()
    {
        requestArgumentsCache.clear();
    }

    void SetCurrentStepGranularity(Dap::Protocol::SteppingGranularity steppingGranularity)
    {
        currentStepGranularity = steppingGranularity;
    }

    Dap::Protocol::SteppingGranularity GetCurrentStepGranularity() const
    {
        return currentStepGranularity;
    }

    void SetRequestHandleMode(RequestHandleMode mode)
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        requestHandleMode = mode;
    }

    RequestHandleMode GetRequestHandleMode()
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        return requestHandleMode;
    }

    void MarkInternalRequest(unsigned int requestSeq)
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        internalRequestSet.insert(requestSeq);
    }

    void UnMarkInternalRequest(unsigned int requestSeq)
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        internalRequestSet.erase(requestSeq);
    }

    void ClearMarkInternalRequest()
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        internalRequestSet.clear();
    }

    bool IsInternalRequest(unsigned int requestSeq)
    {
        std::lock_guard<std::mutex> lock(debuggerMutex);
        return (internalRequestSet.count(requestSeq) == 1);
    }

    std::shared_ptr<DebugCacheHelper> GetDebugCacheHelper() const
    {
        return debugCacheHelper;
    }

    void SetDebugCacheHelper(std::shared_ptr<DebugCacheHelper> cacheHelper)
    {
        debugCacheHelper = std::move(cacheHelper);
    }

    std::string &GetBreakpointLogMsg(const int &number)
    {
        return breakpointLogMsg[number];
    }

    void AddBreakPointLogMsg(const std::string &logMsg, const int &number)
    {
        breakpointLogMsg[number] = logMsg;
    }

    // return: 1.success  0.failed
    size_t RemoveBreakPointLogMsg(const int &number)
    {
        return breakpointLogMsg.erase(number);
    }

    bool HasBreakPointLogMsg(const int &number) const
    {
        return breakpointLogMsg.count(number) != 0;
    }

    int32_t GetEntryBreakpointID()
    {
        return entryBreakpointID;
    }

    void SetEntryBreakpointID(const int32_t& id)
    {
        entryBreakpointID = id;
    }

    BreakPointMap &GetBreakpointMap()
    {
        return breakpointMap;
    }

    FunctionBreakPointMap &GetFunctionBreakpointMap()
    {
        return functionBreakpointMap;
    }

    InstructionBreakPointMap &GetInstructionBreakpointMap()
    {
        return instructionBreakpointMap;
    }

    DataBreakpointSet &GetDataBreakpoints()
    {
        return dataBreakpointSet;
    }

    DataBreakpointSet &GetMemoryBreakpoints()
    {
        return memoryBreakpointSet;
    }

    ExceptionBreakpointSet &GetExceptionBreakpoints()
    {
        return exceptionBreakpointSet;
    }

    BreakpointType GetBreakpointType(int breakpointId)
    {
        if (functionBreakpointMap.IsFunctionBreakPoint(breakpointId)) {
            return BreakpointType::FUNCTION_BREAKPOINT;
        } else if (instructionBreakpointMap.IsInstructionBreakPoint(breakpointId)) {
            return BreakpointType::INSTRUCTION_BREAKPOINT;
        } else if (breakpointMap.IsSourceBreakPoint(breakpointId)) {
            return BreakpointType::BREAKPOINT;
        } else if (exceptionBreakpointSet.IsExceptionBreakPoint(breakpointId)) {
            return BreakpointType::EXCEPTIOM_BREAKPOINT;
        } else {
            return BreakpointType::UNKNOWN;
        }
    }

    Protocol::StoppedReason GetSpecificBreakpointStopReason(int breakpointId)
    {
        BreakpointType breakpointType = GetBreakpointType(breakpointId);
        switch (breakpointType) {
            case BreakpointType::BREAKPOINT:
                return Protocol::StoppedReason::BREAKPOINT;
            case BreakpointType::FUNCTION_BREAKPOINT:
                return Protocol::StoppedReason::FUNCTION_BREAKPOINT;
            case BreakpointType::INSTRUCTION_BREAKPOINT:
                return Protocol::StoppedReason::INSTRUCTION_BREAKPOINT;
            case BreakpointType::EXCEPTIOM_BREAKPOINT:
                return Protocol::StoppedReason::EXCEPTION;
            case BreakpointType::DATA_BREAKPOINT:
                return Protocol::StoppedReason::DATA_BREAKPOINT;
            case BreakpointType::UNKNOWN:
            default:
                return Protocol::StoppedReason::BREAKPOINT;
        }
    }

    void SetPauseOperation(bool pauseOperation)
    {
        isPauseOperation = pauseOperation;
    }

    bool IsPauseOperation() const
    {
        return isPauseOperation;
    }

protected:
    FunctionBreakPointMap functionBreakpointMap;
    DataBreakpointSet dataBreakpointSet;
    DataBreakpointSet memoryBreakpointSet;
    ExceptionBreakpointSet exceptionBreakpointSet;
    BreakPointMap breakpointMap;
    InstructionBreakPointMap instructionBreakpointMap;
    bool isPauseOperation = false;

    std::shared_ptr<DebuggerMessageHandler> msgHandler;
    DebuggeeState debuggeeState = DebuggeeState::EXIT;
    Dap::Protocol::RunInTerminalResponse runInTerminalResponse;
    std::shared_ptr<TimeTravelDebugManager> timeTravelManager = nullptr;
    std::shared_ptr<DebugCacheHelper> debugCacheHelper = nullptr;
    std::mutex handleMutex;
    std::condition_variable handleCv;
    bool dataReady = false;
    int32_t entryBreakpointID = -1;
    struct ChangedVar {
        int parentRef = 0;
        std::string varName;
    } changedVar;

private:
    DebuggeeLanguageType debuggeeLanguageType = DebuggeeLanguageType::CPP;
    unsigned int currentMessageSeq = 0;
    std::mutex debuggerMutex;
    bool debugSessionInitialized = false;
    std::map<int, std::string> breakpointLogMsg = {};
    RequestHandleMode requestHandleMode = RequestHandleMode::EXTERNAL;
    std::unordered_set<unsigned int> internalRequestSet;
    std::map<unsigned int, RequestArgumentsContent> requestArgumentsCache;
    Dap::Protocol::SteppingGranularity currentStepGranularity = Dap::Protocol::SteppingGranularity::LINE;
};
};

#endif // DAP_SERVER_DEBUGGER_H