// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_RESPONSE_MANAGER_H
#define DAP_RESPONSE_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <utility>
#include <mutex>
#include <optional>
#include <vector>

#include "defs.h"
#include "Protocol.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

void SetResponseJsonBaseInfo(const Response &response, json_t &json);

template <typename T> std::optional<json_t> ToResponseJson(const T &response);
template <> std::optional<json_t> ToResponseJson<Response>(const Response &response);
template <> std::optional<json_t> ToResponseJson<ErrorResponse>(const ErrorResponse &response);
template <> std::optional<json_t> ToResponseJson<InitializeResponse>(const InitializeResponse &response);
template <> std::optional<json_t> ToResponseJson<ConfigurationDoneResponse>(
    const ConfigurationDoneResponse &response);
template <> std::optional<json_t> ToResponseJson<LaunchResponse>(const LaunchResponse &response);
template <> std::optional<json_t> ToResponseJson<AttachResponse>(const AttachResponse &response);
template <> std::optional<json_t> ToResponseJson<DisconnectResponse>(const DisconnectResponse &response);
template <> std::optional<json_t> ToResponseJson<TerminateResponse>(const TerminateResponse &response);
template <>
std::optional<json_t> ToResponseJson<BreakpointLocationsResponse>(const BreakpointLocationsResponse &response);
template <> std::optional<json_t> ToResponseJson<SetBreakpointsResponse>(const SetBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<SetFunctionBreakpointsResponse>(
    const SetFunctionBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<DataBreakpointInfoResponse>(
    const DataBreakpointInfoResponse &response);
template <> std::optional<json_t> ToResponseJson<SetDataBreakpointsResponse>(
    const SetDataBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<SetExceptionBreakpointsResponse>(
    const SetExceptionBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<SetInstructionBreakpointsResponse>(
    const SetInstructionBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<ContinueResponse>(const ContinueResponse &response);
template <> std::optional<json_t> ToResponseJson<NextResponse>(const NextResponse &response);
template <> std::optional<json_t> ToResponseJson<StepInResponse>(const StepInResponse &response);
template <> std::optional<json_t> ToResponseJson<StepOutResponse>(const StepOutResponse &response);
template <> std::optional<json_t> ToResponseJson<StackTraceResponse>(const StackTraceResponse &response);
template <> std::optional<json_t> ToResponseJson<ThreadsResponse>(const ThreadsResponse &response);
template <> std::optional<json_t> ToResponseJson<ScopesResponse>(const ScopesResponse &response);
template <> std::optional<json_t> ToResponseJson<VariablesResponse>(const VariablesResponse &response);
template <> std::optional<json_t> ToResponseJson<SetVariableResponse>(const SetVariableResponse &response);
template <> std::optional<json_t> ToResponseJson<PauseResponse>(const PauseResponse &response);
template <> std::optional<json_t> ToResponseJson<DisassembleResponse>(const DisassembleResponse &response);
template <> std::optional<json_t> ToResponseJson<EvaluateResponse>(const EvaluateResponse &response);
template <> std::optional<json_t> ToResponseJson<DebugInConsoleResponse>(const DebugInConsoleResponse &response);
template <> std::optional<json_t> ToResponseJson<SourceResponse>(const SourceResponse &response);
template <> std::optional<json_t> ToResponseJson<ReadMemoryResponse>(const ReadMemoryResponse &response);
template <> std::optional<json_t> ToResponseJson<WriteMemoryResponse>(const WriteMemoryResponse &response);
template <> std::optional<json_t> ToResponseJson<ReverseStepResponse>(const ReverseStepResponse &response);
template <> std::optional<json_t> ToResponseJson<UndoResponse>(const UndoResponse &response);
template <> std::optional<json_t> ToResponseJson<ReverseContinueResponse>(const ReverseContinueResponse &response);
template <> std::optional<json_t> ToResponseJson<SetExecutionPointResponse>(const SetExecutionPointResponse &response);
template <> std::optional<json_t> ToResponseJson<RecordPointsResponse>(const RecordPointsResponse &response);
template <> std::optional<json_t> ToResponseJson<GotoRecordPointResponse>(const GotoRecordPointResponse &response);
template <> std::optional<json_t> ToResponseJson<ContinueInReverseResponse>(const ContinueInReverseResponse &response);
template <> std::optional<json_t> ToResponseJson<StepInReverseResponse>(const StepInReverseResponse &response);
template <> std::optional<json_t> ToResponseJson<CacheDataConfigResponse>(const CacheDataConfigResponse &response);
template <> std::optional<json_t> ToResponseJson<RecordMessageResponse>(const RecordMessageResponse &response);
template <> std::optional<json_t> ToResponseJson<SetMemoryBreakpointsResponse>(
    const SetMemoryBreakpointsResponse &response);
template <> std::optional<json_t> ToResponseJson<GetBitmapResponse>(const GetBitmapResponse &response);
template <> std::optional<json_t> ToResponseJson<LoadModulesResponse>(const LoadModulesResponse &response);
template <> std::optional<json_t> ToResponseJson<AddSourceMappingResponse>(const AddSourceMappingResponse &response);
template <> std::optional<json_t> ToResponseJson<ResetSourceMappingResponse>(
    const ResetSourceMappingResponse &response);
template <> std::optional<json_t> ToResponseJson<AddSourceBreakpointResponse>(
    const AddSourceBreakpointResponse &response);
template <> std::optional<json_t> ToResponseJson<DeleteBreakpointResponse>(const DeleteBreakpointResponse &response);
template <> std::optional<json_t> ToResponseJson<VariableHexViewResponse>(const VariableHexViewResponse &response);
template <> std::optional<json_t> ToResponseJson<CommandCompletionResponse>(const CommandCompletionResponse &response);

class ResponseManager {
public:
    ResponseManager(const ResponseManager&) = delete;
    ResponseManager &operator=(const ResponseManager&) = delete;
    ResponseManager(const ResponseManager &&) = delete;
    const ResponseManager &operator=(const ResponseManager &&) = delete;
    static ResponseManager &Instance()
    {
        static ResponseManager instance;
        return instance;
    }
    using ResponseToJsonFunc = std::function<std::optional<json_t>(const Response &)>;
    using JsonToResponseFunc = std::function<std::unique_ptr<Response>(const json_t &)>;
    void Register();
    void UnRegister();
    bool IsRegister();
    bool IsResponse(const json_t &jsonResponse) const;
    static bool SetResponseBaseInfo(Response &response, const json_t &json);
    static bool HasValue(const json_t &json, const std::string &key);
    std::string Command(const json_t &jsonRequest) const;
    std::optional<ResponseManager::ResponseToJsonFunc> GetResponseJsonFunc(const std::string &command);
    std::optional<ResponseManager::JsonToResponseFunc> GetResponseFunc(const std::string &command);
    std::optional<json_t> ToJson(const Response& response);
    std::unique_ptr<Response> ToResponse(const json_t &responseJson);
    std::unique_ptr<Response> ToResponse(const std::string &responseStr);

private:
    ResponseManager() = default;
    ~ResponseManager() = default;
    static std::optional<json_t> ToResponseBaseJson(const Response &response);
    static std::optional<json_t> ToErrorJson(const Response &response);
    static std::optional<json_t> ToInitializeJson(const Response &response);
    static std::optional<json_t> ToConfigurationDoneJson(const Response &response);
    static std::optional<json_t> ToLaunchJson(const Response &response);
    static std::optional<json_t> ToAttachJson(const Response &response);
    static std::optional<json_t> ToDisconnectJson(const Response &response);
    static std::optional<json_t> ToTerminateJson(const Response &response);
    static std::optional<json_t> ToBreakpointLocationsJson(const Response &response);
    static std::optional<json_t> ToSetBreakpointsJson(const Response &response);
    static std::optional<json_t> ToSetFunctionBreakpointsJson(const Response &response);
    static std::optional<json_t> ToDataBreakpointInfoJson(const Response &response);
    static std::optional<json_t> ToSetDataBreakpointsJson(const Response &response);
    static std::optional<json_t> ToSetExceptionBreakpointsJson(const Response &response);
    static std::optional<json_t> ToSetInstructionBreakpointsJson(const Response &response);
    static std::optional<json_t> ToContinueJson(const Response &response);
    static std::optional<json_t> ToNextJson(const Response &response);
    static std::optional<json_t> ToStepInJson(const Response &response);
    static std::optional<json_t> ToStepOutJson(const Response &response);
    static std::optional<json_t> ToStackTraceJson(const Response &response);
    static std::optional<json_t> ToThreadsJson(const Response &response);
    static std::optional<json_t> ToScopesJson(const Response &response);
    static std::optional<json_t> ToVariablesJson(const Response &response);
    static std::optional<json_t> ToSetVariableJson(const Response &response);
    static std::optional<json_t> ToPauseJson(const Response &response);
    static std::optional<json_t> ToDisassembleJson(const Response &response);
    static std::optional<json_t> ToEvaluateJson(const Response &response);
    static std::optional<json_t> ToDebugInConsoleJson(const Response &response);
    static std::optional<json_t> ToSourceJson(const Response &response);
    static std::optional<json_t> ToReadMemoryJson(const Response &response);
    static std::optional<json_t> ToWriteMemoryJson(const Response &response);
    static std::optional<json_t> ToReverseStepJson(const Response &response);
    static std::optional<json_t> ToUndoJson(const Response &response);
    static std::optional<json_t> ToReverseContinueJson(const Response &response);
    static std::optional<json_t> ToSetExecutionPointJson(const Response &response);
    static std::optional<json_t> ToRecordPointsJson(const Response &response);
    static std::optional<json_t> ToGotoPointJson(const Response &response);
    static std::optional<json_t> ToContinueInReverseJson(const Response &response);
    static std::optional<json_t> ToStepInReverseJson(const Response &response);
    static std::optional<json_t> ToCacheDataConfigJson(const Response &response);
    static std::optional<json_t> ToRecordMessageResponse(const Response &response);
    static std::optional<json_t> ToSetMemoryBreakpointsResponse(const Response &response);
    static std::optional<json_t> ToGetBitmapResponse(const Response &response);
    static std::optional<json_t> ToLoadModulesResponse(const Response &response);
    static std::optional<json_t> ToSourceMappingResponse(const Response &response);
    static std::optional<json_t> ToRemoveSourceMappingResponse(const Response &response);
    static std::optional<json_t> ToAddSourceBreakpointJson(const Response &response);
    static std::optional<json_t> ToDeleteBreakpointJson(const Response &response);
    static std::optional<json_t> ToVariableHexViewResponse(const Response &response);
    static std::optional<json_t> ToCommandCompletionResponse(const Response &response);

    static std::unique_ptr<Response> ToRunInTerminalResponse(const json_t &json);
private:
    std::mutex mutex;
    bool isRegister = false;
    std::map<std::string, ResponseToJsonFunc> responseFactory;
    std::map<std::string, JsonToResponseFunc> toResponseFactory;
};
NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_RESPONSE_MANAGER_H
