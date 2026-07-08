// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TimeTravelDebugManager.h"
#include "EnumUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

TimeTravelDebugManager::TimeTravelDebugManager(
    std::shared_ptr<Debugger> debugger) : debugger(std::move(debugger))
{
    Configure();
    LOG_INFO("Time travel debug manager initialized successfully.");
}

void TimeTravelDebugManager::RecordResponse(RequestArgumentsContent content, std::string &command,
                                            std::shared_ptr<Response> res)
{
    if (!RequestNeedRecord(command)) {
        return;
    }
    LOG_INFO("reverse | TTD enable ,record command = ", command);

    if (recordNodeQueue.empty()) {
        LOG_INFO("The record point container is empty");
        return;
    }
    if (setResponseHandlerMap.count(command) < 1) {
        LOG_INFO("Not Supported ResponseHandler : command = ", command);
        return;
    }
    std::shared_ptr<RecordNode> node = recordNodeQueue.back();
    setResponseHandlerMap[command](node, std::move(res), std::move(content));
}

std::shared_ptr<Response> TimeTravelDebugManager::GetResponse(const std::string &command,
                                                              const RequestArgumentsContent &content)
{
    if (recordNodeQueue.empty()) {
        LOG_INFO("The record node container is empty");
        return nullptr;
    }
    LOG_INFO("The record node container size is ", recordNodeQueue.size());
    std::shared_ptr<RecordNode> node = recordNodeQueue[currentRecordNodeIndex];
    std::shared_ptr<Response> response = getResponseHandlerMap[command](node, content);
    return response;
}

bool TimeTravelDebugManager::IsUndoEnable() const
{
    return undoEnable;
}

std::shared_ptr<Response> TimeTravelDebugManager::GetDebugInfoResponseInUndo(const RequestArgumentsContent &content,
                                                                             const std::string &command,
                                                                             unsigned int seq)
{
    if (!RequestNeedRecord(command)) {
        return nullptr;
    }

    std::shared_ptr<Response> undoResponse = GetResponse(command, content);
    if (undoResponse != nullptr) {
        undoResponse->request_seq = seq;
    } else {
        string error = "Information haven't been saved during debug process";
        LOG_ERROR(error);
        undoResponse = std::make_unique<ErrorResponse>(command);
        undoResponse->request_seq = seq;
        undoResponse->seq = debugger->GetAndIncCurrentMessageSeq();
        undoResponse->message = error;
        undoResponse->success = false;
    }
    return undoResponse;
}

void TimeTravelDebugManager::Configure()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (isConfigure) {
        return;
    }

    setResponseHandlerMap.emplace(REQ_RES_THREADS, SetThreadsHandler);
    setResponseHandlerMap.emplace(REQ_RES_STACK_TRACE, SetStackTraceHandler);
    setResponseHandlerMap.emplace(REQ_RES_SCOPES, SetScopesHandler);
    setResponseHandlerMap.emplace(REQ_RES_VARIABLES, SetVariablesHandler);

    getResponseHandlerMap.emplace(REQ_RES_THREADS, GetThreadsHandler);
    getResponseHandlerMap.emplace(REQ_RES_STACK_TRACE, GetStackTraceHandler);
    getResponseHandlerMap.emplace(REQ_RES_SCOPES, GetScopesHandler);
    getResponseHandlerMap.emplace(REQ_RES_VARIABLES, GetVariablesHandler);

    supportedNodeTypeArray.push_back("step");
    supportedNodeTypeArray.push_back("breakpoint");
    supportedNodeTypeArray.push_back("function breakpoint");
    supportedNodeTypeArray.push_back("instruction breakpoint");
    supportedNodeTypeArray.push_back("data breakpoint");
    supportedNodeTypeArray.push_back("exception");
    supportedNodeTypeArray.push_back("pause");
    supportedNodeTypeArray.push_back("set executionpoint");
    isConfigure = true;
}

