// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLTYPES_H
#define DAP_PROTOCOLTYPES_H

#include <optional>
#include "defs.h"
#include "ProtocolEnums.h"
#include "DapLog.h"
#include "CommonUtils.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

struct ExceptionBreakpointsFilter;
struct ColumnDescriptor;
struct Capabilities {
    std::optional<std::vector<ExceptionBreakpointsFilter>> exceptionBreakpointFilters;
    std::optional<std::vector<std::string>> completionTriggerCharacters;
    std::optional<std::vector<ColumnDescriptor>> additionalModuleColumns;
    std::optional<std::vector<ChecksumAlgorithmType>> supportedChecksumAlgorithms;
    std::optional<std::vector<std::string>> supportedTimeTravelRecordPointTypes;
    std::optional<bool> supportsConfigurationDoneRequest;
    std::optional<bool> supportsFunctionBreakpoints;
    std::optional<bool> supportsConditionalBreakpoints;
    std::optional<bool> supportsHitConditionalBreakpoints;
    std::optional<bool> supportsEvaluateForHovers;
    std::optional<bool> supportsStepBack;
    std::optional<bool> supportsSetVariable;
    std::optional<bool> supportsRestartFrame;
    std::optional<bool> supportsGotoTargetsRequest;
    std::optional<bool> supportsStepInTargetsRequest;
    std::optional<bool> supportsCompletionsRequest;
    std::optional<bool> supportsModulesRequest;
    std::optional<bool> supportsRestartRequest;
    std::optional<bool> supportsExceptionOptions;
    std::optional<bool> supportsValueFormattingOptions;
    std::optional<bool> supportsExceptionInfoRequest;
    std::optional<bool> supportTerminateDebuggee;
    std::optional<bool> supportsDelayedStackTraceLoading;
    std::optional<bool> supportsLoadedSourcesRequest;
    std::optional<bool> supportsLogPoints;
    std::optional<bool> supportsTerminateThreadsRequest;
    std::optional<bool> supportsSetExpression;
    std::optional<bool> supportsTerminateRequest;
    std::optional<bool> supportsDataBreakpoints;
    std::optional<bool> supportsReadMemoryRequest;
    std::optional<bool> supportsWriteMemoryRequest;
    std::optional<bool> supportsDisassembleRequest;
    std::optional<bool> supportsCancelRequest;
    std::optional<bool> supportsBreakpointLocationsRequest;
    std::optional<bool> supportsClipboardContext;
    std::optional<bool> supportsSteppingGranularity;
    std::optional<bool> supportsInstructionBreakpoints;
    std::optional<bool> supportsExceptionFilterOptions;
    std::optional<bool> supportsExceptionBreakpoints;
    std::optional<bool> supportsSymbolicBreakpoints;
    std::optional<bool> supportsTimeTravelDebugging;
    std::optional<bool> supportsSetExecutionPoint;
};

struct ExceptionBreakpointsFilter {
    std::string filter;
    std::string label;
    std::optional<bool> default_;
    std::optional<bool> supportsCondition;
};

struct ColumnDescriptor {
    std::string attributeName;
    std::string label;
    std::optional<std::string> format;
    std::optional<ColumnDescriptorType> type;
    std::optional<int64_t> width;
};

struct MessageVariable {
    std::string key;
    std::string value;
};

struct Message {
    int64_t id;
    std::string format;
    std::optional<MessageVariable> variables;
    std::optional<bool> sendTelemetry;
    std::optional<bool> showUser;
    std::optional<std::string> url;
    std::optional<std::string> urlLabel;
};

struct Checksum {
    ChecksumAlgorithmType algorithm;
    std::string checksum;
};

struct Source {
    std::optional<std::string> name;
    std::optional<std::string> path;
    std::optional<int64_t> sourceReference;
    std::optional<PresentationHint> presentationHint;
    std::optional<std::string> origin;
    std::optional<std::vector<Source>> sources;
    std::optional<any> adapterData;
    std::optional<std::vector<Checksum>> checksums;
};

