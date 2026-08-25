// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLREQUESTS_H
#define DAP_PROTOCOLREQUESTS_H

#include <optional>
#include "defs.h"
#include "ProtocolBase.h"
#include "ProtocolDefs.h"
#include "ProtocolTypes.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

// Cancel
struct CancelArguments {
    std::optional<int64_t> requestId;
    std::optional<std::string> progressId;
};

struct CancelRequest : public Request {
    CancelRequest() : Request(REQ_RES_CANCEL) {}

    std::optional<CancelArguments> arguments;
};

// Launch
struct LaunchRequestArguments {
    std::optional<bool> noDebug;
    std::optional<any> restartData;
    // vscode
    std::optional<bool> stopAtEntry;
    std::optional<bool> externalConsole;
    std::optional<std::string> name;
    std::optional<std::string> type;
    std::optional<std::string> request;
    std::optional<std::string> program;
    std::optional<std::string> cwd;
    std::optional<std::vector<std::string>> args;
    std::optional<std::map<std::string, std::string>> env;
    std::optional<std::vector<SetupCommand>> setupCommands;
    std::optional<int> configurationTarget;
    std::optional<std::string> sessionId;
    // huawei
    std::optional<std::vector<std::string>> scriptCommands;
    std::optional<std::vector<std::string>> startupCommands;
    std::optional<std::string> remotePlatform;
    std::optional<std::string> remoteAddress;
    std::optional<std::string> remoteFilePath;
    std::optional<bool> remote;
    std::optional<SourceFileMap> sourceFileMap;
    std::optional<bool> debuggeeStarted;  // if true, use "continue" instead of "run" to resume debuggee
    std::optional<bool> remoteConnected;  // if true, remote already connected
    // for vm debug such as cjvm
    std::optional<bool> vmMode;
    std::optional<std::string> vmAddress;
    std::optional<int> vmPort;
};

struct LaunchRequest : public Request {
    LaunchRequest() : Request(REQ_RES_LAUNCH) {}

    LaunchRequestArguments arguments;
};

// Attach
struct AttachRequestArguments {
    std::optional<any> restartData;
    // vscode
    std::optional<std::string> name;
    std::optional<std::string> type;
    std::optional<std::string> request;
    std::optional<std::string> program;
    std::optional<std::string> processId;
    std::optional<std::vector<SetupCommand>> setupCommands;
    std::optional<std::vector<std::string>> scriptCommands;
    std::optional<int> configurationTarget;
    std::optional<std::string> sessionId;
    std::optional<std::string> cwd;
    // huawei
    std::optional<std::string> remotePlatform;
    std::optional<std::string> remoteAddress;
    std::optional<bool> remote;
    std::optional<bool> isContainer;
    std::optional<std::vector<std::string>> startupCommands;
    std::optional<std::vector<std::string>> afterConnectCommands;
    std::optional<std::vector<std::string>> postAttachCommands;
    std::optional<AttachStep> attachStep;
    std::optional<bool> showStaticGlobalVars;
    std::optional<SourceFileMap> sourceFileMap;
};

struct AttachRequest : public Request {
    AttachRequest() : Request(REQ_RES_ATTACH) {}

    AttachRequestArguments arguments;
};

// Initialize
struct InitializeRequestArguments {
    std::string adapterID;
    std::optional<std::string> clientID;
    std::optional<std::string> clientName;
    std::optional<std::string> locale;
    std::optional<bool> linesStartAt1;
    std::optional<bool> columnsStartAt1;
    std::optional<PathFormat> pathFormat;
    std::optional<bool> supportsVariableType;
    std::optional<bool> supportsVariablePaging;
    std::optional<bool> supportsRunInTerminalRequest;
    std::optional<bool> supportsMemoryReferences;
    std::optional<bool> supportsProgressReporting;
    std::optional<bool> supportsInvalidatedEvent;
};

struct InitializeRequest : public Request {
    InitializeRequest() : Request(REQ_RES_INITIALIZE) {}

    InitializeRequestArguments arguments;
};

