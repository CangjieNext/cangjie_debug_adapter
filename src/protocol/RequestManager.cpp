// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "RequestManager.h"
#include "EnumUtils.h"
#include "DapLog.h"

using namespace Dap::Protocol;

template <typename T>
void SetRequestField(const json_t &json, const std::string &fieldName, T &requestField)
{
    if (json.contains(fieldName) && !json[fieldName].is_null()) {
        requestField = json[fieldName];
    }
}

template <typename T, typename Function>
void SetConvertedRequestField(const json_t &json, const std::string &fieldName, T &requestField, Function function)
{
    if (json.contains(fieldName) && !json[fieldName].is_null()) {
        requestField = function();
    }
}

void RequestManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (isRegister) {
        return;
    }
    InsertRequest(requestFactory, REQ_RES_INITIALIZE, &ToInitializeRequest);
    InsertRequest(requestFactory, REQ_RES_LAUNCH, &ToLaunchRequest);
    InsertRequest(requestFactory, REQ_RES_ATTACH, &ToAttachRequest);
    InsertRequest(requestFactory, REQ_RES_CONFIGURATION_DONE, &ToConfigurationDoneRequest);
    InsertRequest(requestFactory, REQ_RES_RESTART, &ToRestartRequest);
    InsertRequest(requestFactory, REQ_RES_DISCONNECT, &ToDisconnectRequest);
    InsertRequest(requestFactory, REQ_RES_TERMINATE, &ToTerminateRequest);
    InsertRequest(requestFactory, REQ_RES_SET_BREAKPOINTS, &ToSetBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_BREAKPOINT_LOCATIONS, &ToBreakpointLocationsRequest);
    InsertRequest(requestFactory, REQ_RES_SET_FUNCTION_BREAKPOINTS, &ToSetFunctionBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_DATA_BREAKPOINT_INFO, &ToDataBreakpointInfoRequest);
    InsertRequest(requestFactory, REQ_RES_SET_DATA_BREAKPOINTS, &ToSetDataBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_SET_EXCEPTION_BREAKPOINTS, &ToSetExceptionBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_SET_INSTRUCTION_BREAKPOINTS, &ToSetInstructionBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_CONTINUE, &ToContinueRequest);
    InsertRequest(requestFactory, REQ_RES_PAUSE, &ToPauseRequest);
    InsertRequest(requestFactory, REQ_RES_NEXT, &ToNextRequest);
    InsertRequest(requestFactory, REQ_RES_STEP_IN, &ToStepInRequest);
    InsertRequest(requestFactory, REQ_RES_STEP_OUT, &ToStepOutRequest);
    InsertRequest(requestFactory, REQ_RES_STACK_TRACE, &ToStackTraceRequest);
    InsertRequest(requestFactory, REQ_RES_THREADS, &ToThreadsRequest);
    InsertRequest(requestFactory, REQ_RES_SCOPES, &ToScopesRequest);
    InsertRequest(requestFactory, REQ_RES_VARIABLES, &ToVariablesRequest);
    InsertRequest(requestFactory, REQ_RES_VARIABLES_EXPAND, &ToVariablesExpandRequest);
    InsertRequest(requestFactory, REQ_RES_PRE_VARIABLES, &ToPreVariables);
    InsertRequest(requestFactory, REQ_RES_SET_VARIABLE, &ToSetVariableRequest);
    InsertRequest(requestFactory, REQ_RES_DISASSEMBLE, &ToDisassembleRequest);
    InsertRequest(requestFactory, REQ_RES_EVALUATE, &ToEvaluateRequest);
    InsertRequest(requestFactory, REQ_RES_DEBUG_IN_CONSOLE, &ToDebugInConsoleRequest);
    InsertRequest(requestFactory, REQ_RES_SOURCE, &ToSourceRequest);
    InsertRequest(requestFactory, REQ_RES_READ_MEMORY, &ToReadMemoryRequest);
    InsertRequest(requestFactory, REQ_RES_WRITE_MEMORY, &ToWriteMemoryRequest);
    InsertRequest(requestFactory, REQ_RES_REVERSE_STEP, &ToReverseStepRequest);
    InsertRequest(requestFactory, REQ_RES_UNDO, &ToUndoRequest);
    InsertRequest(requestFactory, REQ_RES_REVERSE_CONTINUE, &ToReverseContinueRequest);
    InsertRequest(requestFactory, REQ_RES_SET_EXECUTION_POINT, &ToSetExecutionPointRequest);
    InsertRequest(requestFactory, REQ_RES_RECORD_POINTS, &ToRecordPointsRequest);
    InsertRequest(requestFactory, REQ_RES_GOTO_RECORD_POINT, &ToGotoRecordPointRequest);
    InsertRequest(requestFactory, REQ_RES_CONTINUE_IN_REVERSE, &ToContinueInReverseRequest);
    InsertRequest(requestFactory, REQ_RES_STEP_IN_REVERSE, &ToStepInReverseRequest);
    InsertRequest(requestFactory, REQ_RES_CACHE_DATA_CONFIG, &ToCacheDataConfigRequest);
    InsertRequest(requestFactory, REQ_RES_RECORD_MESSAGE, &ToRecordMessageRequest);
    InsertRequest(requestFactory, REQ_RES_SET_MEMORY_BREAKPOINTS, &ToSetMemoryBreakpointsRequest);
    InsertRequest(requestFactory, REQ_RES_GET_BITMAP, &ToGetBitmapRequest);
    InsertRequest(requestFactory, REQ_RES_LOAD_MODULES, &ToLoadModulesRequest);
    InsertRequest(requestFactory, REQ_RES_ADD_SOURCE_MAPPING, &ToAddSourceMappingRequest);
    InsertRequest(requestFactory, REQ_RES_RESET_SOURCE_MAPPING, &ToResetSourceMappingRequest);
    InsertRequest(requestFactory, REQ_RES_ADD_SOURCE_BREAKPOINT, &ToAddSourceBreakpointRequest);
    InsertRequest(requestFactory, REQ_RES_DELETE_BREAKPOINT, &ToDeleteBreakpointRequest);
    InsertRequest(requestFactory, REQ_RES_VARIABLE_HEX_VIEW, &ToVariableHexViewRequest);
    InsertRequest(requestFactory, REQ_RES_COMMAND_COMPLETION, &ToCommandCompletionRequest);

    reqToJsonFactory.emplace(REQ_RES_RUN_IN_TERMINAL, &ToRunInTerminalJson);
    isRegister = true;
}

void RequestManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!isRegister) {
        return;
    }
    requestFactory.clear();
    isRegister = false;
}

bool RequestManager::IsRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    return isRegister;
}

std::optional<RequestManager::CreateRequestFunc> RequestManager::GetCreateRequestFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (requestFactory.count(command) == 0) {
        return std::nullopt;
    }
    return requestFactory[command];
}

std::optional<RequestManager::RequestToJsonFunc> RequestManager::GetRequestJsonFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (reqToJsonFactory.count(command) == 0) {
        LOG_ERROR("not find target function, command is:", command);
        return std::nullopt;
    }
    return reqToJsonFactory[command];
}

bool RequestManager::IsRequest(const json_t &jsonRequest) const
{
    return (jsonRequest.contains("type")) && (jsonRequest["type"] == REQUEST_NAME);
}

std::string RequestManager::Command(const json_t &jsonRequest) const
{
    if (jsonRequest.contains("command") && jsonRequest["command"].is_string()) {
        return jsonRequest["command"].get<std::string>();
    }
    return "";
}

bool RequestManager::SetRequestBaseInfo(Request &request, const json_t &json)
{
    if (!json.contains("seq") || json["seq"].is_null() || !json.contains("type") || json["type"].is_null() ||
        !json.contains("command") || json["command"].is_null()) {
        return false;
    }
    request.seq = json["seq"];
    request.type = STR_TO_ENUM<Dap::Protocol::ProtocolMessage::Type>(json["type"]).value();
    return true;
}