struct Breakpoint {
    bool verified = false;
    std::optional<int64_t> id;
    std::optional<std::string> message;
    std::optional<Source> source;
    std::optional<int64_t> line;
    std::optional<int64_t> column;
    std::optional<int64_t> endLine;
    std::optional<int64_t> endColumn;
    std::optional<std::string> instructionReference;
    std::optional<int64_t> offset;
    // huawei
    std::optional<std::string> condition;  // only for breakpoint event
};

struct BreakpointLocation {
    int64_t line = -1;
    std::optional<int64_t> column;
    std::optional<int64_t> endLine;
    std::optional<int64_t> endColumn;
    std::optional<std::string> address;
};

struct FunctionBreakpointData {
    std::optional<bool> supportsConditionalBreakpoints;
    std::optional<bool> supportsHitConditionalBreakpoints;
    std::optional<bool> supportsLogPoints;
    std::optional<bool> supportsFunctionBreakpoints;
    std::optional<bool> supportsDataBreakpoints;
    std::optional<int64_t> id;
    std::optional<bool> verified;
    std::optional<int64_t> line;
    std::optional<std::string> sessionId;
};

struct FunctionBreakpoint {
    std::string name;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
    std::optional<bool> enabled;
    std::optional<std::string> id;
    std::optional<json_t> sessionData;
    std::optional<FunctionBreakpointData> data;
    std::optional<bool> isRegex;
    std::optional<bool> hitOnce;
    // huawei
    std::optional<std::string> moduleName;
};

struct SymbolicBreakpoint {
    std::string symbolName;
    std::optional<std::string> moduleName;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
    std::optional<bool> isRegex;
    std::optional<bool> hitOnce;
};

struct SourceBreakpoint {
    int64_t line = -1;
    std::optional<int64_t> column;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
    std::optional<std::string> logMessage;
    std::optional<bool> hitOnce;
};

struct InstructionBreakpoint {
    std::string instructionReference;
    std::optional<int64_t> offset;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
};

struct MemoryBreakpoint {
    std::string address;
    DataBreakpointAccessType accessType;
};

struct ValueFormat {
    virtual ~ValueFormat() = default;
    std::optional<bool> hex;
};

struct VariableFormat : public ValueFormat {
    // extension filed
};

struct StackFrameFormat : public ValueFormat {
    std::optional<bool> parameters;
    std::optional<bool> parameterTypes;
    std::optional<bool> parameterNames;
    std::optional<bool> parameterValues;
    std::optional<bool> line;
    std::optional<bool> module;
    std::optional<bool> includeAll;
};

struct VariablePresentationHint {
    std::optional<Kind> kind;
    std::optional<std::vector<Attributes>> attributes;
    std::optional<Visibility> visibility;
};

struct Variable {
    std::string name;
    std::string value;
    int64_t variablesReference = 0;
    std::optional<std::string> type;
    std::optional<VariablePresentationHint> presentationHint;
    std::optional<std::string> evaluateName;
    std::optional<int64_t> namedVariables;
    std::optional<int64_t> indexedVariables;
    std::optional<std::string> memoryReference;
    // huawei
    std::optional<int64_t> declaredLine = -1;
    std::optional<Source> declaredSource;
    std::optional<bool> isVarCreated = true;
    std::optional<std::string> memoryAddress;  // hex string, start with "0x"
};

struct Module {
    int64_t id;
    std::string uuid;
    std::string name;
    std::optional<std::string> path;
    std::optional<bool> isOptimized;
    std::optional<bool> isUserCode;
    std::optional<std::string> version;
    std::optional<std::string> symbolStatus;
    std::optional<std::string> symbolFilePath;
    std::optional<std::string> dateTimeStamp;
    std::optional<std::string> addressRange;
    std::optional<std::string> codeRange;
    std::optional<std::string> loadedAddress;
    std::optional<std::string> platformFilePath;
};

struct StackFrame {
    int64_t id;
    std::string name;
    int64_t line;
    int64_t column;
    std::optional<Source> source;
    std::optional<int64_t> endLine;
    std::optional<int64_t> endColumn;
    std::optional<std::string> instructionPointerReference;
    std::optional<int64_t> moduleId;
    std::optional<StackFramePresentationHint> presentationHint;
    std::optional<std::string> symbolInstructionRange;  // support mixed stacks(DevEco).
    std::optional<std::string> moduleName;
};