RequestArgumentsContent TimeTravelDebugManager::GetArgumentsContent(Request *req) const
{
    RequestArgumentsContent content;
    if (req->command == REQ_RES_STACK_TRACE) {
        content.content = ((StackTraceRequest *) req)->arguments;
    } else if (req->command == REQ_RES_SCOPES) {
        content.content = ((ScopesRequest *) req)->arguments;
    } else if (req->command == REQ_RES_VARIABLES) {
        content.content = ((VariablesRequest *) req)->arguments;
    }
    return content;
}

void TimeTravelDebugManager::SetThreadsHandler(const std::shared_ptr<RecordNode> &node,
                                               const std::shared_ptr<Response> &res,
                                               const RequestArgumentsContent &content)
{
    (void)content;
    if (node->responses.threads != nullptr) {
        return;
    }
    node->responses.threads = res;
    // record stopped thread name
    if (node->threadName.empty() && node->threadId > 0) {
        auto threadsResponse = dynamic_cast<ThreadsResponse *>(res.get());
        if (threadsResponse == nullptr) {
            return;
        }
        for (const auto& thread : threadsResponse->threads) {
            if (node->threadId == thread.id) {
                node->threadName = thread.name;
            }
        }
    }
}

void TimeTravelDebugManager::SetStackTraceHandler(const std::shared_ptr<RecordNode> &node,
                                                  const std::shared_ptr<Response> &res,
                                                  RequestArgumentsContent content)
{
    StackTraceArguments arguments = std::get<StackTraceArguments>(content.content);
    node->responses.stackTrace[arguments.threadId] = res;
}

void TimeTravelDebugManager::SetScopesHandler(const std::shared_ptr<RecordNode> &node,
                                              const std::shared_ptr<Response> &res,
                                              RequestArgumentsContent content)
{
    ScopesArguments arguments = std::get<ScopesArguments>(content.content);
    node->responses.scopes[arguments.frameId] = res;
}

void TimeTravelDebugManager::SetVariablesHandler(const std::shared_ptr<RecordNode> &node,
                                                 const std::shared_ptr<Response> &res,
                                                 RequestArgumentsContent content)
{
    VariablesArguments arguments = std::get<VariablesArguments>(content.content);
    int64_t start = arguments.start.has_value() ? arguments.start.value() : 0;
    node->responses.variables[make_pair(arguments.variablesReference, start)] = res;
}

std::shared_ptr<Dap::Protocol::Response> TimeTravelDebugManager::GetThreadsHandler(
    const std::shared_ptr<RecordNode> &node, const RequestArgumentsContent &content)
{
    (void)content;
    return node->responses.threads;
}