std::unique_ptr<Request> RequestManager::FromJson(const json_t &requestJson)
{
    if (!IsRequest(requestJson)) {
        LOG_ERROR("this not request, requestJson is:", requestJson);
        return nullptr;
    }
    std::string command = Command(requestJson);
    std::optional<RequestManager::CreateRequestFunc> func = GetCreateRequestFunc(command);
    if (!func.has_value()) {
        LOG_ERROR("not find target function, command is:", command);
        return nullptr;
    }
    return func.value()(requestJson);
}

std::unique_ptr<Request> RequestManager::FromJson(const std::string &requestStr)
{
    json_t requestJson;
    try {
        requestJson = json_t::parse(requestStr);
    } catch (const json_t::parse_error &e) {
        LOG_ERROR("wrong json String, requestStr is:", requestStr);
        return nullptr;
    }
    return FromJson(requestJson);
}

std::optional<json_t> RequestManager::ToJson(const Request &request)
{
    std::string command = request.command;
    std::optional<RequestManager::RequestToJsonFunc> func = GetRequestJsonFunc(command);
    if (!func.has_value()) {
        LOG_ERROR("not find target function, request is:", command);
        return std::nullopt;
    }
    return func.value()(request);
}

bool RequestManager::SetRequestSource(const json_t &json, Dap::Protocol::Source &source)
{
    SetRequestField(json, "name", source.name);
    SetRequestField(json, "path", source.path);
    SetRequestField(json, "sourceReference", source.sourceReference);
    if (HasValue(json, "presentationHint")) {
        source.presentationHint = STR_TO_ENUM<PresentationHint>(json["presentationHint"]);
    }
    SetRequestField(json, "origin", source.origin);
    if (HasValue(json, "sources")) {
        std::vector<Source> sources;
        for (auto it = json["sources"].begin(); it != json["sources"].end(); ++it) {
            Source sourceElement;
            if (!SetRequestSource(it.value(), sourceElement)) {
                LOG_ERROR("SetRequestSource error, json is:", it.value());
                return false;
            }
            sources.push_back(std::move(sourceElement));
        }
        source.sources = std::move(sources);
    }
    if (HasValue(json, "adapterData")) {
        source.adapterData = json["adapterData"];
    }
    if (HasValue(json, "checksums")) {
        std::vector<Checksum> checksums;
        for (auto it = json["checksums"].begin(); it != json["checksums"].end(); ++it) {
            Checksum checksum;
            if (!HasValue(it.value(), "algorithm")) {
                LOG_ERROR("not find algorithm in json, json is:", it.value());
                return false;
            }
            if (!HasValue(it.value(), "checksum")) {
                LOG_ERROR("not find checksum in json, json is:", it.value());
                return false;
            }
            checksum.algorithm = STR_TO_ENUM<ChecksumAlgorithmType>(it.value()["algorithm"]).value();
            checksum.checksum = it.value()["checksum"];
            checksums.push_back(std::move(checksum));
        }
        source.checksums = std::move(checksums);
    }
    return true;
}

std::unique_ptr<Request> RequestManager::ToInitializeRequest(const json_t &json)
{
    std::unique_ptr<InitializeRequest> requestPointer = std::make_unique<InitializeRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToInitializeRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "adapterID")) {
        LOG_ERROR("not found arguments or adapterID in json, json is:", json);
        return nullptr;
    }
    requestPointer->arguments.adapterID = json["arguments"]["adapterID"];
    SetRequestField(json["arguments"], "clientID", requestPointer->arguments.clientID);
    SetRequestField(json["arguments"], "clientName", requestPointer->arguments.clientName);
    SetRequestField(json["arguments"], "locale", requestPointer->arguments.locale);
    SetRequestField(json["arguments"], "linesStartAt1", requestPointer->arguments.linesStartAt1);
    SetRequestField(json["arguments"], "columnsStartAt1", requestPointer->arguments.columnsStartAt1);
    SetConvertedRequestField(json["arguments"], "pathFormat", requestPointer->arguments.pathFormat,
        [&json]() { return STR_TO_ENUM<PathFormat>(json["arguments"]["pathFormat"]); });
    SetRequestField(json["arguments"], "supportsVariableType", requestPointer->arguments.supportsVariableType);
    SetRequestField(json["arguments"], "supportsVariablePaging", requestPointer->arguments.supportsVariablePaging);
    SetRequestField(json["arguments"], "supportsRunInTerminalRequest",
        requestPointer->arguments.supportsRunInTerminalRequest);
    SetRequestField(json["arguments"], "supportsMemoryReferences", requestPointer->arguments.supportsMemoryReferences);
    SetRequestField(json["arguments"], "supportsProgressReporting",
        requestPointer->arguments.supportsProgressReporting);
    SetRequestField(json["arguments"], "supportsInvalidatedEvent", requestPointer->arguments.supportsInvalidatedEvent);
    return requestPointer;
}

std::vector<SetupCommand> RequestManager::GetSetupCommands(const json_t &setupCommandsJson)
{
    std::vector<SetupCommand> setupCommandVec;
    for (auto cmdJson : setupCommandsJson) {
        SetupCommand setupCommand;
        SetRequestField(cmdJson, "ignoreFailures", setupCommand.ignoreFailures);
        SetRequestField(cmdJson, "description", setupCommand.description);
        SetRequestField(cmdJson, "text", setupCommand.text);
        setupCommandVec.push_back(std::move(setupCommand));
    }
    return setupCommandVec;
}

std::vector<std::string> RequestManager::GetVectorString(const json_t &stringVectorJson)
{
    std::vector<std::string> res;
    for (auto str : stringVectorJson) {
        res.push_back(str);
    }
    return res;
}

void RequestManager::SetLaunchArguments(const json_t &json, LaunchRequestArguments &arguments)
{
    SetRequestField(json["arguments"], "noDebug", arguments.noDebug);
    SetRequestField(json["arguments"], "__restart", arguments.restartData);
    SetRequestField(json["arguments"], "name", arguments.name);
    SetRequestField(json["arguments"], "type", arguments.type);
    SetRequestField(json["arguments"], "request", arguments.request);
    SetRequestField(json["arguments"], "program", arguments.program);
    SetRequestField(json["arguments"], "stopAtEntry", arguments.stopAtEntry);
    SetRequestField(json["arguments"], "cwd", arguments.cwd);
    SetRequestField(json["arguments"], "externalConsole", arguments.externalConsole);
    SetRequestField(json["arguments"], "__configurationTarget", arguments.configurationTarget);
    SetRequestField(json["arguments"], "__sessionId", arguments.sessionId);
    SetRequestField(json["arguments"], "remotePlatform", arguments.remotePlatform);
    SetRequestField(json["arguments"], "remoteAddress", arguments.remoteAddress);
    SetRequestField(json["arguments"], "remoteFilePath", arguments.remoteFilePath);
    SetRequestField(json["arguments"], "remote", arguments.remote);
    SetRequestField(json["arguments"], "debuggeeStarted", arguments.debuggeeStarted);
    SetRequestField(json["arguments"], "remoteConnected", arguments.remoteConnected);
    SetRequestField(json["arguments"], "vmMode", arguments.vmMode);
    SetRequestField(json["arguments"], "vmAddress", arguments.vmAddress);
    SetRequestField(json["arguments"], "vmPort", arguments.vmPort);
    if (HasValue(json["arguments"], "args") && json["arguments"]["args"].is_array()) {
        std::vector<std::string> argsVec;
        for (auto arg : json["arguments"]["args"]) {
            argsVec.push_back(arg);
        }
        arguments.args = std::move(argsVec);
    }
    if (HasValue(json["arguments"], "env")) {
        std::map<std::string, std::string> envMap;
        for (auto it = json["arguments"]["env"].begin(); it != json["arguments"]["env"].end(); ++it) {
            envMap[it.key()] = it.value();
        }
        arguments.env = std::move(envMap);
    }
    if (HasValue(json["arguments"], "setupCommands") && json["arguments"]["setupCommands"].is_array()) {
        arguments.setupCommands = GetSetupCommands(json["arguments"]["setupCommands"]);
    }
    if (HasValue(json["arguments"], "scriptCommands") && json["arguments"]["scriptCommands"].is_array()) {
        arguments.scriptCommands = GetVectorString(json["arguments"]["scriptCommands"]);
    }
    if (HasValue(json["arguments"], "startupCommands") && json["arguments"]["startupCommands"].is_array()) {
        arguments.startupCommands = GetVectorString(json["arguments"]["startupCommands"]);
    }
    if (HasValue(json["arguments"], "sourceFileMap")) {
        if (json["arguments"]["sourceFileMap"].is_object()) {
            std::string remotePath = json["arguments"]["sourceFileMap"].cbegin().key();
            std::string localPath = json["arguments"]["sourceFileMap"].cbegin().value();
            SourceFileMap fileMap(remotePath, localPath);
            arguments.sourceFileMap = std::move(fileMap);
        } else {
            LOG_ERROR("sourceFileMap error in SetLaunchArguments, json[\"arguments\"][\"sourceFileMap\"] is:",
                      json["arguments"]["sourceFileMap"]);
            return;
        }
    }
}

