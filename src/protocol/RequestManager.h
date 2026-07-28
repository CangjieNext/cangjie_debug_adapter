// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_REQUEST_MANAGER_H
#define DAP_REQUEST_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <utility>
#include <mutex>
#include <optional>

#include "defs.h"
#include "Protocol.h"
#include "ProtocolDefs.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

/* *
 * Insert request-command-func
 * CONTAINER  : type of map<string, std::function<std::shared_ptr<Request>(const json_t&)>>
 *              or std::map<std::string, std::function<std::optional<json_t>(const Request &)>>
 * REQ        : request command
 * Func       : std::function<std::shared_ptr<Request>(const json_t&)> object
                or std::function<std::optional<json_t>(const Request &)> object
 */
#define INSERT_REQUEST(CONTAINER, REQ, FUNC) \
    if ((CONTAINER).count(REQ) == 0) {         \
        (CONTAINER).emplace(REQ, FUNC);        \
    }

class RequestManager {
public:
    RequestManager(const RequestManager&) = delete;
    RequestManager &operator=(const RequestManager&) = delete;
    RequestManager(const RequestManager &&) = delete;
    const RequestManager &operator=(const RequestManager &&) = delete;
    using CreateRequestFunc = std::function<std::unique_ptr<Request>(const json_t &)>;
    using RequestToJsonFunc = std::function<std::optional<json_t>(const Request &)>;
    static RequestManager &Instance()
    {
        static RequestManager instance;
        return instance;
    }

    void Register();
    void UnRegister();
    bool IsRegister();
    std::optional<CreateRequestFunc> GetCreateRequestFunc(const std::string &command);
    std::optional<RequestManager::RequestToJsonFunc> GetRequestJsonFunc(const std::string &command);

    bool IsRequest(const json_t &jsonRequest) const;
    std::string Command(const json_t &jsonRequest) const;
    std::unique_ptr<Request> FromJson(const json_t &requestJson);
    std::unique_ptr<Request> FromJson(const std::string &requestStr);

    std::optional<json_t> ToJson(const Request &request);

private:
    RequestManager() = default;
    ~RequestManager() = default;
    static bool SetRequestBaseInfo(Request &request, const json_t &requestJson);
    static bool SetRequestSource(const json_t &json, Dap::Protocol::Source &source);
    static bool SetFilter(const json_t &json, std::vector<ExceptionFilterOptions> exVec);
    static bool SetExceptionOptions(const json_t &json, std::vector<ExceptionOptions> excVec);
    static void SetRequestJsonBaseInfo(const Request &request, json_t &json);
    static bool HasValue(const json_t &json, const std::string &key);
    static void SetLaunchArguments(const json_t &json, LaunchRequestArguments &arguments);
    static void SetAttachArguments(const json_t &json, AttachRequestArguments &arguments);
    static std::vector<SetupCommand> GetSetupCommands(const json_t &setupCommandsJson);
    static std::vector<std::string> GetVectorString(const json_t &stringVectorJson);
    static FunctionBreakpointData SetFunctionBreakpointData(const json_t &json);
    static std::unique_ptr<Request> ToInitializeRequest(const json_t &json);
    static std::unique_ptr<Request> ToLaunchRequest(const json_t &json);
    static std::unique_ptr<Request> ToAttachRequest(const json_t &json);
    static std::unique_ptr<Request> ToConfigurationDoneRequest(const json_t &json);
    static std::unique_ptr<Request> ToRestartRequest(const json_t &json);
    static std::unique_ptr<Request> ToDisconnectRequest(const json_t &json);
    static std::unique_ptr<Request> ToTerminateRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToBreakpointLocationsRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetFunctionBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToDataBreakpointInfoRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetDataBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetExceptionBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetInstructionBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToContinueRequest(const json_t &json);
    static std::unique_ptr<Request> ToPauseRequest(const json_t &json);
    static std::unique_ptr<Request> ToNextRequest(const json_t &json);
    static std::unique_ptr<Request> ToStepInRequest(const json_t &json);
    static std::unique_ptr<Request> ToStepOutRequest(const json_t &json);
    static std::unique_ptr<Request> ToStackTraceRequest(const json_t &json);
    static std::unique_ptr<Request> ToThreadsRequest(const json_t &json);
    static std::unique_ptr<Request> ToScopesRequest(const json_t &json);
    static std::unique_ptr<Request> ToVariablesRequest(const json_t &json);
    static std::unique_ptr<Request> ToVariablesExpandRequest(const json_t &json);
    static std::unique_ptr<Request> ToPreVariables(const json_t &json);
    static std::unique_ptr<Request> ToSetVariableRequest(const json_t &json);
    static std::unique_ptr<Request> ToDisassembleRequest(const json_t &json);
    static std::unique_ptr<Request> ToEvaluateRequest(const json_t &json);
    static std::unique_ptr<Request> ToDebugInConsoleRequest(const json_t &json);
    static std::unique_ptr<Request> ToSourceRequest(const json_t &json);
    static std::unique_ptr<Request> ToReadMemoryRequest(const json_t &json);
    static std::unique_ptr<Request> ToWriteMemoryRequest(const json_t &json);
    static std::unique_ptr<Request> ToReverseStepRequest(const json_t &json);
    static std::unique_ptr<Request> ToUndoRequest(const json_t &json);
    static std::unique_ptr<Request> ToReverseContinueRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetExecutionPointRequest(const json_t &json);
    static std::unique_ptr<Request> ToRecordPointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToGotoRecordPointRequest(const json_t &json);
    static std::unique_ptr<Request> ToContinueInReverseRequest(const json_t &json);
    static std::unique_ptr<Request> ToStepInReverseRequest(const json_t &json);
    static std::unique_ptr<Request> ToCacheDataConfigRequest(const json_t &json);
    static std::unique_ptr<Request> ToRecordMessageRequest(const json_t &json);
    static std::unique_ptr<Request> ToSetMemoryBreakpointsRequest(const json_t &json);
    static std::unique_ptr<Request> ToGetBitmapRequest(const json_t &json);
    static std::unique_ptr<Request> ToLoadModulesRequest(const json_t &json);
    static std::unique_ptr<Request> ToAddSourceMappingRequest(const json_t &json);
    static std::unique_ptr<Request> ToResetSourceMappingRequest(const json_t &json);
    static std::unique_ptr<Request> ToAddSourceBreakpointRequest(const json_t &json);
    static std::unique_ptr<Request> ToDeleteBreakpointRequest(const json_t &json);
    static std::unique_ptr<Request> ToVariableHexViewRequest(const json_t &json);
    static std::unique_ptr<Request> ToCommandCompletionRequest(const json_t &json);

    static std::optional<json_t> ToRunInTerminalJson(const Request &request);

    bool isRegister = false;
    std::mutex mutex;
    std::map<std::string, CreateRequestFunc> requestFactory;
    std::map<std::string, RequestToJsonFunc> reqToJsonFactory;

    static VariablesArguments GetVariablesArguments(const json_t &json);
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_REQUEST_MANAGER_H