// ConfigurationDone
struct ConfigurationDoneArguments {};

struct ConfigurationDoneRequest : public Request {
    ConfigurationDoneRequest() : Request(REQ_RES_CONFIGURATION_DONE) {}

    std::optional<ConfigurationDoneArguments> arguments;
};

// Restart
struct RestartArguments {};

struct RestartRequest : public Request {
    RestartRequest() : Request(REQ_RES_RESTART) {}
    std::optional<RestartArguments> arguments;
};

// Disconnect
struct DisconnectArguments {
    std::optional<bool> restart;
    std::optional<bool> terminateDebuggee;
    std::optional<bool> isDebuggeeCrashed;
};

struct DisconnectRequest : public Request {
    DisconnectRequest() : Request(REQ_RES_DISCONNECT) {}
    std::optional<DisconnectArguments> arguments;
};

// Terminate
struct TerminateArguments {
    std::optional<bool> restart;
};
struct TerminateRequest : public Request {
    TerminateRequest() : Request(REQ_RES_TERMINATE) {}
    std::optional<TerminateArguments> arguments;
};

// BreakpointLocations
struct BreakpointLocationsArguments {
    Source source;
    int64_t line;
    std::optional<int64_t> column;
    std::optional<int64_t> endLine;
    std::optional<int64_t> endColumn;
    std::optional<std::string> startAddress;
    std::optional<bool> filterStepinLocation;
};

struct BreakpointLocationsRequest : public Request {
    BreakpointLocationsRequest() : Request(REQ_RES_BREAKPOINT_LOCATIONS) {}
    std::optional<BreakpointLocationsArguments> arguments;
};

// SetBreakpoints
struct SetBreakpointsArguments {
    Source source;
    std::optional<std::vector<SourceBreakpoint>> breakpoints;
    std::optional<std::vector<int64_t>> lines;
    std::optional<bool> sourceModified;
};

struct SetBreakpointsRequest : public Request {
    SetBreakpointsRequest() : Request(REQ_RES_SET_BREAKPOINTS) {}
    SetBreakpointsArguments arguments;
};

// SetFunctionBreakpoints
struct SetFunctionBreakpointsArguments {
    std::vector<FunctionBreakpoint> breakpoints;
};

struct SetFunctionBreakpointsRequest : public Request {
    SetFunctionBreakpointsRequest() : Request(REQ_RES_SET_FUNCTION_BREAKPOINTS) {}
    SetFunctionBreakpointsArguments arguments;
};


// SetInstructionBreakpoints
struct SetInstructionBreakpointsArguments {
    std::vector<InstructionBreakpoint> breakpoints;
};

struct SetInstructionBreakpointsRequest : public Request {
    SetInstructionBreakpointsRequest() : Request(REQ_RES_SET_INSTRUCTION_BREAKPOINTS) {}
    SetInstructionBreakpointsArguments arguments;
};

// Continue
struct ContinueArguments {
    int64_t threadId = -1;
};

struct ContinueRequest : public Request {
    ContinueRequest() : Request(REQ_RES_CONTINUE) {}
    ContinueArguments arguments;
};

// Next
struct NextArguments {
    int64_t threadId = -1;
    std::optional<SteppingGranularity> granularity;
};

struct NextRequest : public Request {
    NextRequest() : Request(REQ_RES_NEXT) {}
    NextArguments arguments;
};

// Variables
struct VariablesArguments {
    int64_t variablesReference = 0;
    std::optional<Filter> filter;
    std::optional<int64_t> start;
    std::optional<int64_t> count;
    std::optional<ValueFormat> format;
};

struct VariablesRequest : public Request {
    explicit VariablesRequest(const std::string &command = REQ_RES_VARIABLES) : Request(command) {}
    VariablesArguments arguments;
};

// VariablesExpand
struct VariablesExpandRequest : public VariablesRequest {
    VariablesExpandRequest() : VariablesRequest(REQ_RES_VARIABLES_EXPAND) {}
};