std::unique_ptr<Request> RequestManager::ToLaunchRequest(const json_t &json)
{
    std::unique_ptr<LaunchRequest> requestPointer = std::make_unique<LaunchRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToLaunchRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not found arguments in json, json is:", json);
        return nullptr;
    }
    LaunchRequestArguments launchRequestArguments;
    SetLaunchArguments(json, launchRequestArguments);
    requestPointer->arguments = std::move(launchRequestArguments);
    return requestPointer;
}

void RequestManager::SetAttachArguments(const json_t &json, AttachRequestArguments &arguments)
{
    SetRequestField(json["arguments"], "name", arguments.name);
    SetRequestField(json["arguments"], "type", arguments.type);
    SetRequestField(json["arguments"], "request", arguments.request);
    SetRequestField(json["arguments"], "program", arguments.program);
    SetRequestField(json["arguments"], "processId", arguments.processId);
    if (HasValue(json["arguments"], "setupCommands") && json["arguments"]["setupCommands"].is_array()) {
        arguments.setupCommands = GetSetupCommands(json["arguments"]["setupCommands"]);
    }
    if (HasValue(json["arguments"], "scriptCommands") && json["arguments"]["scriptCommands"].is_array()) {
        arguments.scriptCommands = GetVectorString(json["arguments"]["scriptCommands"]);
    }
    SetRequestField(json["arguments"], "__configurationTarget", arguments.configurationTarget);
    SetRequestField(json["arguments"], "__sessionId", arguments.sessionId);
    SetRequestField(json["arguments"], "cwd", arguments.cwd);
    SetRequestField(json["arguments"], "__restart", arguments.restartData);
    SetRequestField(json["arguments"], "remotePlatform", arguments.remotePlatform);
    SetRequestField(json["arguments"], "remoteAddress", arguments.remoteAddress);
    SetRequestField(json["arguments"], "remote", arguments.remote);
    SetRequestField(json["arguments"], "isContainer", arguments.isContainer);
    SetConvertedRequestField(json["arguments"], "attachStep", arguments.attachStep,
        [&json]() { return STR_TO_ENUM<AttachStep>(json["arguments"]["attachStep"]); });
    SetRequestField(json["arguments"], "showStaticGlobalVars", arguments.showStaticGlobalVars);
    if (HasValue(json["arguments"], "startupCommands") && json["arguments"]["startupCommands"].is_array()) {
        arguments.startupCommands = GetVectorString(json["arguments"]["startupCommands"]);
    }
    if (HasValue(json["arguments"], "afterConnectCommands") && json["arguments"]["afterConnectCommands"].is_array()) {
        arguments.afterConnectCommands = GetVectorString(json["arguments"]["afterConnectCommands"]);
    }
    if (HasValue(json["arguments"], "postAttachCommands") && json["arguments"]["postAttachCommands"].is_array()) {
        arguments.postAttachCommands = GetVectorString(json["arguments"]["postAttachCommands"]);
    }

    if (HasValue(json["arguments"], "sourceFileMap")) {
        if (json["arguments"]["sourceFileMap"].is_object()) {
            std::string remotePath = json["arguments"]["sourceFileMap"].cbegin().key();
            std::string localPath = json["arguments"]["sourceFileMap"].cbegin().value();
            SourceFileMap fileMap(remotePath, localPath);
            arguments.sourceFileMap = std::move(fileMap);
        } else {
            LOG_ERROR("sourceFileMap error in SetAttachArguments, json[\"arguments\"][\"sourceFileMap\"] is:",
                      json["arguments"]["sourceFileMap"]);
            return;
        }
    }
}

