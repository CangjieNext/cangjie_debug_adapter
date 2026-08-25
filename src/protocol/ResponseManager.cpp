// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <exception>

#include "ResponseManager.h"
#include "TypeUtils.h"

using namespace Dap::Protocol;

std::optional<ResponseManager::ResponseToJsonFunc> ResponseManager::GetResponseJsonFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (responseFactory.count(command) == 0) {
        return std::nullopt;
    }
    return responseFactory[command];
}

std::optional<json_t> ResponseManager::ToJson(const Response &response)
{
    std::string command = response.command;
    std::optional<ResponseManager::ResponseToJsonFunc> func = GetResponseJsonFunc(command);
    if (!func.has_value()) {
        LOG_ERROR("not find target function, response is:", command);
        return std::nullopt;
    }
    try {
        return func.value()(response);
    } catch (const json_t::exception &err) {
        LOG_ERROR("to json failed, json error: ", err.what());
        return std::nullopt;
    } catch (const std::exception &err) {
        LOG_ERROR("to json failed, error: ", err.what());
        return std::nullopt;
    } catch (...) {
        LOG_ERROR("to json failed, unknown json error.");
        return std::nullopt;
    }
}

std::optional<ResponseManager::JsonToResponseFunc> ResponseManager::GetResponseFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (toResponseFactory.count(command) == 0) {
        return std::nullopt;
    }
    return toResponseFactory[command];
}

bool ResponseManager::IsResponse(const json_t &jsonResponse) const
{
    return (jsonResponse.contains("type")) && (jsonResponse["type"] == RESPONSE_NAME);
}

std::string ResponseManager::Command(const json_t &jsonResponse) const
{
    if (jsonResponse.contains("command") && jsonResponse["command"].is_string()) {
        return jsonResponse["command"].get<std::string>();
    }
    return "";
}

std::unique_ptr<Response> ResponseManager::ToResponse(const std::string &responseStr)
{
    std::optional<json_t> responseJson;
    try {
        responseJson = json_t::parse(responseStr);
    } catch (const json_t::parse_error &e) {
        LOG_ERROR("wrong json String, responseJson is:", responseStr);
        return nullptr;
    }
    return ToResponse(responseJson.value());
}

std::unique_ptr<Response> ResponseManager::ToResponse(const json_t &responseJson)
{
    if (!IsResponse(responseJson)) {
        LOG_ERROR("this not response, responseJson is:", responseJson);
        return nullptr;
    }
    std::string command = Command(responseJson);
    std::optional<ResponseManager::JsonToResponseFunc> func = GetResponseFunc(command);
    if (!func.has_value()) {
        LOG_ERROR("not find target function, command is:", command);
        return nullptr;
    }
    return func.value()(responseJson);
}

void ResponseManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (isRegister) {
        return;
    }
    responseFactory.emplace("", ToErrorJson);
    responseFactory.emplace(REQ_RES_INITIALIZE, ToInitializeJson);
    responseFactory.emplace(REQ_RES_CONFIGURATION_DONE, ToConfigurationDoneJson);
    responseFactory.emplace(REQ_RES_LAUNCH, ToLaunchJson);
    responseFactory.emplace(REQ_RES_ATTACH, ToAttachJson);
    responseFactory.emplace(REQ_RES_DISCONNECT, ToDisconnectJson);
    responseFactory.emplace(REQ_RES_TERMINATE, ToTerminateJson);
    responseFactory.emplace(REQ_RES_BREAKPOINT_LOCATIONS, ToBreakpointLocationsJson);
    responseFactory.emplace(REQ_RES_SET_BREAKPOINTS, ToSetBreakpointsJson);
    responseFactory.emplace(REQ_RES_SET_INSTRUCTION_BREAKPOINTS, ToSetInstructionBreakpointsJson);
    responseFactory.emplace(REQ_RES_SET_FUNCTION_BREAKPOINTS, ToSetFunctionBreakpointsJson);
    responseFactory.emplace(REQ_RES_DATA_BREAKPOINT_INFO, ToDataBreakpointInfoJson);
    responseFactory.emplace(REQ_RES_SET_EXCEPTION_BREAKPOINTS, ToSetExceptionBreakpointsJson);
    responseFactory.emplace(REQ_RES_SET_DATA_BREAKPOINTS, ToSetDataBreakpointsJson);
    responseFactory.emplace(REQ_RES_CONTINUE, ToContinueJson);
    responseFactory.emplace(REQ_RES_NEXT, ToNextJson);
    responseFactory.emplace(REQ_RES_STEP_IN, ToStepInJson);
    responseFactory.emplace(REQ_RES_STEP_OUT, ToStepOutJson);
    responseFactory.emplace(REQ_RES_STACK_TRACE, ToStackTraceJson);
    responseFactory.emplace(REQ_RES_THREADS, ToThreadsJson);
    responseFactory.emplace(REQ_RES_SCOPES, ToScopesJson);
    responseFactory.emplace(REQ_RES_VARIABLES, ToVariablesJson);
    responseFactory.emplace(REQ_RES_VARIABLES_EXPAND, ToVariablesJson);
    responseFactory.emplace(REQ_RES_PRE_VARIABLES, ToVariablesJson);
    responseFactory.emplace(REQ_RES_SET_VARIABLE, ToSetVariableJson);
    responseFactory.emplace(REQ_RES_PAUSE, ToPauseJson);
    responseFactory.emplace(REQ_RES_DISASSEMBLE, ToDisassembleJson);
    responseFactory.emplace(REQ_RES_EVALUATE, ToEvaluateJson);
    responseFactory.emplace(REQ_RES_DEBUG_IN_CONSOLE, ToDebugInConsoleJson);
    responseFactory.emplace(REQ_RES_SOURCE, ToSourceJson);
    responseFactory.emplace(REQ_RES_READ_MEMORY, ToReadMemoryJson);
    responseFactory.emplace(REQ_RES_WRITE_MEMORY, ToWriteMemoryJson);
    responseFactory.emplace(REQ_RES_REVERSE_STEP, ToReverseStepJson);
    responseFactory.emplace(REQ_RES_UNDO, ToUndoJson);
    responseFactory.emplace(REQ_RES_REVERSE_CONTINUE, ToReverseContinueJson);
    responseFactory.emplace(REQ_RES_SET_EXECUTION_POINT, ToSetExecutionPointJson);
    responseFactory.emplace(REQ_RES_RECORD_POINTS, ToRecordPointsJson);
    responseFactory.emplace(REQ_RES_GOTO_RECORD_POINT, ToGotoPointJson);
    responseFactory.emplace(REQ_RES_CONTINUE_IN_REVERSE, ToContinueInReverseJson);
    responseFactory.emplace(REQ_RES_STEP_IN_REVERSE, ToStepInReverseJson);
    responseFactory.emplace(REQ_RES_CACHE_DATA_CONFIG, ToCacheDataConfigJson);
    responseFactory.emplace(REQ_RES_RECORD_MESSAGE, ToRecordMessageResponse);
    responseFactory.emplace(REQ_RES_SET_MEMORY_BREAKPOINTS, ToSetMemoryBreakpointsResponse);
    responseFactory.emplace(REQ_RES_GET_BITMAP, ToGetBitmapResponse);
    responseFactory.emplace(REQ_RES_LOAD_MODULES, ToLoadModulesResponse);
    responseFactory.emplace(REQ_RES_ADD_SOURCE_MAPPING, ToSourceMappingResponse);
    responseFactory.emplace(REQ_RES_RESET_SOURCE_MAPPING, ToRemoveSourceMappingResponse);
    responseFactory.emplace(REQ_RES_ADD_SOURCE_BREAKPOINT, ToAddSourceBreakpointJson);
    responseFactory.emplace(REQ_RES_DELETE_BREAKPOINT, ToDeleteBreakpointJson);
    responseFactory.emplace(REQ_RES_VARIABLE_HEX_VIEW, ToVariableHexViewResponse);
    responseFactory.emplace(REQ_RES_COMMAND_COMPLETION, ToCommandCompletionResponse);

    toResponseFactory.emplace(REQ_RES_RUN_IN_TERMINAL, ToRunInTerminalResponse);

    isRegister = true;
}
void ResponseManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!isRegister) {
        return;
    }
    responseFactory.clear();
    isRegister = false;
}
bool ResponseManager::IsRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    return isRegister;
}