struct PreVariablesRequest : public VariablesRequest {
    PreVariablesRequest() : VariablesRequest(REQ_RES_PRE_VARIABLES) {}
};

// StepIn
struct StepInArguments {
    int64_t threadId = -1;
    int64_t targetId = -1;
    std::optional<SteppingGranularity> granularity;
};

struct StepInRequest : public Request {
    StepInRequest() : Request(REQ_RES_STEP_IN) {}
    StepInArguments arguments;
};

// StepOut
struct StepOutArguments {
    int64_t threadId = -1;
    std::optional<SteppingGranularity> granularity;
};

struct StepOutRequest : public Request {
    StepOutRequest() : Request(REQ_RES_STEP_OUT) {}
    StepOutArguments arguments;
};

// StepBack
struct StepBackArguments {
    int64_t threadId = -1;
    std::optional<SteppingGranularity> granularity;
};

struct StepBackRequest : public Request {
    StepBackRequest() : Request(REQ_RES_STEP_BACK) {}
    StepBackArguments arguments;
};

// ReverseContinue
struct ReverseContinueArguments {
    int64_t threadId = -1;
};

struct ReverseContinueRequest : public Request {
    ReverseContinueRequest() : Request(REQ_RES_REVERSE_CONTINUE) {}
    ReverseContinueArguments arguments;
};

// RestartFrame
struct RestartFrameArguments {
    int64_t frameId = 0;
};

struct RestartFrameRequest : public Request {
    RestartFrameRequest() : Request(REQ_RES_RESTART_FRAME) {}
    RestartFrameArguments arguments;
};

// RestartFrame
struct GotoArguments {
    int64_t threadId = -1;
    int64_t targetId = -1;
};

struct GotoRequest : public Request {
    GotoRequest() : Request(REQ_RES_GOTO) {}
    GotoArguments arguments;
};

// Pause
struct PauseArguments {
    int64_t threadId = -1;
};

struct PauseRequest : public Request {
    PauseRequest() : Request(REQ_RES_PAUSE) {}
    PauseArguments arguments;
};

// StackTrace
struct StackTraceArguments {
    int64_t threadId = -1;
    std::optional<int64_t> startFrame;
    std::optional<int64_t> levels;
    std::optional<StackFrameFormat> format;
};

struct StackTraceRequest : public Request {
    StackTraceRequest() : Request(REQ_RES_STACK_TRACE) {}
    StackTraceArguments arguments;
};

// Scopes
struct ScopesArguments {
    int64_t frameId = 0;
};

struct ScopesRequest : public Request {
    ScopesRequest() : Request(REQ_RES_SCOPES) {}
    ScopesArguments arguments;
};

// SetVariable
struct SetVariableArguments {
    int64_t variablesReference = 0;
    std::string name;
    std::string value;
    std::optional<ValueFormat> format;
};

struct SetVariableRequest : public Request {
    SetVariableRequest() : Request(REQ_RES_SET_VARIABLE) {}
    SetVariableArguments arguments;
};

// Source
struct SourceArguments {
    int64_t sourceReference = 0;
    std::optional<Source> source;
};

struct SourceRequest : public Request {
    SourceRequest() : Request(REQ_RES_SOURCE) {}
    SourceArguments arguments;
};

// Threads
struct ThreadsRequest : public Request {
    ThreadsRequest() : Request(REQ_RES_THREADS) {};
};

// TerminateThreads
struct TerminateThreadsArguments {
    std::optional<std::vector<int64_t>> threadIds;
};

struct TerminateThreadsRequest : public Request {
    TerminateThreadsRequest() : Request(REQ_RES_TERMINATE_THREADS) {}

    TerminateThreadsArguments arguments;
};

// Modules
struct ModulesArguments {
    std::optional<int64_t> startModule;
    std::optional<int64_t> moduleCount;
};

struct ModulesRequest : public Request {
    ModulesRequest() : Request(REQ_RES_MODULES) {}

    ModulesArguments arguments;
};

// LoadedSources
struct LoadedSourcesArguments {};

