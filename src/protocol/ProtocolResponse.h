// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLRESPONSE_H
#define DAP_PROTOCOLRESPONSE_H

#include "defs.h"
#include "ProtocolBase.h"
#include "ProtocolDefs.h"
#include "ProtocolTypes.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

// Error
struct ErrorResponse : public Response {
    ErrorResponse(std::string requestCommand) : Response(""), requestCommand(requestCommand) {};
    std::optional<Message> error;
    std::string requestCommand;
};

// Cancel
struct CancelResponse : public Response {
    CancelResponse() : Response(REQ_RES_CANCEL) {};
};

// Launch
struct LaunchResponse : public Response {
    LaunchResponse() : Response(REQ_RES_LAUNCH) {};
};

// Attach
struct AttachResponse : public Response {
    AttachResponse() : Response(REQ_RES_ATTACH) {};
};

// Initialize
struct InitializeResponse : public Response {
    InitializeResponse() : Response(REQ_RES_INITIALIZE) {};
    std::optional<Capabilities> body;
};

// ConfigurationDone
struct ConfigurationDoneResponse : public Response {
    ConfigurationDoneResponse() : Response(REQ_RES_CONFIGURATION_DONE) {};
};

// Restart
struct RestartResponse : public Response {
    RestartResponse() : Response(REQ_RES_RESTART) {};
};

// Disconnect
struct DisconnectResponse : public Response {
    DisconnectResponse() : Response(REQ_RES_DISCONNECT) {};
};

// Terminate
struct TerminateResponse : public Response {
    TerminateResponse() : Response(REQ_RES_TERMINATE) {};
};

// BreakpointLocations
struct BreakpointLocationsResponse : public Response {
    BreakpointLocationsResponse() : Response(REQ_RES_BREAKPOINT_LOCATIONS) {};
    std::vector<BreakpointLocation> breakpoints;
};

struct BreakpointsResponse : public Response {
    explicit BreakpointsResponse(const std::string &cmd) : Response(cmd) {};
    std::vector<Breakpoint> breakpoints;
};

// SetBreakpoints
struct SetBreakpointsResponse : public BreakpointsResponse {
    SetBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_BREAKPOINTS) {};
};

// SetFunctionBreakpoints
struct SetFunctionBreakpointsResponse : public BreakpointsResponse {
    SetFunctionBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_FUNCTION_BREAKPOINTS) {};
};

// SetInstructionBreakpoints
struct SetInstructionBreakpointsResponse : public BreakpointsResponse {
    SetInstructionBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_INSTRUCTION_BREAKPOINTS) {};
};

// DataBreakpointInfo
struct DataBreakpointInfoResponse : public Response {
    DataBreakpointInfoResponse() : Response(REQ_RES_DATA_BREAKPOINT_INFO) {};
    std::string dataId;
    std::string description;
    std::optional<std::vector<DataBreakpointAccessType>> accessTypes;
    std::optional<bool> canPersist;
};

// SetExceptionBreakpointsResponse
struct SetExceptionBreakpointsResponse : public BreakpointsResponse {
    SetExceptionBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_EXCEPTION_BREAKPOINTS) {};
};

// SetDataBreakpoints
struct SetDataBreakpointsResponse : public BreakpointsResponse {
    SetDataBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_DATA_BREAKPOINTS) {};
};

// Continue
struct ContinueResponse : public Response {
    ContinueResponse() : Response(REQ_RES_CONTINUE) {};
    std::optional<bool> allThreadsContinued;
};

// Pause
struct PauseResponse : public Response {
    PauseResponse() : Response(REQ_RES_PAUSE) {};
};

// Next
struct NextResponse : public Response {
    NextResponse() : Response(REQ_RES_NEXT) {};
};

// Variables
struct VariablesResponse : public Response {
    explicit VariablesResponse(const std::string &command = REQ_RES_VARIABLES) : Response(command) {};
    std::vector<Variable> variables;
    std::optional<int> variablesCount;
};

struct VariablesExpandResponse : public VariablesResponse {
    VariablesExpandResponse() : VariablesResponse(REQ_RES_VARIABLES_EXPAND) {};
};