void Dap::Protocol::SetResponseJsonBaseInfo(const Response &response, json_t &json)
{
    json["type"] = "response";
    json["seq"] = response.seq;
    json["request_seq"] = response.request_seq;
    json["success"] = response.success;
    json["command"] = response.command;
    if (response.message.has_value()) {
        json["message"] = response.message.value();
    }
}

template <typename RESPONSE> std::optional<json_t> Dap::Protocol::ToResponseJson(const RESPONSE &response)
{
    (void)response;
    return std::nullopt;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::Response>(const Response &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::ErrorResponse>(const ErrorResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    if (response.error.has_value()) {
        json["body"]["error"] = GetTypeJson<Message>(response.error.value()).value();
    }
    json["command"] = response.requestCommand;
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::InitializeResponse>(
    const InitializeResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    if (response.body.has_value()) {
        json["body"] = GetTypeJson<Dap::Protocol::Capabilities>(response.body.value()).value();
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::ConfigurationDoneResponse>(
    const ConfigurationDoneResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::LaunchResponse>(const LaunchResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::AttachResponse>(const AttachResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::DisconnectResponse>(
    const DisconnectResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::TerminateResponse>(
    const TerminateResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::BreakpointLocationsResponse>(
    const BreakpointLocationsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t breakpointsJson = json_t::array();
    for (BreakpointLocation breakpointLocation : response.breakpoints) {
        json_t breakpointJson = json_t::object();
        breakpointJson["line"] = breakpointLocation.line;
        SetJsonOptional(breakpointJson, breakpointLocation.column, "column");
        SetJsonOptional(breakpointJson, breakpointLocation.endLine, "endLine");
        SetJsonOptional(breakpointJson, breakpointLocation.endColumn, "endColumn");
        SetJsonOptional(breakpointJson, breakpointLocation.address, "address");
        breakpointsJson.push_back(std::move(breakpointJson));
    }
    json["body"]["breakpoints"] = std::move(breakpointsJson);
    return json;
}

json_t SetBreakpointsJson(const std::vector<Breakpoint> &breakpoints)
{
    json_t breakpointsJson = json_t::array();
    for (Breakpoint breakpoint : breakpoints) {
        json_t breakpointJson = json_t::object();
        breakpointJson["verified"] = breakpoint.verified;
        SetJsonOptional(breakpointJson, breakpoint.id, "id");
        SetJsonOptional(breakpointJson, breakpoint.message, "message");
        if (breakpoint.source.has_value()) {
            breakpointJson["source"] = GetTypeJson(breakpoint.source.value()).value();
        }
        SetJsonOptional(breakpointJson, breakpoint.line, "line");
        SetJsonOptional(breakpointJson, breakpoint.column, "column");
        SetJsonOptional(breakpointJson, breakpoint.endLine, "endLine");
        SetJsonOptional(breakpointJson, breakpoint.endColumn, "endColumn");
        SetJsonOptional(breakpointJson, breakpoint.instructionReference, "instructionReference");
        SetJsonOptional(breakpointJson, breakpoint.offset, "offset");
        breakpointsJson.push_back(std::move(breakpointJson));
    }
    return breakpointsJson;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetBreakpointsResponse>(
    const SetBreakpointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["breakpoints"] = SetBreakpointsJson(response.breakpoints);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::AddSourceBreakpointResponse>(
    const AddSourceBreakpointResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t breakpointJson = json_t::object();
    breakpointJson["verified"] = response.breakpoint.verified;
    SetJsonOptional(breakpointJson, response.breakpoint.id, "id");
    SetJsonOptional(breakpointJson, response.breakpoint.message, "message");
    if (response.breakpoint.source.has_value()) {
        breakpointJson["source"] = GetTypeJson(response.breakpoint.source.value()).value();
    }
    SetJsonOptional(breakpointJson, response.breakpoint.line, "line");
    SetJsonOptional(breakpointJson, response.breakpoint.column, "column");
    SetJsonOptional(breakpointJson, response.breakpoint.endLine, "endLine");
    SetJsonOptional(breakpointJson, response.breakpoint.endColumn, "endColumn");
    SetJsonOptional(breakpointJson, response.breakpoint.instructionReference, "instructionReference");
    SetJsonOptional(breakpointJson, response.breakpoint.offset, "offset");
    json["body"]["breakpoint"] = breakpointJson;
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::DeleteBreakpointResponse>(
    const DeleteBreakpointResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetFunctionBreakpointsResponse>(
    const SetFunctionBreakpointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["breakpoints"] = SetBreakpointsJson(response.breakpoints);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::DataBreakpointInfoResponse>(
    const DataBreakpointInfoResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["dataId"] = response.dataId;
    json["body"]["description"] = response.description;
    if (response.accessTypes.has_value()) {
        json_t accessTypes = json_t::array();
        for (DataBreakpointAccessType dataBreakpointAccessType : response.accessTypes.value()) {
            accessTypes.push_back(ENUM_TO_STR(dataBreakpointAccessType).value());
        }
        json["body"]["accessTypes"] = accessTypes;
    }
    SetJsonOptional(json, response.canPersist, "body", "canPersist");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetDataBreakpointsResponse>(
    const SetDataBreakpointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["breakpoints"] = SetBreakpointsJson(response.breakpoints);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetExceptionBreakpointsResponse>(
    const SetExceptionBreakpointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["breakpoints"] = SetBreakpointsJson(response.breakpoints);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetInstructionBreakpointsResponse>(
    const SetInstructionBreakpointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["breakpoints"] = SetBreakpointsJson(response.breakpoints);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::ContinueResponse>(const ContinueResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    SetJsonOptional(json, response.allThreadsContinued, "body", "allThreadsContinued");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::NextResponse>(const NextResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::StepInResponse>(const StepInResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::StepOutResponse>(const StepOutResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::StackTraceResponse>(
    const StackTraceResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t stackFramesJson = json_t::array();
    for (StackFrame stackFrame : response.stackFrames) {
        stackFramesJson.push_back(GetTypeJson(stackFrame).value());
    }
    json["body"]["stackFrames"] = std::move(stackFramesJson);
    SetJsonOptional(json, response.totalFrames, "body", "totalFrames");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::ThreadsResponse>(const ThreadsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t threadsJson = json_t::array();
    for (Thread thread : response.threads) {
        json_t threadJson = json_t::object();
        threadJson["id"] = thread.id;
        threadJson["name"] = thread.name;
        SetJsonOptional(threadJson, thread.tid, "tid");
        threadsJson.push_back(std::move(threadJson));
    }
    json["body"]["threads"] = std::move(threadsJson);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::ScopesResponse>(const ScopesResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t scopesJson = json_t::array();
    for (Scope scope : response.scopes) {
        json_t scopeJson = json_t::object();
        scopeJson["name"] = scope.name;
        scopeJson["variablesReference"] = scope.variablesReference;
        scopeJson["expensive"] = scope.expensive;
        SetJsonOptional(scopeJson, scope.presentationHint, "presentationHint");
        scopesJson.push_back(scopeJson);
    }
    json["body"]["scopes"] = std::move(scopesJson);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::VariablesResponse>(
    const VariablesResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    if (response.variablesCount.has_value()) {
        json["body"]["variablesCount"] = response.variablesCount.value();
    }
    json_t varsJson = json_t::array();
    for (const Variable &var : response.variables) {
        varsJson.push_back(GetTypeJson(var).value());
    }
    json["body"]["variables"] = varsJson;
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::SetVariableResponse>(
    const SetVariableResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t varJson = json_t::object();
    varJson["value"] = response.value;
    SetJsonOptional(varJson, response.variableType, "type");
    SetJsonOptional(varJson, response.variablesReference, "variablesReference");
    SetJsonOptional(varJson, response.namedVariables, "namedVariables");
    SetJsonOptional(varJson, response.indexedVariables, "indexedVariables");
    json["body"] = varJson;
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<Dap::Protocol::PauseResponse>(const PauseResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<DisassembleResponse>(const DisassembleResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json_t instructionsJson = json_t::array();
    for (auto instruction : response.instructions) {
        json_t instructionJson = GetTypeJson(instruction).value();
        instructionsJson.push_back(instructionJson);
    }
    json["body"]["instructions"] = instructionsJson;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToResponseJson<EvaluateResponse>(const EvaluateResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["result"] = response.result;
    json["body"]["variablesReference"] = response.variablesReference;
    SetJsonOptional(json, response.resultType, "body", "type");
    if (response.presentationHint.has_value()) {
        SetJsonOptional(json, GetTypeJson<VariablePresentationHint>(response.presentationHint.value()),
            "body", "presentationHint");
    }
    SetJsonOptional(json, response.namedVariables, "body", "namedVariables");
    SetJsonOptional(json, response.indexedVariables, "body", "indexedVariables");
    SetJsonOptional(json, response.memoryReference, "body", "memoryReference");
    SetJsonOptional(json, response.memoryAddress, "body", "memoryAddress");

    if (CommonUtils::JsonDump(json).empty() && CommonUtils::JsonDump(json["body"]["result"]).empty()) {
        LOG_ERROR("can not get value");
        json["body"]["result"] = "";
        json["success"] = false;
        json["message"] = "error:can not get value";
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<DebugInConsoleResponse>(const DebugInConsoleResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["output"] = response.output;
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<SourceResponse>(const SourceResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["content"] = response.content;
    SetJsonOptional(json, response.mimeType, "body", "mimeType");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<ReadMemoryResponse>(const ReadMemoryResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["address"] = response.address;
    SetJsonOptional(json, response.unreadableBytes, "body", "unreadableBytes");
    SetJsonOptional(json, response.data, "body", "data");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<WriteMemoryResponse>(const WriteMemoryResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    SetJsonOptional(json, response.offset, "body", "offset");
    SetJsonOptional(json, response.bytesWritten, "body", "bytesWritten");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<ReverseStepResponse>(const ReverseStepResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<UndoResponse>(const UndoResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<ReverseContinueResponse>(const ReverseContinueResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<RecordPointsResponse>(const RecordPointsResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["recordPoints"] = json_t::array();
    for (const auto &it : response.recordPoints) {
        json_t point;
        point["id"] = it.id;
        point["threadId"] = it.threadId;
        point["threadName"] = it.threadName;
        point["dateTimeStamp"] = it.dateTimeStamp;
        point["timeStamp"] = it.timeStamp;
        point["type"] = it.reason;
        SetJsonOptional(point, it.coroutineId, "coroutineId");
        json["body"]["recordPoints"].push_back(point);
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<GotoRecordPointResponse>(const GotoRecordPointResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<CacheDataConfigResponse>(const CacheDataConfigResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<SetExecutionPointResponse>(
    const SetExecutionPointResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<ContinueInReverseResponse>(
    const ContinueInReverseResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<StepInReverseResponse>(const StepInReverseResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <typename T>
void SetOptionalVariablesJson(json_t &recordMessage, const std::string &fieldName,
    const std::optional<std::vector<T>> &variables)
{
    if (!variables.has_value()) {
        return;
    }
    recordMessage[fieldName] = json_t::array();
    for (const auto &variable : variables.value()) {
        auto variableJson = GetTypeJson(variable);
        if (variableJson.has_value()) {
            recordMessage[fieldName].push_back(variableJson.value());
        }
    }
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<RecordMessageResponse>(const RecordMessageResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["recordMessages"] = json_t::array();
    for (const auto &it : response.recordMessages) {
        json_t recordMessage;
        recordMessage["recordPointId"] = it.recordPointId;
        if (it.thread.has_value()) {
            recordMessage["thread"]["id"] = it.thread->id;
            recordMessage["thread"]["name"] = it.thread->name;
        }
        if (it.stackFrame.has_value()) {
            auto stackFrameJson = GetTypeJson(it.stackFrame.value());
            if (stackFrameJson.has_value()) {
                recordMessage["stackFrame"] = stackFrameJson.value();
            }
        }
        SetOptionalVariablesJson(recordMessage, "localVariables", it.localVariables);
        SetOptionalVariablesJson(recordMessage, "globalVariables", it.globalVariables);
        SetOptionalVariablesJson(recordMessage, "staticVariables", it.staticVariables);
        SetOptionalVariablesJson(recordMessage, "monitorVariables", it.monitorVariables);
        json["body"]["recordMessages"].push_back(recordMessage);
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<SetMemoryBreakpointsResponse>(
    const SetMemoryBreakpointsResponse &response)
{
    return ToResponseJson<SetBreakpointsResponse>((SetBreakpointsResponse &)response);
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<GetBitmapResponse>(const GetBitmapResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["width"] = response.bitmap.width;
    json["body"]["height"] = response.bitmap.height;
    json["body"]["format"]["colorFormat"] = response.bitmap.format.colorFormat;
    json["body"]["format"]["alphaFormat"] = response.bitmap.format.alphaFormat;
    SetJsonOptional(json, response.bitmap.size, "body", "size");
    SetJsonOptional(json, response.bitmap.data, "body", "data");
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<LoadModulesResponse>(const LoadModulesResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["modules"] = json_t::array();
    for (const auto &module : response.modules) {
        json_t moduleJson = GetTypeJson(module).value();
        json["body"]["modules"].push_back(moduleJson);
    }
    if (response.totalModules.has_value()) {
        json["body"]["totalModules"] = response.totalModules.value();
    }
    if (response.error.has_value()) {
        json["body"]["error"] = response.error.value();
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<AddSourceMappingResponse>(const AddSourceMappingResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["moduleIdList"] = json_t::array();
    for (const auto &moduleId : response.moduleIdList) {
        json["body"]["moduleIdList"].push_back(moduleId);
    }
    json["body"]["sourceRootDirList"] = json_t::array();
    for (const auto &dir : response.sourceRootDirList) {
        json["body"]["sourceRootDirList"].push_back(dir);
    }
    if (response.error.has_value()) {
        json["body"]["error"] = response.error.value();
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<ResetSourceMappingResponse>(
    const ResetSourceMappingResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["moduleIdList"] = json_t::array();
    for (const auto &moduleId : response.moduleIdList) {
        json["body"]["moduleIdList"].push_back(moduleId);
    }
    if (response.error.has_value()) {
        json["body"]["error"] = response.error.value();
    }
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<VariableHexViewResponse>(const VariableHexViewResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    return json;
}

template <>
std::optional<json_t> Dap::Protocol::ToResponseJson<CommandCompletionResponse>(
    const CommandCompletionResponse &response)
{
    json_t json;
    SetResponseJsonBaseInfo(response, json);
    json["body"]["isPatch"] = response.isPatch;
    json["body"]["objects"] = json_t::array();
    for (const auto &obj : response.objects) {
        json_t jsonObj = json_t::object();
        jsonObj["command"] = obj.command;
        SetJsonOptional(jsonObj, obj.alias, "alias");
        jsonObj["spec"] = obj.spec;
        json["body"]["objects"].push_back(jsonObj);
    }
    return json;
}

std::optional<json_t> ResponseManager::ToResponseBaseJson(const Response &response)
{
    return ToResponseJson<Response>(response);
}

std::optional<json_t> ResponseManager::ToErrorJson(const Response &response)
{
    return ToResponseJson<ErrorResponse>((ErrorResponse &)response);
}

std::optional<json_t> ResponseManager::ToInitializeJson(const Response &response)
{
    return ToResponseJson<InitializeResponse>((InitializeResponse &)response);
}

std::optional<json_t> ResponseManager::ToConfigurationDoneJson(const Response &response)
{
    return ToResponseJson<ConfigurationDoneResponse>((ConfigurationDoneResponse &)response);
}

std::optional<json_t> ResponseManager::ToLaunchJson(const Response &response)
{
    return ToResponseJson<LaunchResponse>((LaunchResponse &)response);
}

std::optional<json_t> ResponseManager::ToAttachJson(const Response &response)
{
    return ToResponseJson<AttachResponse>((AttachResponse &)response);
}

std::optional<json_t> ResponseManager::ToDisconnectJson(const Response &response)
{
    return ToResponseJson<DisconnectResponse>((DisconnectResponse &)response);
}

std::optional<json_t> ResponseManager::ToTerminateJson(const Response &response)
{
    return ToResponseJson<TerminateResponse>((TerminateResponse &)response);
}

std::optional<json_t> ResponseManager::ToBreakpointLocationsJson(const Response &response)
{
    return ToResponseJson<BreakpointLocationsResponse>((BreakpointLocationsResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetBreakpointsJson(const Response &response)
{
    return ToResponseJson<SetBreakpointsResponse>((SetBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetFunctionBreakpointsJson(const Response &response)
{
    return ToResponseJson<SetFunctionBreakpointsResponse>((SetFunctionBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToDataBreakpointInfoJson(const Response &response)
{
    return ToResponseJson<DataBreakpointInfoResponse>((DataBreakpointInfoResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetExceptionBreakpointsJson(const Response &response)
{
    return ToResponseJson<SetExceptionBreakpointsResponse>((SetExceptionBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetInstructionBreakpointsJson(const Response &response)
{
    return ToResponseJson<SetInstructionBreakpointsResponse>((SetInstructionBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetDataBreakpointsJson(const Response &response)
{
    return ToResponseJson<SetDataBreakpointsResponse>((SetDataBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToContinueJson(const Response &response)
{
    return ToResponseJson<ContinueResponse>((ContinueResponse &)response);
}

std::optional<json_t> ResponseManager::ToNextJson(const Response &response)
{
    return ToResponseJson<NextResponse>((NextResponse &)response);
}

std::optional<json_t> ResponseManager::ToStepInJson(const Response &response)
{
    return ToResponseJson<StepInResponse>((StepInResponse &)response);
}

std::optional<json_t> ResponseManager::ToStepOutJson(const Response &response)
{
    return ToResponseJson<StepOutResponse>((StepOutResponse &)response);
}

std::optional<json_t> ResponseManager::ToStackTraceJson(const Response &response)
{
    return ToResponseJson<StackTraceResponse>((StackTraceResponse &)response);
}

std::optional<json_t> ResponseManager::ToThreadsJson(const Response &response)
{
    return ToResponseJson<ThreadsResponse>((ThreadsResponse &)response);
}

std::optional<json_t> ResponseManager::ToScopesJson(const Response &response)
{
    return ToResponseJson<ScopesResponse>((ScopesResponse &)response);
}

std::optional<json_t> ResponseManager::ToVariablesJson(const Response &response)
{
    return ToResponseJson<VariablesResponse>((VariablesResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetVariableJson(const Response &response)
{
    return ToResponseJson<SetVariableResponse>((SetVariableResponse &)response);
}

std::optional<json_t> ResponseManager::ToPauseJson(const Response &response)
{
    return ToResponseJson<PauseResponse>((PauseResponse &)response);
}

std::optional<json_t> ResponseManager::ToDisassembleJson(const Response &response)
{
    return ToResponseJson<DisassembleResponse>((DisassembleResponse &)response);
}

std::optional<json_t> ResponseManager::ToEvaluateJson(const Response &response)
{
    return ToResponseJson<EvaluateResponse>((EvaluateResponse &)response);
}

bool ResponseManager::HasValue(const json_t &json, const std::string &key)
{
    return json.contains(key) && !json[key].is_null();
}

std::optional<json_t> ResponseManager::ToDebugInConsoleJson(const Response &response)
{
    return ToResponseJson<DebugInConsoleResponse>((DebugInConsoleResponse &)response);
}

std::optional<json_t> ResponseManager::ToSourceJson(const Response &response)
{
    return ToResponseJson<SourceResponse>((SourceResponse &)response);
}

std::optional<json_t> ResponseManager::ToReadMemoryJson(const Response &response)
{
    return ToResponseJson<ReadMemoryResponse>((ReadMemoryResponse &)response);
}

std::optional<json_t> ResponseManager::ToWriteMemoryJson(const Response &response)
{
    return ToResponseJson<WriteMemoryResponse>((WriteMemoryResponse &)response);
}

bool ResponseManager::SetResponseBaseInfo(Response &response, const json_t &json)
{
    if (!HasValue(json, "seq") || !HasValue(json, "type") || !HasValue(json, "command") ||
        !HasValue(json, "request_seq") || !HasValue(json, "success")) {
        LOG_ERROR("SetResponseBaseInfo error, json is ", json);
        return false;
    }
    response.seq = json["seq"];
    response.type = STR_TO_ENUM<Dap::Protocol::ProtocolMessage::Type>(json["type"]).value();
    response.command = json["command"];
    response.request_seq = json["request_seq"];
    response.success = json["success"];
    if (HasValue(json, "message")) {
        response.message = json["message"];
    }
    return true;
}

std::unique_ptr<Response> ResponseManager::ToRunInTerminalResponse(const json_t &json)
{
    std::unique_ptr<RunInTerminalResponse> responsePtr = std::make_unique<RunInTerminalResponse>();
    if (!SetResponseBaseInfo(*responsePtr, json)) {
        LOG_ERROR("to RunInTerminalResponse failed in json, json is:", json);
        return nullptr;
    }
    if (!HasValue(json, "body")) {
        return responsePtr;
    }
    if (HasValue(json["body"], "processId") && json["body"]["processId"].is_number_integer()) {
        responsePtr->processId = json["body"]["processId"];
    }
    if (HasValue(json["body"], "shellProcessId") && json["body"]["shellProcessId"].is_number_integer()) {
        responsePtr->shellProcessId = json["body"]["shellProcessId"];
    }
    return responsePtr;
}

std::optional<json_t> ResponseManager::ToReverseStepJson(const Response &response)
{
    return ToResponseJson<ReverseStepResponse>((ReverseStepResponse &)response);
}

std::optional<json_t> ResponseManager::ToUndoJson(const Response &response)
{
    return ToResponseJson<UndoResponse>((UndoResponse &)response);
}

std::optional<json_t> ResponseManager::ToReverseContinueJson(const Response &response)
{
    return ToResponseJson<ReverseContinueResponse>((ReverseContinueResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetExecutionPointJson(const Response &response)
{
    return ToResponseJson<SetExecutionPointResponse>((SetExecutionPointResponse &)response);
}
std::optional<json_t> ResponseManager::ToRecordPointsJson(const Response &response)
{
    return ToResponseJson<RecordPointsResponse>((RecordPointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToGotoPointJson(const Response &response)
{
    return ToResponseJson<GotoRecordPointResponse>((GotoRecordPointResponse &)response);
}

std::optional<json_t> ResponseManager::ToContinueInReverseJson(const Response &response)
{
    return ToResponseJson<ContinueInReverseResponse>((ContinueInReverseResponse &)response);
}

std::optional<json_t> ResponseManager::ToStepInReverseJson(const Response &response)
{
    return ToResponseJson<StepInReverseResponse>((StepInReverseResponse &)response);
}

std::optional<json_t> ResponseManager::ToCacheDataConfigJson(const Response &response)
{
    return ToResponseJson<CacheDataConfigResponse>((CacheDataConfigResponse &)response);
}

std::optional<json_t> ResponseManager::ToRecordMessageResponse(const Response &response)
{
    return ToResponseJson<RecordMessageResponse>((RecordMessageResponse &)response);
}

std::optional<json_t> ResponseManager::ToSetMemoryBreakpointsResponse(const Response &response)
{
    return ToResponseJson<SetMemoryBreakpointsResponse>((SetMemoryBreakpointsResponse &)response);
}

std::optional<json_t> ResponseManager::ToGetBitmapResponse(const Response &response)
{
    return ToResponseJson<GetBitmapResponse>((GetBitmapResponse &)response);
}

std::optional<json_t> ResponseManager::ToLoadModulesResponse(const Response &response)
{
    return ToResponseJson<LoadModulesResponse>((LoadModulesResponse &)response);
}

std::optional<json_t> ResponseManager::ToSourceMappingResponse(const Response &response)
{
    return ToResponseJson<AddSourceMappingResponse>((AddSourceMappingResponse &)response);
}

std::optional<json_t> ResponseManager::ToRemoveSourceMappingResponse(const Response &response)
{
    return ToResponseJson<ResetSourceMappingResponse>((ResetSourceMappingResponse &)response);
}

std::optional<json_t> ResponseManager::ToAddSourceBreakpointJson(const Response &response)
{
    return ToResponseJson<AddSourceBreakpointResponse>((AddSourceBreakpointResponse &)response);
}

std::optional<json_t> ResponseManager::ToDeleteBreakpointJson(const Response &response)
{
    return ToResponseJson<DeleteBreakpointResponse>((DeleteBreakpointResponse &)response);
}

std::optional<json_t> ResponseManager::ToVariableHexViewResponse(const Response &response)
{
    return ToResponseJson<VariableHexViewResponse>((VariableHexViewResponse &)response);
}

std::optional<json_t> ResponseManager::ToCommandCompletionResponse(const Response &response)
{
    return ToResponseJson<CommandCompletionResponse>((CommandCompletionResponse &)response);
}