std::unique_ptr<Request> RequestManager::ToAttachRequest(const json_t &json)
{
    std::unique_ptr<AttachRequest> requestPointer = std::make_unique<AttachRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToAttachRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not found arguments in json, json is:", json);
        return nullptr;
    }
    AttachRequestArguments attachRequestArguments;
    json_t attachJson = json;
    if (json["arguments"].contains("processId") && json["arguments"]["processId"].is_number()) {
        attachJson["arguments"]["processId"] = to_string(attachJson["arguments"]["processId"]);
    }
    SetAttachArguments(attachJson, attachRequestArguments);
    requestPointer->arguments = std::move(attachRequestArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToConfigurationDoneRequest(const json_t &json)
{
    std::unique_ptr<ConfigurationDoneRequest> requestPointer = std::make_unique<ConfigurationDoneRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToConfigurationDoneRequest, json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        ConfigurationDoneArguments configurationDoneArguments;
        requestPointer->arguments = std::move(configurationDoneArguments);
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToRestartRequest(const json_t &json)
{
    std::unique_ptr<RestartRequest> requestPointer = std::make_unique<RestartRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToRestartRequest, json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        RestartArguments restartArguments;
        requestPointer->arguments = std::move(restartArguments);
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToDisconnectRequest(const json_t &json)
{
    std::unique_ptr<DisconnectRequest> requestPointer = std::make_unique<DisconnectRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToDisconnectRequest, json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        DisconnectArguments disconnectArguments;
        SetRequestField(json["arguments"], "restart", disconnectArguments.restart);
        SetRequestField(json["arguments"], "terminateDebuggee", disconnectArguments.terminateDebuggee);
        SetRequestField(json["arguments"], "isDebuggeeCrashed", disconnectArguments.isDebuggeeCrashed);
        requestPointer->arguments = std::move(disconnectArguments);
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToTerminateRequest(const json_t &json)
{
    std::unique_ptr<TerminateRequest> requestPointer = std::make_unique<TerminateRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToTerminateRequest, json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        TerminateArguments terminateArguments;
        SetRequestField(json["arguments"], "restart", terminateArguments.restart);
        requestPointer->arguments = std::move(terminateArguments);
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetBreakpointsRequest> requestPointer = std::make_unique<SetBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSetBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "source")) {
        LOG_ERROR("not found arguments or source in json, json is:", json);
        return nullptr;
    }
    SetBreakpointsArguments setBreakpointsArguments;
    if (!SetRequestSource(json["arguments"]["source"], setBreakpointsArguments.source)) {
        LOG_ERROR("SetRequestSource error in ToSetBreakpointsRequest, json[\"arguments\"][\"source\"] is:",
            json["arguments"]["source"]);
        return nullptr;
    }
    if (HasValue(json["arguments"], "breakpoints")) {
        std::vector<SourceBreakpoint> breakpoints;
        for (auto it = json["arguments"]["breakpoints"].begin(); it != json["arguments"]["breakpoints"].end(); ++it) {
            SourceBreakpoint sourceBreakpoint;
            if (!HasValue(it.value(), "line")) {
                LOG_ERROR("not find line in json, json is:", it.value());
                return nullptr;
            }
            sourceBreakpoint.line = it.value()["line"];
            SetRequestField(it.value(), "column", sourceBreakpoint.column);
            SetRequestField(it.value(), "condition", sourceBreakpoint.condition);
            SetRequestField(it.value(), "hitCondition", sourceBreakpoint.hitCondition);
            SetRequestField(it.value(), "logMessage", sourceBreakpoint.logMessage);
            breakpoints.push_back(std::move(sourceBreakpoint));
        }
        setBreakpointsArguments.breakpoints = std::move(breakpoints);
    }
    if (HasValue(json["arguments"], "lines")) {
        std::vector<int64_t> lines;
        for (auto it = json["arguments"]["lines"].begin(); it != json["arguments"]["lines"].end(); ++it) {
            lines.push_back(it.value());
        }
        setBreakpointsArguments.lines = std::move(lines);
    }
    SetRequestField(json["arguments"], "sourceModified", setBreakpointsArguments.sourceModified);
    requestPointer->arguments = std::move(setBreakpointsArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToBreakpointLocationsRequest(const json_t &json)
{
    std::unique_ptr<BreakpointLocationsRequest> requestPointer = std::make_unique<BreakpointLocationsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToBreakpointLocationsRequest, json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        BreakpointLocationsArguments breakpointLocationsArguments;
        if (!HasValue(json["arguments"], "source")) {
            LOG_ERROR("not find source in json, json[\"arguments\"] is:", json["arguments"]);
            return nullptr;
        }
        if (!SetRequestSource(json["arguments"]["source"], breakpointLocationsArguments.source)) {
            LOG_ERROR("SetRequestSource error in ToBreakpointLocationsRequest, json[\"arguments\"][\"source\"] is:",
                json["arguments"]["source"]);
            return nullptr;
        }
        if (!HasValue(json["arguments"], "line")) {
            LOG_ERROR("not find line in json, json[\"arguments\"] is:", json["arguments"]);
            return nullptr;
        }
        breakpointLocationsArguments.line = json["arguments"]["line"];
        SetRequestField(json["arguments"], "column", breakpointLocationsArguments.column);
        SetRequestField(json["arguments"], "endLine", breakpointLocationsArguments.endLine);
        SetRequestField(json["arguments"], "endColumn", breakpointLocationsArguments.endColumn);
        SetRequestField(json["arguments"], "startAddress", breakpointLocationsArguments.startAddress);
        SetRequestField(json["arguments"], "filterStepinLocation", breakpointLocationsArguments.filterStepinLocation);
        requestPointer->arguments = std::move(breakpointLocationsArguments);
    }
    return requestPointer;
}

FunctionBreakpointData RequestManager::SetFunctionBreakpointData(const json_t &dataJson)
{
    FunctionBreakpointData functionBreakpointData;
    SetRequestField(dataJson["data"], "supportsConditionalBreakpoints",
        functionBreakpointData.supportsConditionalBreakpoints);
    SetRequestField(dataJson["data"], "supportsHitConditionalBreakpoints",
        functionBreakpointData.supportsHitConditionalBreakpoints);
    SetRequestField(dataJson["data"], "supportsLogPoints", functionBreakpointData.supportsLogPoints);
    SetRequestField(dataJson["data"], "supportsFunctionBreakpoints",
        functionBreakpointData.supportsFunctionBreakpoints);
    SetRequestField(dataJson["data"], "supportsDataBreakpoints", functionBreakpointData.supportsDataBreakpoints);
    SetRequestField(dataJson["data"], "id", functionBreakpointData.id);
    SetRequestField(dataJson["data"], "verified", functionBreakpointData.verified);
    SetRequestField(dataJson["data"], "line", functionBreakpointData.line);
    SetRequestField(dataJson["data"], "sessionId", functionBreakpointData.sessionId);
    return functionBreakpointData;
}

std::unique_ptr<Request> RequestManager::ToDataBreakpointInfoRequest(const json_t &json)
{
    std::unique_ptr<DataBreakpointInfoRequest> requestPointer = std::make_unique<DataBreakpointInfoRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToDataBreakpointInfoRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "name")) {
        LOG_ERROR("not find arguments or name in json, json is:", json);
        return nullptr;
    }
    DataBreakpointInfoArguments dataBreakpointInfoArguments;
    dataBreakpointInfoArguments.name = json["arguments"]["name"];
    SetRequestField(json["arguments"], "variablesReference", dataBreakpointInfoArguments.variablesReference);
    requestPointer->arguments = std::move(dataBreakpointInfoArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetDataBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetDataBreakpointsRequest> requestPointer = std::make_unique<SetDataBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSetDataBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "breakpoints")) {
        LOG_ERROR("not find arguments or breakpoints in json, json is:", json);
        return nullptr;
    }
    SetDataBreakpointsArguments setDataBreakpointsArguments;
    std::vector<DataBreakpoint> dataBreakpoints;
    for (auto it = json["arguments"]["breakpoints"].begin(); it != json["arguments"]["breakpoints"].end(); ++it) {
        DataBreakpoint dataBreakpoint;
        if (!HasValue(it.value(), "dataId")) {
            LOG_ERROR("not find dataId in json, json is:", it.value());
            return nullptr;
        }
        dataBreakpoint.dataId = it.value()["dataId"];
        if (HasValue(it.value(), "accessType")) {
            dataBreakpoint.accessType = STR_TO_ENUM<DataBreakpointAccessType>(it.value()["accessType"]).value();
        }
        SetRequestField(it.value(), "condition", dataBreakpoint.condition);
        SetRequestField(it.value(), "hitCondition", dataBreakpoint.hitCondition);
        dataBreakpoints.push_back(std::move(dataBreakpoint));
    }
    setDataBreakpointsArguments.breakpoints = std::move(dataBreakpoints);
    requestPointer->arguments = std::move(setDataBreakpointsArguments);
    return requestPointer;
}

bool RequestManager::SetFilter(const json_t &json, std::vector<ExceptionFilterOptions> exVec)
{
    for (auto it = json["arguments"]["filterOptions"].begin(); it != json["arguments"]["filterOptions"].end(); ++it) {
        ExceptionFilterOptions exceptionFilterOptions;
        if (!HasValue(it.value(), "filterId")) {
            LOG_ERROR("not find filterId in json, json is:", it.value());
            return false;
        }
        exceptionFilterOptions.filterId = it.value()["filterId"];
        SetRequestField(it.value(), "condition", exceptionFilterOptions.condition);
        exVec.push_back(exceptionFilterOptions);
    }
    return true;
}

bool RequestManager::SetExceptionOptions(const json_t &json, std::vector<ExceptionOptions> /* excVec */)
{
    for (auto it = json["arguments"]["exceptionOptions"].begin(); it != json["arguments"]["exceptionOptions"].end();
        ++it) {
        ExceptionOptions exceptionOptions;
        if (!HasValue(it.value(), "breakMode")) {
            LOG_ERROR("not find breakMode in json, json is:", it.value());
            return false;
        }
        // the depth of the method MIOutputParser::TranslateCString() reduced to 4 using guard code
        if (!HasValue(it.value(), "path")) {
            continue;
        }
        std::vector<ExceptionPathSegment> path;
        for (auto jt = it.value()["path"].begin(); jt != it.value()["path"].end(); ++jt) {
            ExceptionPathSegment exceptionPathSegment;
            if (!HasValue(jt.value(), "names")) {
                LOG_ERROR("not find names in json, json is:", jt.value());
                return false;
            }
            std::vector<std::string> names;
            for (unsigned i = 0; i < jt.value()["names"].size(); i++) {
                names.push_back(jt.value()["names"].at(i));
            }
            exceptionPathSegment.names = names;
            SetRequestField(jt.value(), "negate", exceptionPathSegment.negate);
            path.push_back(exceptionPathSegment);
        }
        exceptionOptions.path = path;
    }
    return true;
}