struct PreVariablesResponse : public VariablesResponse {
    PreVariablesResponse() : VariablesResponse(REQ_RES_PRE_VARIABLES) {};
};

// StepIn
struct StepInResponse : public Response {
    StepInResponse() : Response(REQ_RES_STEP_IN) {};
};

// StepOut
struct StepOutResponse : public Response {
    StepOutResponse() : Response(REQ_RES_STEP_OUT) {};
};

// Threads
struct Thread {
    int64_t id;
    std::string name;
    std::optional<int64_t> tid;
};

struct ThreadsResponse : public Response {
    ThreadsResponse() : Response(REQ_RES_THREADS) {};
    std::vector<Thread> threads;
};

// Modules
struct LoadModulesResponse : public Response {
    LoadModulesResponse() : Response(REQ_RES_LOAD_MODULES) {};
    std::vector<Module> modules;
    std::optional<int64_t> totalModules;
    std::optional<std::string> error;
};

// Source Mapping
struct AddSourceMappingResponse : public Response {
    AddSourceMappingResponse() : Response(REQ_RES_ADD_SOURCE_MAPPING) {};
    std::vector<int32_t> moduleIdList;
    std::vector<std::string> sourceRootDirList;
    std::optional<std::string> error;
};

struct ResetSourceMappingResponse : public Response {
    ResetSourceMappingResponse() : Response(REQ_RES_RESET_SOURCE_MAPPING) {};
    std::vector<int32_t> moduleIdList;
    std::optional<std::string> error;
};

// StackTrace
struct StackTraceResponse : public Response {
    StackTraceResponse() : Response(REQ_RES_STACK_TRACE) {};
    std::vector<StackFrame> stackFrames;
    std::optional<int64_t> totalFrames;
};

// Scopes
struct ScopesResponse : public Response {
    ScopesResponse() : Response(REQ_RES_SCOPES) {};
    std::vector<Scope> scopes;
};

// SetVariable
struct SetVariableResponse : public Response {
    SetVariableResponse() : Response(REQ_RES_SET_VARIABLE) {};
    std::string value;
    // original name is "type". To avoid the repetition of the protocol type, change to "variableType".
    std::optional<std::string> variableType;
    std::optional<int64_t> variablesReference;
    std::optional<int64_t> namedVariables;
    std::optional<int64_t> indexedVariables;
};

// RunInTerminal
struct RunInTerminalResponse : public Response {
    RunInTerminalResponse() : Response(REQ_RES_RUN_IN_TERMINAL) {};
    std::optional<int64_t> processId;
    std::optional<int64_t> shellProcessId;
};

// Disassemble
struct DisassembleResponse : public Response {
    DisassembleResponse() : Response(REQ_RES_DISASSEMBLE) {}
    std::vector<DisassembledInstruction> instructions;
};

// ReadMemory
struct ReadMemoryResponse : public Response {
    ReadMemoryResponse() : Response(REQ_RES_READ_MEMORY) {}
    std::string address;
    std::optional<int64_t> unreadableBytes;
    std::optional<std::string> data; // encoded using base64
};

// WriteMemory
struct WriteMemoryResponse : public Response {
    WriteMemoryResponse() : Response(REQ_RES_WRITE_MEMORY) {}
    std::optional<int64_t> offset;
    std::optional<int64_t> bytesWritten;
};

// Evaluate
struct EvaluateResponse : public Response {
    EvaluateResponse() : Response(REQ_RES_EVALUATE) {}
    std::string result;
    std::optional<std::string> resultType;
    std::optional<VariablePresentationHint> presentationHint;
    /*
     * If variablesReference is > 0, the evaluate result is structured and its children can be retrieved by passing
     * variablesReference to the VariablesRequest.
     * The value should be less than or equal to 2147483647 (2^31-1).
     */
    int variablesReference = 0;
    std::optional<int64_t> namedVariables;
    std::optional<int64_t> indexedVariables;
    std::optional<std::string> memoryReference;
    // huawei
    std::optional<std::string> memoryAddress;  // hex string, start with "0x"
};