struct LoadedSourcesRequest : public Request {
    LoadedSourcesRequest() : Request(REQ_RES_LOADED_SOURCES) {}

    std::optional<LoadedSourcesArguments> arguments;
};

// Evaluate
struct EvaluateArguments {
    std::string expression;
    std::optional<int64_t> frameId;
    std::optional<Context> context;
    std::optional<ValueFormat> format;
    std::optional<bool> isViewMore;
};

struct EvaluateRequest : public Request {
    EvaluateRequest() : Request(REQ_RES_EVALUATE) {}

    EvaluateArguments arguments;
};

// SetExpression
struct SetExpressionArguments {
    std::string expression;
    std::string value;
    std::optional<int64_t> frameId;
    std::optional<ValueFormat> format;
};

struct SetExpressionRequest : public Request {
    SetExpressionRequest() : Request(REQ_RES_SET_EXPRESSION) {}

    SetExpressionArguments arguments;
};

// RunInTerminal
struct RunInTerminalRequestArguments {
    std::string cwd;
    std::vector<std::string> args;
    std::optional<RunInTerminalKind> kind;
    std::optional<std::string> title;
    std::optional<std::vector<EnvPair>> env;
};

struct RunInTerminalRequest : public Request {
    RunInTerminalRequest() : Request(REQ_RES_RUN_IN_TERMINAL) {}

    RunInTerminalRequestArguments arguments;
};

// Disassemble
struct DisassembleArguments {
    std::string memoryReference;
    std::optional<int64_t> offset;
    std::optional<int64_t> instructionOffset;
    int64_t instructionCount = 0;
    std::optional<bool> resolveSymbols;
};

struct DisassembleRequest : public Request {
    DisassembleRequest() : Request(REQ_RES_DISASSEMBLE) {}
    DisassembleArguments arguments;
};

// ReadMemory
struct ReadMemoryArguments {
    std::string memoryReference;
    std::optional<int64_t> offset;
    uint64_t count = 0;
};

struct ReadMemoryRequest : public Request {
    explicit ReadMemoryRequest() : Request(REQ_RES_READ_MEMORY) {}
    ReadMemoryArguments arguments;
};

// WriteMemory
struct WriteMemoryArguments {
    std::string memoryReference;
    std::optional<int64_t> offset;
    std::optional<bool> allowPartial;
    std::string data; // encoded using base64
};

struct WriteMemoryRequest : public Request {
    explicit WriteMemoryRequest() : Request(REQ_RES_WRITE_MEMORY) {}
    WriteMemoryArguments arguments;
};

// DataBreakpointInfo
struct DataBreakpointInfoArguments {
    std::string name;
    std::optional<int64_t> variablesReference;
};

struct DataBreakpointInfoRequest : public Request {
    DataBreakpointInfoRequest() : Request(REQ_RES_DATA_BREAKPOINT_INFO) {}
    DataBreakpointInfoArguments arguments;
};

// SetDataBreakpoints
struct SetDataBreakpointsArguments {
    std::vector<DataBreakpoint> breakpoints;
};

struct SetDataBreakpointsRequest : public Request {
    SetDataBreakpointsRequest() : Request(REQ_RES_SET_DATA_BREAKPOINTS) {}
    SetDataBreakpointsArguments arguments;
};

// SetExceptionBreakpoints
struct SetExceptionBreakpointsArguments {
    std::vector<std::string> filters;
    std::optional<std::vector<ExceptionFilterOptions>> filterOptions;
    std::optional<std::vector<ExceptionOptions>> exceptionOptions;
};

struct SetExceptionBreakpointsRequest : public Request {
    explicit SetExceptionBreakpointsRequest() : Request(REQ_RES_SET_EXCEPTION_BREAKPOINTS) {}
    SetExceptionBreakpointsArguments arguments;
};

// DebugInConsole
struct DebugInConsoleArguments {
    std::string debugCommand;
};

struct DebugInConsoleRequest : public Request {
    DebugInConsoleRequest() : Request(REQ_RES_DEBUG_IN_CONSOLE) {}
    DebugInConsoleArguments arguments;
};