std::unique_ptr<Request> RequestManager::ToSetExceptionBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetExceptionBreakpointsRequest> requestPointer =
        std::make_unique<SetExceptionBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in SetExceptionBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "filters")) {
        LOG_ERROR("not find arguments or filters in json, json is:", json);
        return nullptr;
    }
    SetExceptionBreakpointsArguments setExceptionBreakpointsArguments;
    setExceptionBreakpointsArguments.filters.clear();
    for (unsigned i = 0; i < json["arguments"]["filters"].size(); i++) {
        setExceptionBreakpointsArguments.filters.push_back(json["arguments"]["filters"].at(i));
    }
    if (HasValue(json["arguments"], "filterOptions")) {
        std::vector<ExceptionFilterOptions> exVec;
        if (!SetFilter(json, exVec)) {
            LOG_ERROR("filters error!");
            return nullptr;
        }
        setExceptionBreakpointsArguments.filterOptions = std::move(exVec);
    }
    if (HasValue(json["arguments"], "exceptionOptions")) {
        std::vector<ExceptionOptions> excVec;
        if (!SetExceptionOptions(json, excVec)) {
            LOG_ERROR("exceptionOptions error!");
            return nullptr;
        }
        setExceptionBreakpointsArguments.exceptionOptions = std::move(excVec);
    }
    requestPointer->arguments = std::move(setExceptionBreakpointsArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetInstructionBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetInstructionBreakpointsRequest> requestPointer =
        std::make_unique<SetInstructionBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSetInstructionBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "breakpoints")) {
        LOG_ERROR("not find arguments or breakpoints in json, json is:", json);
        return nullptr;
    }
    SetInstructionBreakpointsArguments setInstructionBreakpointsArguments;
    std::vector<InstructionBreakpoint> breakpoints;
    for (auto it = json["arguments"]["breakpoints"].begin(); it != json["arguments"]["breakpoints"].end(); ++it) {
        InstructionBreakpoint instructionBreakpoint;
        if (!HasValue(it.value(), "instructionReference")) {
            LOG_ERROR("not find instructionReference in instructionBreakpoint json, json is:", it.value());
            continue;
        };
        instructionBreakpoint.instructionReference = it.value()["instructionReference"];
        SetRequestField(it.value(), "offset", instructionBreakpoint.offset);
        SetRequestField(it.value(), "condition", instructionBreakpoint.condition);
        SetRequestField(it.value(), "hitCondition", instructionBreakpoint.hitCondition);
        breakpoints.push_back(std::move(instructionBreakpoint));
    }
    setInstructionBreakpointsArguments.breakpoints = std::move(breakpoints);
    requestPointer->arguments = std::move(setInstructionBreakpointsArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetFunctionBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetFunctionBreakpointsRequest> requestPointer = std::make_unique<SetFunctionBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSetFunctionBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "breakpoints")) {
        LOG_ERROR("not find arguments or breakpoints in json, json is:", json);
        return nullptr;
    }
    SetFunctionBreakpointsArguments setFunctionBreakpointsArguments;
    std::vector<FunctionBreakpoint> breakpoints;
    for (auto it = json["arguments"]["breakpoints"].begin(); it != json["arguments"]["breakpoints"].end(); ++it) {
        FunctionBreakpoint functionBreakpoint;
        if (!HasValue(it.value(), "name")) {
            LOG_ERROR("not find name in functionBreakpoint json, json is:", it.value());
            continue;
        }
        functionBreakpoint.name = it.value()["name"];
        SetRequestField(it.value(), "condition", functionBreakpoint.condition);
        SetRequestField(it.value(), "hitCondition", functionBreakpoint.hitCondition);
        SetRequestField(it.value(), "enabled", functionBreakpoint.enabled);
        SetRequestField(it.value(), "id", functionBreakpoint.id);
        SetRequestField(it.value(), "sessionData", functionBreakpoint.sessionData);
        SetRequestField(it.value(), "hitOnce", functionBreakpoint.hitOnce);
        SetRequestField(it.value(), "isRegex", functionBreakpoint.isRegex);
        SetRequestField(it.value(), "moduleName", functionBreakpoint.moduleName);
        if (HasValue(it.value(), "data")) {
            FunctionBreakpointData functionBreakpointData = SetFunctionBreakpointData(it.value());
            functionBreakpoint.data = std::move(functionBreakpointData);
        }
        breakpoints.push_back(std::move(functionBreakpoint));
    }
    setFunctionBreakpointsArguments.breakpoints = std::move(breakpoints);
    requestPointer->arguments = std::move(setFunctionBreakpointsArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToContinueRequest(const json_t &json)
{
    std::unique_ptr<ContinueRequest> requestPointer = std::make_unique<ContinueRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToContinueRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    ContinueArguments continueArguments;
    continueArguments.threadId = json["arguments"]["threadId"];
    requestPointer->arguments = std::move(continueArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToPauseRequest(const json_t &json)
{
    std::unique_ptr<PauseRequest> requestPointer = std::make_unique<PauseRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToPauseRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    PauseArguments pauseArguments;
    pauseArguments.threadId = json["arguments"]["threadId"];
    requestPointer->arguments = std::move(pauseArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToNextRequest(const json_t &json)
{
    std::unique_ptr<NextRequest> requestPointer = std::make_unique<NextRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToNextRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    NextArguments nextArguments;
    nextArguments.threadId = json["arguments"]["threadId"];
    if (HasValue(json["arguments"], "granularity")) {
        nextArguments.granularity = STR_TO_ENUM<SteppingGranularity>(json["arguments"]["granularity"]).value();
    }
    requestPointer->arguments = std::move(nextArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToStepInRequest(const json_t &json)
{
    std::unique_ptr<StepInRequest> requestPointer = std::make_unique<StepInRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToStepInRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    StepInArguments stepInArguments;
    stepInArguments.threadId = json["arguments"]["threadId"];
    SetRequestField(json["arguments"], "targetId", stepInArguments.targetId);
    if (HasValue(json["arguments"], "granularity")) {
        stepInArguments.granularity = STR_TO_ENUM<SteppingGranularity>(json["arguments"]["granularity"]).value();
    }
    requestPointer->arguments = std::move(stepInArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToStepOutRequest(const json_t &json)
{
    std::unique_ptr<StepOutRequest> requestPointer = std::make_unique<StepOutRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToStepOutRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    StepOutArguments stepOutArguments;
    stepOutArguments.threadId = json["arguments"]["threadId"];
    if (HasValue(json["arguments"], "granularity")) {
        stepOutArguments.granularity = STR_TO_ENUM<SteppingGranularity>(json["arguments"]["granularity"]).value();
    }
    requestPointer->arguments = std::move(stepOutArguments);
    return requestPointer;
}

bool RequestManager::HasValue(const json_t &json, const std::string &key)
{
    return json.contains(key) && !json[key].is_null();
}

std::unique_ptr<Request> RequestManager::ToStackTraceRequest(const json_t &json)
{
    std::unique_ptr<StackTraceRequest> requestPointer = std::make_unique<StackTraceRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to threads request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    StackTraceArguments stackTraceArguments;
    stackTraceArguments.threadId = json["arguments"]["threadId"];
    SetRequestField(json["arguments"], "startFrame", stackTraceArguments.startFrame);
    SetRequestField(json["arguments"], "levels", stackTraceArguments.levels);
    if (HasValue(json["arguments"], "format")) {
        StackFrameFormat stackFrameFormat;
        SetRequestField(json["arguments"]["format"], "parameters", stackFrameFormat.parameters);
        SetRequestField(json["arguments"]["format"], "parameterTypes", stackFrameFormat.parameterTypes);
        SetRequestField(json["arguments"]["format"], "parameterNames", stackFrameFormat.parameterNames);
        SetRequestField(json["arguments"]["format"], "parameterValues", stackFrameFormat.parameterValues);
        SetRequestField(json["arguments"]["format"], "line", stackFrameFormat.line);
        SetRequestField(json["arguments"]["format"], "module", stackFrameFormat.module);
        SetRequestField(json["arguments"]["format"], "includeAll", stackFrameFormat.includeAll);
        stackTraceArguments.format = std::move(stackFrameFormat);
    }
    requestPointer->arguments = std::move(stackTraceArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToThreadsRequest(const json_t &json)
{
    std::unique_ptr<ThreadsRequest> requestPointer = std::make_unique<ThreadsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to threads request failed in json, json is:", json);
        return nullptr;
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToScopesRequest(const json_t &json)
{
    std::unique_ptr<ScopesRequest> requestPointer = std::make_unique<ScopesRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to scopes request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "frameId")) {
        LOG_ERROR("not find arguments or frameId in json, json is:", json);
        return nullptr;
    }
    ScopesArguments scopesArguments;
    scopesArguments.frameId = json["arguments"]["frameId"];
    requestPointer->arguments = std::move(scopesArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToVariablesRequest(const json_t &json)
{
    std::unique_ptr<VariablesRequest> requestPointer = std::make_unique<VariablesRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to variables request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "variablesReference")) {
        LOG_ERROR("not find arguments or variables in json, json is:", json);
        return nullptr;
    }
    requestPointer->arguments = GetVariablesArguments(json);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToVariablesExpandRequest(const json_t &json)
{
    std::unique_ptr<VariablesExpandRequest> requestPointer = std::make_unique<VariablesExpandRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to expand variables request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "variablesReference")) {
        LOG_ERROR("not find arguments or variables in json, json is:", json);
        return nullptr;
    }
    requestPointer->arguments = GetVariablesArguments(json);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToPreVariables(const json_t &json)
{
    std::unique_ptr<PreVariablesRequest> requestPointer = std::make_unique<PreVariablesRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to pre variables request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "variablesReference")) {
        LOG_ERROR("not find arguments or variables in json, json is:", json);
        return nullptr;
    }
    requestPointer->arguments = GetVariablesArguments(json);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetVariableRequest(const json_t &json)
{
    std::unique_ptr<SetVariableRequest> requestPtr = std::make_unique<SetVariableRequest>();
    if (!SetRequestBaseInfo(*requestPtr, json)) {
        LOG_ERROR("to setVariable request failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "variablesReference") ||
        !HasValue(json["arguments"], "name") || !HasValue(json["arguments"], "value")) {
        LOG_ERROR("not find necessary arguments in json, json is:", json);
        return nullptr;
    }
    SetVariableArguments setVar;
    setVar.variablesReference = json["arguments"]["variablesReference"];
    setVar.name = json["arguments"]["name"];
    setVar.value = json["arguments"]["value"];
    if (HasValue(json["arguments"], "format")) {
        ValueFormat vf;
        SetRequestField(json["arguments"]["format"], "hex", vf.hex);
        setVar.format = std::move(vf);
    }
    SetRequestField(json["type"], "type", requestPtr->type);
    requestPtr->arguments = setVar;
    return requestPtr;
}

void RequestManager::SetRequestJsonBaseInfo(const Request &request, json_t &json)
{
    json["type"] = REQUEST_NAME;
    json["seq"] = request.seq;
    json["command"] = request.command;
}

std::optional<json_t> RequestManager::ToRunInTerminalJson(const Request &request)
{
    json_t json;
    SetRequestJsonBaseInfo(request, json);
    json_t argumentsJson = json_t::object();
    RunInTerminalRequestArguments runInTerminalRequestArguments = ((RunInTerminalRequest &)request).arguments;
    argumentsJson["cwd"] = runInTerminalRequestArguments.cwd;
    json_t argsArray = json_t::array();
    for (std::string arg : runInTerminalRequestArguments.args) {
        argsArray.push_back(arg);
    }
    argumentsJson["args"] = argsArray;
    if (runInTerminalRequestArguments.kind.has_value()) {
        argumentsJson["kind"] = ENUM_TO_STR(runInTerminalRequestArguments.kind.value()).value();
    }
    if (runInTerminalRequestArguments.title.has_value()) {
        argumentsJson["title"] = runInTerminalRequestArguments.title.value();
    }
    if (runInTerminalRequestArguments.env.has_value()) {
        json_t env = json_t::object();
        for (auto envPair : runInTerminalRequestArguments.env.value()) {
            env[envPair.key] = envPair.value;
        }
        argumentsJson["env"] = env;
    }
    json["arguments"] = argumentsJson;
    return json;
}

std::unique_ptr<Request> RequestManager::ToDisassembleRequest(const json_t &json)
{
    std::unique_ptr<DisassembleRequest> requestPointer = std::make_unique<DisassembleRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to DisassembleRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in DisassembleRequest json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "memoryReference") || !HasValue(json["arguments"], "instructionCount")) {
        LOG_ERROR("not find some necessary arguments in DisassembleRequest json, json is:", json);
        return nullptr;
    }

    DisassembleArguments disassembleArguments;
    disassembleArguments.memoryReference = json["arguments"]["memoryReference"];
    disassembleArguments.instructionCount = json["arguments"]["instructionCount"];
    SetRequestField(json["arguments"], "instructionOffset", disassembleArguments.instructionOffset);
    SetRequestField(json["arguments"], "offset", disassembleArguments.offset);
    SetRequestField(json["arguments"], "resolveSymbols", disassembleArguments.resolveSymbols);

    requestPointer->arguments = std::move(disassembleArguments);
    return requestPointer;
}

VariablesArguments RequestManager::GetVariablesArguments(const json_t &json)
{
    VariablesArguments variablesArguments;
    variablesArguments.variablesReference = json["arguments"]["variablesReference"];
    SetRequestField(json["arguments"], "start", variablesArguments.start);
    SetRequestField(json["arguments"], "count", variablesArguments.count);
    SetConvertedRequestField(json["arguments"], "filter", variablesArguments.filter,
        [&json]() { return STR_TO_ENUM<Filter>(json["arguments"]["filter"]); });
    SetConvertedRequestField(json["arguments"], "format", variablesArguments.format,
        [&json]() { return STR_TO_ENUM<ValueFormat>(json["arguments"]["format"]); });
    return variablesArguments;
}

std::unique_ptr<Request> RequestManager::ToEvaluateRequest(const json_t &json)
{
    std::unique_ptr<EvaluateRequest> requestPointer = std::make_unique<EvaluateRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to EvaluateRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in EvaluateRequest json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "expression")) {
        LOG_ERROR("not find some necessary arguments in EvaluateRequest json, json is:", json);
        return nullptr;
    }
    EvaluateArguments arguments;
    arguments.expression = json["arguments"]["expression"];
    SetRequestField(json["arguments"], "frameId", arguments.frameId);
    if (json["arguments"].contains("context")) {
        arguments.context = STR_TO_ENUM<Context>(json["arguments"]["context"]);
    }
    if (json["arguments"].contains("format") && json["arguments"]["format"].contains("hex")) {
        ValueFormat format;
        SetRequestField(json["arguments"]["format"], "hex", format.hex);
        arguments.format = format;
    }
    SetRequestField(json["arguments"], "isViewMore", arguments.isViewMore);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToDebugInConsoleRequest(const json_t &json)
{
    std::unique_ptr<DebugInConsoleRequest> requestPointer = std::make_unique<DebugInConsoleRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to DebugInConsoleRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in DebugInConsoleRequest json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "debugCommand")) {
        LOG_ERROR("not find some necessary arguments in DebugInConsoleRequest json, json is:", json);
        return nullptr;
    }

    DebugInConsoleArguments debugInConsoleArguments;
    debugInConsoleArguments.debugCommand = json["arguments"]["debugCommand"];

    requestPointer->arguments = std::move(debugInConsoleArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSourceRequest(const json_t &json)
{
    std::unique_ptr<SourceRequest> requestPointer = std::make_unique<SourceRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSourceRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "sourceReference")) {
        LOG_ERROR("not found arguments or sourceReference in json, json is:", json);
        return nullptr;
    }
    SourceArguments sourceArguments;
    sourceArguments.sourceReference = json["arguments"]["sourceReference"];
    if (HasValue(json["arguments"], "source")) {
        Source source;
        if (!SetRequestSource(json["arguments"]["source"], source)) {
            LOG_ERROR("SetRequestSource error in ToSourceRequest, json[\"arguments\"][\"source\"] is:",
                      json["arguments"]["source"]);
            return nullptr;
        }
        sourceArguments.source = std::move(source);
    }
    requestPointer->arguments = std::move(sourceArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToReadMemoryRequest(const json_t &json)
{
    std::unique_ptr<ReadMemoryRequest> requestPointer = std::make_unique<ReadMemoryRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to ReadMemoryRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in ReadMemoryRequest json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "memoryReference") || !HasValue(json["arguments"], "count")) {
        LOG_ERROR("not find some necessary arguments in ReadMemoryRequest json, json is:", json);
        return nullptr;
    }

    ReadMemoryArguments readMemoryArguments;
    readMemoryArguments.memoryReference = json["arguments"]["memoryReference"];
    readMemoryArguments.count = json["arguments"]["count"];
    SetRequestField(json["arguments"], "offset", readMemoryArguments.offset);

    requestPointer->arguments = std::move(readMemoryArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToWriteMemoryRequest(const json_t &json)
{
    std::unique_ptr<WriteMemoryRequest> requestPointer = std::make_unique<WriteMemoryRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to WriteMemoryRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in WriteMemoryRequest json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "memoryReference") || !HasValue(json["arguments"], "data")) {
        LOG_ERROR("not find some necessary arguments in WriteMemoryRequest json, json is:", json);
        return nullptr;
    }

    WriteMemoryArguments writeMemoryArguments;
    writeMemoryArguments.memoryReference = json["arguments"]["memoryReference"];
    writeMemoryArguments.data = json["arguments"]["data"];
    SetRequestField(json["arguments"], "offset", writeMemoryArguments.offset);
    SetRequestField(json["arguments"], "allowPartial", writeMemoryArguments.allowPartial);

    requestPointer->arguments = std::move(writeMemoryArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToReverseStepRequest(const json_t &json)
{
    std::unique_ptr<ReverseStepRequest> requestPointer = std::make_unique<ReverseStepRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToReverseStepRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    ReverseStepArguments reverseStepArguments;
    reverseStepArguments.threadId = json["arguments"]["threadId"];
    if (HasValue(json["arguments"], "granularity")) {
        reverseStepArguments.granularity =
                STR_TO_ENUM<SteppingGranularity>(json["arguments"]["granularity"]).value();
    }
    requestPointer->arguments = std::move(reverseStepArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToUndoRequest(const json_t &json)
{
    std::unique_ptr<UndoRequest> requestPointer = std::make_unique<UndoRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in UndoRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "undoEnable")) {
        LOG_ERROR("not find arguments or undoEnable in json, json is:", json);
        return nullptr;
    }
    UndoArguments undoArguments;
    undoArguments.undoEnable = json["arguments"]["undoEnable"];
    requestPointer->arguments = std::move(undoArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToReverseContinueRequest(const json_t &json)
{
    std::unique_ptr<ReverseContinueRequest> requestPointer = std::make_unique<ReverseContinueRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToReverseContinueRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    ReverseContinueArguments reverseContinueArguments;
    reverseContinueArguments.threadId = json["arguments"]["threadId"];
    requestPointer->arguments = std::move(reverseContinueArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetExecutionPointRequest(const json_t &json)
{
    std::unique_ptr<SetExecutionPointRequest> requestPointer = std::make_unique<SetExecutionPointRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to SetExecutionPointRequest failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments")) {
        LOG_ERROR("not find arguments in SetExecutionPointRequest json, json is:", json);
        return nullptr;
    }

    SetExecutionPointArguments setExecutionPointArguments;
    SetRequestField(json["arguments"], "sourcePath", setExecutionPointArguments.sourcePath);
    SetRequestField(json["arguments"], "line", setExecutionPointArguments.line);
    SetRequestField(json["arguments"], "instructionReference", setExecutionPointArguments.instructionReference);

    requestPointer->arguments = std::move(setExecutionPointArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToRecordPointsRequest(const json_t &json)
{
    std::unique_ptr<RecordPointsRequest> requestPointer = std::make_unique<RecordPointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to RecordPointsRequest failed in json,json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments")) {
        RecordPointsArguments recordPointsArguments;
        if (HasValue(json["arguments"], "filters")) {
            for (auto str : json["arguments"]["filters"]) {
                recordPointsArguments.filters.push_back(str);
            }
        }
        SetRequestField(json["arguments"], "start", recordPointsArguments.start);
        SetRequestField(json["arguments"], "count", recordPointsArguments.count);
        requestPointer->arguments = recordPointsArguments;
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToGotoRecordPointRequest(const json_t &json)
{
    std::unique_ptr<GotoRecordPointRequest> requestPointer = std::make_unique<GotoRecordPointRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to GotoEventRequest failed in json,json is:", json);
        return nullptr;
    }
    if (HasValue(json, "arguments") && HasValue(json["arguments"], "recordPointId")) {
        requestPointer->arguments.recordPointId = json["arguments"]["recordPointId"];
    }
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToContinueInReverseRequest(const json_t &json)
{
    std::unique_ptr<ContinueInReverseRequest> requestPointer = std::make_unique<ContinueInReverseRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ContinueInReverseRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    ContinueInReverseArguments continueInReverseArguments;
    continueInReverseArguments.threadId = json["arguments"]["threadId"];
    requestPointer->arguments = std::move(continueInReverseArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToStepInReverseRequest(const json_t &json)
{
    std::unique_ptr<StepInReverseRequest> requestPointer = std::make_unique<StepInReverseRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in StepInReverseRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "threadId")) {
        LOG_ERROR("not find arguments or threadId in json, json is:", json);
        return nullptr;
    }
    StepInReverseArguments stepInReverseArguments;
    stepInReverseArguments.threadId = json["arguments"]["threadId"];
    requestPointer->arguments = std::move(stepInReverseArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToCacheDataConfigRequest(const json_t &json)
{
    std::unique_ptr<CacheDataConfigRequest> requestPointer = std::make_unique<CacheDataConfigRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to CacheDataConfigRequest failed in json,json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "startCacheData")) {
        LOG_ERROR("not find arguments or startCacheData in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "saveHistoryCache")) {
        LOG_ERROR("not find arguments or saveHistoryCache in json, json is:", json);
        return nullptr;
    }
    CacheDataConfigArguments cacheDataConfigArguments;
    cacheDataConfigArguments.startCacheData = json["arguments"]["startCacheData"];
    cacheDataConfigArguments.saveHistoryCache = json["arguments"]["saveHistoryCache"];
    SetRequestField(json["arguments"], "threadCount", cacheDataConfigArguments.threadCount);
    SetRequestField(json["arguments"], "stackFrameCount", cacheDataConfigArguments.stackFrameCount);
    SetRequestField(json["arguments"], "variableChildrenCount", cacheDataConfigArguments.variableChildrenCount);
    SetRequestField(json["arguments"], "variableChildrenLayers", cacheDataConfigArguments.variableChildrenLayers);
    SetRequestField(json["arguments"], "stackTracePageSize", cacheDataConfigArguments.stackTracePageSize);
    SetRequestField(json["arguments"], "variablePageSize", cacheDataConfigArguments.variablePageSize);
    SetRequestField(json["arguments"], "scopeType", cacheDataConfigArguments.scopeType);
    requestPointer->arguments = std::move(cacheDataConfigArguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToRecordMessageRequest(const json_t &json)
{
    std::unique_ptr<RecordMessageRequest> requestPointer = std::make_unique<RecordMessageRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("to RecordMessageRequest failed in json,json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "recordPointIds") ||
        !json["arguments"]["recordPointIds"].is_array()) {
        LOG_ERROR("not find recordPointIds in json, json is:", json);
        return nullptr;
    }
    RecordMessageArguments arguments;
    for (const auto &it : json["arguments"]["recordPointIds"]) {
        arguments.recordPointIds.push_back(it);
    }
    SetRequestField(json["arguments"], "requireThread", arguments.requireThread);
    SetRequestField(json["arguments"], "requireStackFrame", arguments.requireStackFrame);
    SetRequestField(json["arguments"], "requireLocalVariables", arguments.requireLocalVariables);
    SetRequestField(json["arguments"], "requireGlobalVariables", arguments.requireGlobalVariables);
    SetRequestField(json["arguments"], "requireStaticVariables", arguments.requireStaticVariables);
    if (HasValue(json["arguments"], "monitorVariables") && json["arguments"]["monitorVariables"].is_array()) {
        std::vector<MonitorVariable> monitorVariables;
        for (auto it : json["arguments"]["monitorVariables"]) {
            MonitorVariable monitorVariable;
            SetRequestField(it, "id", monitorVariable.id);
            SetRequestField(it, "name", monitorVariable.name);
            SetRequestField(it, "scopeName", monitorVariable.scopeName);
            SetRequestField(it, "type", monitorVariable.type);
            SetRequestField(it, "value", monitorVariable.value);
            monitorVariables.push_back(monitorVariable);
        }
        arguments.monitorVariables = std::move(monitorVariables);
    }
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToSetMemoryBreakpointsRequest(const json_t &json)
{
    std::unique_ptr<SetMemoryBreakpointsRequest> requestPointer = std::make_unique<SetMemoryBreakpointsRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSetMemoryBreakpointsRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "breakpoints")) {
        LOG_ERROR("not find arguments or breakpoints in json, json is:", json);
        return nullptr;
    }
    SetMemoryBreakpointsArguments arguments;
    std::vector<MemoryBreakpoint> memoryBreakpoints;
    for (auto it = json["arguments"]["breakpoints"].begin(); it != json["arguments"]["breakpoints"].end(); ++it) {
        MemoryBreakpoint breakpoint;
        if (!HasValue(it.value(), "address")) {
            LOG_ERROR("not find address in json, json is:", it.value());
            return nullptr;
        }
        if (!HasValue(it.value(), "accessType")) {
            LOG_ERROR("not find accessType in json, json is:", it.value());
            return nullptr;
        }
        breakpoint.address = it.value()["address"];
        breakpoint.accessType = STR_TO_ENUM<DataBreakpointAccessType>(it.value()["accessType"]).value();
        memoryBreakpoints.push_back(std::move(breakpoint));
    }
    arguments.breakpoints = std::move(memoryBreakpoints);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToGetBitmapRequest(const json_t &json)
{
    std::unique_ptr<GetBitmapRequest> requestPointer = std::make_unique<GetBitmapRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToGetBitmapReqeust, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "varName")) {
        LOG_ERROR("not find arguments or varName in json, json is:", json);
        return nullptr;
    }
    GetBitmapArguments arguments;
    arguments.varName = json["arguments"]["varName"];
    SetRequestField(json["arguments"], "frameId", arguments.frameId);
    SetRequestField(json["arguments"], "varAddress", arguments.varAddress);
    SetRequestField(json["arguments"], "varType", arguments.varType);
    SetRequestField(json["arguments"], "hasData", arguments.hasData);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToLoadModulesRequest(const json_t &json)
{
    std::unique_ptr<LoadModuleRequest> requestPointer = std::make_unique<LoadModuleRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToLoadModulesRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "pathList")) {
        LOG_ERROR("not find arguments or symbol pathList in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "uuidList")) {
        LOG_ERROR("not find arguments or uuidList in json, json is:", json);
        return nullptr;
    }
    LoadModulesArguments arguments;
    std::vector<std::string> pathList;
    for (auto it = json["arguments"]["pathList"].begin(); it != json["arguments"]["pathList"].end(); ++it) {
        pathList.push_back(it.value());
    }
    std::vector<std::string> uuidList;
    for (auto it = json["arguments"]["uuidList"].begin(); it != json["arguments"]["uuidList"].end(); ++it) {
        uuidList.push_back(it.value());
    }
    arguments.pathList = std::move(pathList);
    arguments.uuidList = std::move(uuidList);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToAddSourceMappingRequest(const json_t &json)
{
    std::unique_ptr<AddSourceMappingRequest> requestPointer = std::make_unique<AddSourceMappingRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToSourceMappingRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "targetSourcePathList")) {
        LOG_ERROR("not find arguments or target source path list in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "moduleIdList")) {
        LOG_ERROR("not find arguments or moduleIdList in json, json is:", json);
        return nullptr;
    }
    AddSourceMappingArguments arguments;
    std::vector<std::string> pathList;
    auto begin = json["arguments"]["targetSourcePathList"].begin();
    auto end = json["arguments"]["targetSourcePathList"].end();
    for (auto it = begin; it != end; ++it) {
        pathList.push_back(it.value());
    }
    std::vector<int32_t> moduleIdList;
    for (auto it = json["arguments"]["moduleIdList"].begin(); it != json["arguments"]["moduleIdList"].end(); ++it) {
        moduleIdList.push_back(it.value());
    }
    arguments.targetSourcePathList = std::move(pathList);
    arguments.moduleIdList = std::move(moduleIdList);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToResetSourceMappingRequest(const json_t &json)
{
    std::unique_ptr<ResetSourceMappingRequest> requestPointer = std::make_unique<ResetSourceMappingRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToRemoveSourceMappingRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "sourceRootDirList")) {
        LOG_ERROR("not find arguments or target source root directory list in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "moduleIdList")) {
        LOG_ERROR("not find arguments or moduleIdList in json, json is:", json);
        return nullptr;
    }
    ResetSourceMappingArguments arguments;
    std::vector<std::string> pathList;
    auto begin = json["arguments"]["sourceRootDirList"].begin();
    auto end = json["arguments"]["sourceRootDirList"].end();
    for (auto it = begin; it != end; ++it) {
        pathList.push_back(it.value());
    }
    std::vector<int32_t> moduleIdList;
    for (auto it = json["arguments"]["moduleIdList"].begin(); it != json["arguments"]["moduleIdList"].end(); ++it) {
        moduleIdList.push_back(it.value());
    }
    arguments.sourceRootDirList = std::move(pathList);
    arguments.moduleIdList = std::move(moduleIdList);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToAddSourceBreakpointRequest(const json_t &json)
{
    std::unique_ptr<AddSourceBreakpointRequest> requestPointer = std::make_unique<AddSourceBreakpointRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToAddSourceBreakpointRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "source")) {
        LOG_ERROR("not found arguments or source in json, json is:", json);
        return nullptr;
    }
    AddSourceBreakpointArguments arguments;
    if (!SetRequestSource(json["arguments"]["source"], arguments.source)) {
        LOG_ERROR("SetRequestSource error in ToAddSourceBreakpointRequest, json[\"arguments\"][\"source\"] is:",
            json["arguments"]["source"]);
        return nullptr;
    }
    if (!HasValue(json["arguments"], "line")) {
        LOG_ERROR("not find line in arguments, json is:", json);
        return nullptr;
    }
    arguments.line = json["arguments"]["line"];
    SetRequestField(json["arguments"], "column", arguments.column);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToDeleteBreakpointRequest(const json_t &json)
{
    std::unique_ptr<DeleteBreakpointRequest> requestPointer = std::make_unique<DeleteBreakpointRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToAddSourceBreakpointRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "id")) {
        LOG_ERROR("not found arguments or id in json, json is:", json);
        return nullptr;
    }
    DeleteBreakpointArguments arguments;
    arguments.id = json["arguments"]["id"];
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToVariableHexViewRequest(const json_t &json)
{
    std::unique_ptr<VariableHexViewRequest> requestPointer = std::make_unique<VariableHexViewRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToVariableHexViewRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "isEnable")) {
        LOG_ERROR("Not find arguments or enable flag in json, json is:", json);
        return nullptr;
    }
    VariableHexViewArguments arguments;
    arguments.isEnable = json["arguments"]["isEnable"];
    if (HasValue(json["arguments"], "formats")) {
        for (auto it = json["arguments"]["formats"].begin(); it != json["arguments"]["formats"].end(); ++it) {
            arguments.formats.push_back(it.value());
        }
    }
    SetRequestField(json["arguments"], "isArrayView", arguments.isArrayView);
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}

std::unique_ptr<Request> RequestManager::ToCommandCompletionRequest(const json_t &json)
{
    std::unique_ptr<CommandCompletionRequest> requestPointer = std::make_unique<CommandCompletionRequest>();
    if (!SetRequestBaseInfo(*requestPointer, json)) {
        LOG_ERROR("SetRequestBaseInfo error in ToCommandCompletionRequest, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "arguments") || !HasValue(json["arguments"], "command")) {
        LOG_ERROR("Not find arguments or command in json, json is:", json);
        return nullptr;
    }
    CommandCompletionArguments arguments;
    arguments.command = json["arguments"]["command"];
    requestPointer->arguments = std::move(arguments);
    return requestPointer;
}