// DebugInConsole
struct DebugInConsoleResponse : public Response {
    DebugInConsoleResponse() : Response(REQ_RES_DEBUG_IN_CONSOLE) {}
    std::string output;
};

// Source
struct SourceResponse : public Response {
    SourceResponse() : Response(REQ_RES_SOURCE) {}
    std::string content;
    std::optional<std::string> mimeType;
};

// ReverseStep
struct ReverseStepResponse : public Response {
    ReverseStepResponse() : Response(REQ_RES_REVERSE_STEP) {};
};

// Undo
struct UndoResponse : public Response {
    UndoResponse() : Response(REQ_RES_UNDO) {};
};

// ReverseContinue
struct ReverseContinueResponse : public Response {
    ReverseContinueResponse() : Response(REQ_RES_REVERSE_CONTINUE) {};
};

// SetExecutionPoint
struct SetExecutionPointResponse : public Response {
    SetExecutionPointResponse() : Response(REQ_RES_SET_EXECUTION_POINT) {};
};

// GotoRecordPoint
struct GotoRecordPointResponse : public Response {
    GotoRecordPointResponse() : Response(REQ_RES_GOTO_RECORD_POINT) {};
};

// RecordPoints
struct RecordPointsResponse : public Response {
    explicit RecordPointsResponse() : Response(REQ_RES_RECORD_POINTS) {};
    std::vector<RecordPoint> recordPoints;
};

// ContinueInReverse
struct ContinueInReverseResponse : public Response {
    ContinueInReverseResponse() : Response(REQ_RES_CONTINUE_IN_REVERSE) {};
};

// StepInReverse
struct StepInReverseResponse : public Response {
    StepInReverseResponse() : Response(REQ_RES_STEP_IN_REVERSE) {};
};

// CacheDataConfig
struct CacheDataConfigResponse : public Response {
    explicit CacheDataConfigResponse() : Response(REQ_RES_CACHE_DATA_CONFIG) {};
};

// RecordMessage
struct RecordMessage {
    int recordPointId = -1;
    std::optional<Thread> thread;
    std::optional<StackFrame> stackFrame;
    std::optional<std::vector<Variable>> localVariables;
    std::optional<std::vector<Variable>> globalVariables;
    std::optional<std::vector<Variable>> staticVariables;
    std::optional<std::vector<MonitorVariable>> monitorVariables;
};

struct RecordMessageResponse : public Response {
    explicit RecordMessageResponse() : Response(REQ_RES_RECORD_MESSAGE) {};
    std::vector<RecordMessage> recordMessages;
};

// SetMemoryBreakpoints
struct SetMemoryBreakpointsResponse : public BreakpointsResponse {
    explicit SetMemoryBreakpointsResponse() : BreakpointsResponse(REQ_RES_SET_MEMORY_BREAKPOINTS) {};
};

// GetBitmap
struct GetBitmapResponse : public Response {
    GetBitmapResponse() : Response(REQ_RES_GET_BITMAP) {}
    Bitmap bitmap;
};

// VariableHexView
struct VariableHexViewResponse : public Response {
    VariableHexViewResponse() : Response(REQ_RES_VARIABLE_HEX_VIEW) {}
};

// AddSourceBreakpoint
struct AddSourceBreakpointResponse : public Response {
    AddSourceBreakpointResponse() : Response(REQ_RES_ADD_SOURCE_BREAKPOINT) {};
    Breakpoint breakpoint;
};

// DeleteBreakpoint
struct DeleteBreakpointResponse : public Response {
    DeleteBreakpointResponse() : Response(REQ_RES_DELETE_BREAKPOINT) {};
};

// CommandCompletionResponse
struct CommandCompletionObject {
    std::string command;
    std::optional<std::string> alias;
    std::string spec;
};

struct CommandCompletionResponse : public Response {
    CommandCompletionResponse() : Response(REQ_RES_COMMAND_COMPLETION) {};
    std::vector<CommandCompletionObject> objects;
    bool isPatch = false;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLRESPONSE_H