// ReverseStep
struct ReverseStepArguments {
    int64_t threadId = -1;
    std::optional<SteppingGranularity> granularity;
};
struct ReverseStepRequest : public Request {
    ReverseStepRequest() : Request(REQ_RES_REVERSE_STEP) {}
    ReverseStepArguments arguments;
};

// Undo
struct UndoArguments {
    bool undoEnable = false;
};
struct UndoRequest : public Request {
    UndoRequest() : Request(REQ_RES_UNDO) {}
    UndoArguments arguments;
};

// SetExecutionPoint
struct SetExecutionPointArguments {
    std::optional<std::string> sourcePath;
    std::optional<int64_t> line;
    std::optional<std::string> instructionReference;
};

struct SetExecutionPointRequest : public Request {
    SetExecutionPointRequest() : Request(REQ_RES_SET_EXECUTION_POINT) {}
    SetExecutionPointArguments arguments;
};

// GotoRecordPoint
struct GotoRecordPointArguments {
    int recordPointId = -1;
};
struct GotoRecordPointRequest : public Request {
    GotoRecordPointRequest() : Request(REQ_RES_GOTO_RECORD_POINT) {}
    GotoRecordPointArguments arguments;
};

// RecordPoints
struct RecordPointsArguments {
    std::vector<std::string> filters;   // server only return record points which is in filters
    std::optional<uint64_t> start;    // request data from an index of cache(not recordPoint id)
    std::optional<uint64_t> count;    // 0: return as many record points as possible
};
struct RecordPointsRequest :public Request {
    RecordPointsRequest() : Request(REQ_RES_RECORD_POINTS) {}
    RecordPointsArguments arguments;
};

// ContinueInReverse
struct ContinueInReverseArguments {
    int64_t threadId = -1;
};

struct ContinueInReverseRequest : public Request {
    ContinueInReverseRequest() : Request(REQ_RES_CONTINUE_IN_REVERSE) {}
    ContinueInReverseArguments arguments;
};

// StepInReverse
struct StepInReverseArguments {
    int64_t threadId = -1;
};

struct StepInReverseRequest : public Request {
    StepInReverseRequest() : Request(REQ_RES_STEP_IN_REVERSE) {}
    StepInReverseArguments arguments;
};

// CacheDataConfig
struct CacheDataConfigArguments {
    bool startCacheData = false;
    bool saveHistoryCache = true;
    std::optional<int64_t> threadCount;
    std::optional<int64_t> stackFrameCount;
    std::optional<int64_t> variableChildrenCount;
    std::optional<int64_t> variableChildrenLayers;
    std::optional<int64_t> stackTracePageSize;
    std::optional<int64_t> variablePageSize;
    std::optional<uint64_t> scopeType;
};

struct CacheDataConfigRequest :public Request {
    CacheDataConfigRequest() : Request(REQ_RES_CACHE_DATA_CONFIG) {}
    CacheDataConfigArguments arguments;
};

// RecordMessage
struct RecordMessageArguments {
    std::vector<int> recordPointIds;
    std::optional<bool> requireThread;
    std::optional<bool> requireStackFrame;
    std::optional<bool> requireLocalVariables;
    std::optional<bool> requireGlobalVariables;
    std::optional<bool> requireStaticVariables;
    std::optional<std::vector<MonitorVariable>> monitorVariables;
};

struct RecordMessageRequest :public Request {
    RecordMessageRequest() : Request(REQ_RES_RECORD_MESSAGE) {}
    RecordMessageArguments arguments;
};

// SetMemoryBreakpoints
struct SetMemoryBreakpointsArguments {
    std::vector<MemoryBreakpoint> breakpoints;
};

struct SetMemoryBreakpointsRequest :public Request {
    SetMemoryBreakpointsRequest() : Request(REQ_RES_SET_MEMORY_BREAKPOINTS) {}
    SetMemoryBreakpointsArguments arguments;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLREQUESTS_H