std::shared_ptr<Dap::Protocol::Response> TimeTravelDebugManager::GetStackTraceHandler(
    const std::shared_ptr<RecordNode> &node, RequestArgumentsContent content)
{
    StackTraceArguments arguments = std::get<StackTraceArguments>(content.content);
    auto it = node->responses.stackTrace.find(arguments.threadId);
    if (it == node->responses.stackTrace.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<Dap::Protocol::Response> TimeTravelDebugManager::GetScopesHandler(
    const std::shared_ptr<RecordNode> &node, RequestArgumentsContent content)
{
    ScopesArguments arguments = std::get<ScopesArguments>(content.content);
    auto it = node->responses.scopes.find(arguments.frameId);
    if (it == node->responses.scopes.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<Dap::Protocol::Response> TimeTravelDebugManager::GetVariablesHandler(
    const std::shared_ptr<RecordNode> &node, RequestArgumentsContent content)
{
    VariablesArguments arguments = std::get<VariablesArguments>(content.content);
    int64_t start = arguments.start.has_value() ? arguments.start.value() : 0;
    auto it = node->responses.variables.find(make_pair(arguments.variablesReference, start));
    if (it == node->responses.variables.end()) {
        return nullptr;
    }
    return it->second;
}

void TimeTravelDebugManager::CreateRecordNode(const std::string &reason)
{
    RecordNode node;
    node.id = GetAndIncRecordId();
    node.reason = reason;
    node.dataTimeStamp = Dap::TimeUtils::Instance().NowStr(Dap::TimeStyle::DATE_TIME_1);
    // Configure
    SetRecordNodeContainer(std::make_shared<RecordNode>(node));
}

void TimeTravelDebugManager::CreateRecordNode(const Dap::Protocol::StoppedEventBody &body, const int64_t coroutineId)
{
    RecordNode node;
    node.id = GetAndIncRecordId();
    node.reason = ENUM_TO_STR(body.reason).value();
    node.stoppedEventBody = body;
    node.stoppedEventBody.recordPointId = node.id;
    if (body.threadId.has_value()) {
        node.threadId = body.threadId.value();
    }
    node.coroutineId = coroutineId;
    node.dataTimeStamp = Dap::TimeUtils::Instance().NowStr(Dap::TimeStyle::DATE_TIME_1);
    node.timeStamp = Dap::TimeUtils::Instance().NowUnixTimeMs();
    if (debugger != nullptr) {
        node.steppingGranularity = debugger->GetCurrentStepGranularity();
    }

    // cache record points added and deleted between stop events
    if (cacheOverflow) {
        // cache overwritten record point ID
        removedRecordPointIds.push_back(node.id);
        // erase the overwritten record point ID, if it exists, it always at the top of newRecordPointIds
        if (!newRecordPointIds.empty() && newRecordPointIds.at(0) == node.id) {
            newRecordPointIds.erase(newRecordPointIds.begin());
        }
    }
    if (IsSupportedRecordPointType(node.reason)) {
        newRecordPointIds.push_back(node.id);
    }

    // Configure
    SetRecordNodeContainer(std::make_shared<RecordNode>(node));
}

void TimeTravelDebugManager::SetRecordNodeContainer(const std::shared_ptr<RecordNode> &node)
{
    RecordPointStorage(&recordNodeQueue, node);
    currentRecordNodeIndex = recordNodeQueue.size() - 1;
    LOG_INFO("Number of record points is ", recordNodeQueue.size());
}

void TimeTravelDebugManager::SelectRecordNode(const std::string &key, bool isGotoType)
{
    isGotoRecordPointType = isGotoType;
    LOG_INFO("current work key = ", key);
    currentWorkKey = key;
    SetCurrentContainerAvailable(true);

    constexpr int gapBetweenIndexAndSize = 1;
    if (key == "step") {
        if (currentRecordNodeIndex + gapBetweenIndexAndSize + 1 <= recordNodeQueue.size()) {
            currentRecordNodeIndex++;
        } else {
            SetCurrentContainerAvailable(false);
        }
    } else if (key == "reverse step") {
        if (currentRecordNodeIndex > 0) {
            currentRecordNodeIndex--;
        } else {
            SetCurrentContainerAvailable(false);
        }
    } else if (key == "breakpoint") {
        SelectBreakpointRecordNode(true);
    } else if (key == "reverse breakpoint") {
        SelectBreakpointRecordNode(false);
    }
}

void TimeTravelDebugManager::StateSwitchProcess(bool flag)
{
    this->undoEnable = flag;
    AdjustRecordContainerOnStopUndo();
}


void TimeTravelDebugManager::SelectBreakpointRecordNode(bool forward)
{
    if (forward) {
        for (auto i = currentRecordNodeIndex + 1; i < recordNodeQueue.size(); i++) {
            if (IsBreakpoint(recordNodeQueue[i]->reason)) {
                currentRecordNodeIndex = i;
                return;
            }
        }
        SetCurrentContainerAvailable(false);
    } else {
        for (auto j = currentRecordNodeIndex; j > 0; j--) {
            if (IsBreakpoint(recordNodeQueue[j - 1]->reason)) {
                currentRecordNodeIndex = j - 1;
                return;
            }
        }
        SetCurrentContainerAvailable(false);
    }
}

void TimeTravelDebugManager::AdjustRecordContainerOnStopUndo()
{
    currentRecordNodeIndex = recordNodeQueue.size() - 1;
    currentWorkKey = "reverse step";
    SetCurrentContainerAvailable(true);
}

void TimeTravelDebugManager::RecordPointStorage(std::deque<std::shared_ptr<RecordNode>> *container,
                                                const std::shared_ptr<RecordNode> &node) const
{
    if (container->size() == RECORD_ID_MAX) {
        container->pop_front();
        container->emplace_back(node);
    } else {
        container->emplace_back(node);
    }
}

bool TimeTravelDebugManager::IsAllowedMessagesInUndo(const string &command) const
{
    return std::find(allowedMessagesInUndo.begin(), allowedMessagesInUndo.end(), command) !=
        allowedMessagesInUndo.end();
}

bool TimeTravelDebugManager::RequestNeedRecord(const string &command) const
{
    return std::find(recordRequests.begin(), recordRequests.end(), command) != recordRequests.end();
}

bool TimeTravelDebugManager::IsTriggeredUndoEvent(const string &command) const
{
    return std::find(triggeredUndoEvents.begin(), triggeredUndoEvents.end(), command) != triggeredUndoEvents.end();
}

bool TimeTravelDebugManager::IsNotAllowedInNormal(const string &command) const
{
    return std::find(notAllowedInNormal.begin(), notAllowedInNormal.end(), command) != notAllowedInNormal.end();
}

bool TimeTravelDebugManager::IsBreakpoint(const string &reason) const
{
    if (reason == "breakpoint" || reason == "function breakpoint" || reason == "exception" ||
        reason == "instruction breakpoint" || reason == "data breakpoint" || reason == "set executionpoint") {
        return true;
    }
    return false;
}

bool TimeTravelDebugManager::IsAvailableOnCurrentContainer() const
{
    return isGotoRecordPointType ? true : isAvailableOnCurrentContainer;
}

void TimeTravelDebugManager::SetCurrentContainerAvailable(bool status)
{
    isAvailableOnCurrentContainer = status;
}

std::vector<Protocol::RecordPoint> TimeTravelDebugManager::GetSupportedRecordPoints(uint64_t start, uint64_t count)
{
    std::vector<Protocol::RecordPoint> rcdPoints;
    if (start == 0) {
        start = 1;
    }
    // if count is 0, then return as many record points as possible
    if (count == 0) {
        count = recordNodeQueue.size();
    }
    for (auto i = start - 1; i < recordNodeQueue.size() && count > 0; i++, count--) {
        if (!IsSupportedRecordPointType(recordNodeQueue[i]->reason)) {
            continue;
        }
        RecordPoint rcdPoint;
        rcdPoint.id = static_cast<int>(recordNodeQueue[i]->id);
        rcdPoint.dateTimeStamp = recordNodeQueue[i]->dataTimeStamp;
        rcdPoint.timeStamp = recordNodeQueue[i]->timeStamp;
        rcdPoint.reason = recordNodeQueue[i]->reason;
        rcdPoint.threadId = recordNodeQueue[i]->threadId;
        rcdPoint.threadName = recordNodeQueue[i]->threadName;
        if (recordNodeQueue[i]->coroutineId != -1) {
            rcdPoint.coroutineId = recordNodeQueue[i]->coroutineId;
        }
        rcdPoints.push_back(rcdPoint);
    }
    ClearRecordPointsUpdateCache();
    return rcdPoints;
}

int TimeTravelDebugManager::GetCurrentNodeId() const
{
    if (recordNodeQueue.size() > 0) {
        return recordNodeQueue[currentRecordNodeIndex]->id;
    }
    LOG_WARN("no record node in container");
    return -1;
}

bool TimeTravelDebugManager::GotoRecordPoint(int id)
{
    isGotoRecordPointType = true;
    if (recordNodeQueue.size() == 0 || id < 0) {
        return false;
    }
    int targetIndex = GetRecordNodeIndexById(id);
    if (targetIndex < 0) {
        return false;
    }
    currentRecordNodeIndex = static_cast<unsigned int>(targetIndex);
    return true;
}

void TimeTravelDebugManager::UpdateVariablesCache(const std::string &varName, const int &parentRef,
    const std::string &newValue)
{
    LOG_INFO("update variables cache.");
    std::shared_ptr<Dap::Protocol::VariablesResponse> cachedVariablesResponse;
    std::shared_ptr<RecordNode> currentRecordNode = GetCurrentRecordNode();
    if (currentRecordNode == nullptr) {
        return;
    }
    for (auto &variablesCache : currentRecordNode->responses.variables) {
        if (std::get<0>(variablesCache.first) == parentRef) {
            cachedVariablesResponse = std::dynamic_pointer_cast<VariablesResponse>(variablesCache.second);
            break;
        }
    }
    if (cachedVariablesResponse == nullptr) {
        return;
    }
    for (auto &variable : cachedVariablesResponse->variables) {
        if (variable.name == varName) {
            LOG_INFO("change value of ", variable.name, " from ", variable.value, " to ", newValue);
            variable.value = newValue;
            break;
        }
    }
}

int TimeTravelDebugManager::GetRecordNodeIndexById(int id) const
{
    if (id < 0) {
        return -1;
    }
    unsigned int unId = static_cast<unsigned int>(id);
    for (unsigned int i = 0; i < recordNodeQueue.size(); i++) {
        if (recordNodeQueue[i]->id == unId) {
            return i;
        }
    }
    return -1;
}

bool TimeTravelDebugManager::IsSupportedRecordPointType(std::string type) const
{
    if (std::find(supportedNodeTypeArray.begin(), supportedNodeTypeArray.end(),  type) ==
        supportedNodeTypeArray.end()) {
        return false;
    }
    return true;
}

std::shared_ptr<Response> TimeTravelDebugManager::GetResponseFromLatestNode(const RequestArgumentsContent &content,
                                                                            const string &command, unsigned int seq)
{
    if (!RequestNeedRecord(command) || recordNodeQueue.empty()) {
        return nullptr;
    }
    std::shared_ptr<RecordNode> node = recordNodeQueue.back();
    std::shared_ptr<Response> response = getResponseHandlerMap[command](node, content);
    if (response != nullptr) {
        response->request_seq = seq;
    }
    return response;
}

RecordPointsUpdateEventBody TimeTravelDebugManager::GetRecordPointsUpdateEventBody()
{
    RecordPointsUpdateEventBody body;
    if (!removedRecordPointIds.empty()) {
        body.removedRecordPointIds = removedRecordPointIds;
    }
    if (!newRecordPointIds.empty()) {
        auto startId = newRecordPointIds.at(0);
        auto startIndex = recordNodeQueue.size();
        while (startIndex > 0 && recordNodeQueue.at(startIndex - 1)->id != startId) {
            startIndex--;
        }
        if (startIndex > 0 && recordNodeQueue.at(startIndex - 1)->id == startId) {
            body.newRecordPoints = GetSupportedRecordPoints(startIndex, newRecordPointIds.size());
        }
    }
    ClearRecordPointsUpdateCache();
    return body;
}

bool TimeTravelDebugManager::IsSupportedRecordPointType(Dap::Protocol::StoppedReason reason) const
{
    if (ENUM_TO_STR(reason).has_value()) {
        return IsSupportedRecordPointType(ENUM_TO_STR(reason).value());
    } else {
        return false;
    }
}

Dap::Protocol::Thread RecordNode::GetStoppedThread() const
{
    Dap::Protocol::Thread thread;
    thread.tid = threadId;
    thread.name = threadName;
    return thread;
}

Dap::Protocol::StackFrame RecordNode::GetTopFrame() const
{
    StackFrame stackFrame;
    stackFrame.id = -1;
    auto stackTraceMap = responses.stackTrace;
    if (stackTraceMap.count(threadId) == 0) {
        return stackFrame;
    }
    auto response = dynamic_cast<StackTraceResponse *>(stackTraceMap.at(threadId).get());
    if (response == nullptr) {
        return stackFrame;
    }
    if (!response->stackFrames.empty()) {
        return response->stackFrames.at(0);
    }
    return stackFrame;
}