// vscode launch setUpCommands
struct SetupCommand {
    bool ignoreFailures = true;
    std::string description;
    std::string text;
};

struct Scope {
    std::string name;
    int64_t variablesReference;
    bool expensive;
    std::optional<std::string> presentationHint;
};

struct SourceBreakpointExpand {
    Source source;
    int64_t line = -1;
    std::optional<int64_t> column;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
    std::optional<std::string> logMessage;
    std::optional<bool> hitOnce;
};

struct EnvPair {
    std::string key;
    std::string value;
};

struct DisassembledInstruction {
    std::string address;
    std::optional<std::string> instructionBytes;
    std::string instruction;
    std::optional<std::string> symbol;
    std::optional<Source> location;
    std::optional<int64_t> line;
    std::optional<int64_t> column;
    std::optional<int64_t> endLine;
    std::optional<int64_t> endColumn;
};

struct DataBreakpoint {
    std::string dataId;
    std::optional<DataBreakpointAccessType> accessType;
    std::optional<std::string> condition;
    std::optional<std::string> hitCondition;
};

struct ExceptionPathSegment {
    std::vector<std::string> names;
    std::optional<bool> negate;
};

struct ExceptionOptions {
    ExceptionBreakMode breakMode = ExceptionBreakMode::NEVER;
    std::optional<std::vector<ExceptionPathSegment>> path;
};

struct ExceptionFilterOptions {
    std::string filterId;
    std::optional<std::string> condition;
};

struct RecordPoint {
    int id;
    std::string dateTimeStamp;
    uint64_t timeStamp = 0;
    std::string reason;
    int64_t threadId = -1;
    std::string threadName;
    std::optional<int64_t> coroutineId;
};

struct SourceFileMap {
    std::string remotePath;
    std::string localPath;

    SourceFileMap(const std::string &remote, const std::string &local)
    {
        remotePath = CommonUtils::ReplaceString(remote, "\\", "/");
        remotePath = CommonUtils::ConvertDriveLetterToUppercase(remotePath);
        localPath = CommonUtils::ReplaceString(local, "\\", "/");
        localPath = CommonUtils::ConvertDriveLetterToUppercase(localPath);
    }

    std::string ToLocalPath(const std::string &remoteFullPath)
    {
        std::string newRemoteFullPath = CommonUtils::ReplaceString(remoteFullPath, "\\", "/");
        newRemoteFullPath = CommonUtils::ConvertDriveLetterToUppercase(newRemoteFullPath);
        if (newRemoteFullPath.find(remotePath) != std::string::npos) {
            std::string local = localPath + newRemoteFullPath.substr(remotePath.length());
            return local;
        } else {
            LOG_ERROR("remoteFullPath is not under the SourceFileMap.remotePath");
            return newRemoteFullPath;
        }
    }

    std::string ToRemotePath(const std::string &localFullPath)
    {
        std::string newLocalFullPath = CommonUtils::ReplaceString(localFullPath, "\\", "/");
        newLocalFullPath = CommonUtils::ConvertDriveLetterToUppercase(newLocalFullPath);
        if (newLocalFullPath.find(localPath) != std::string::npos) {
            std::string remote = remotePath + newLocalFullPath.substr(localPath.length());
            return remote;
        } else {
            LOG_ERROR("localFullPath is not under the SourceFileMap.localPath");
            return newLocalFullPath;
        }
    }
};

struct MonitorVariable {
    int id = -1;
    std::optional<std::string> name;
    std::optional<std::string> scopeName;
    std::optional<std::string> type;
    std::optional<std::string> value;
};

struct BitmapFormat {
    int colorFormat = -1;
    int alphaFormat = -1;
};

struct Bitmap {
    int width = 0;
    int height = 0;
    BitmapFormat format;
    std::optional<uint32_t> size;
    std::optional<std::string> data;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLTYPES_H
