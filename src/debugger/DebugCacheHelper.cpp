// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <thread>
#include "Debugger.h"
#include "DebugCacheHelper.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

DebugCacheHelper::DebugCacheHelper(std::shared_ptr<Debugger> debugger)
    : debugger(std::move(debugger))
{
}

void DebugCacheHelper::StartCaching(const StoppedEvent &stoppedEvent)
{
    LOG_INFO("Start caching.");
    Clear();
    this->eventToSend = stoppedEvent;
    debugger->SetRequestHandleMode(RequestHandleMode::INTERNAL);
    const int waitMs = 50;
    SendThreadRequest();
    while (debugger->GetRequestHandleMode() == RequestHandleMode::INTERNAL) {
        if (requestQueue.empty()) {
            std::unique_lock<std::mutex> locker(queueMutex);
            queueCv.wait_for(locker, std::chrono::milliseconds(waitMs));
        } else {
            std::unique_ptr<Request> req;
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                req = std::move(requestQueue.front());
                requestQueue.pop();
            } // out of lock_guard scope, unlock automatically
            debugger->OnDapRequest(std::move(req));
        }
    }
}

void DebugCacheHelper::StopCaching()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!requestQueue.empty()) {
            requestQueue.pop();
        }
    } // out of lock_guard scope, unlock automatically
    debugger->ClearMarkInternalRequest();
    debugger->SetRequestHandleMode(RequestHandleMode::EXTERNAL);
}

void DebugCacheHelper::CancelCaching()
{
    Clear();
    StopCaching();
}

void DebugCacheHelper::SendRequest(std::unique_ptr<Request> request)
{
    debugger->MarkInternalRequest(request->seq);
    std::unique_lock<std::mutex> locker(queueMutex);
    requestQueue.emplace(std::move(request));
    locker.unlock();
    queueCv.notify_one();
}

int DebugCacheHelper::HandleResponse(const Response &response)
{
    int ret = 0;
    if (response.command == REQ_RES_THREADS) {
        ret = HandleThreadsResponse(dynamic_cast<const ThreadsResponse &>(response));
    } else if (response.command == REQ_RES_STACK_TRACE) {
        ret = HandleStackTraceResponse(dynamic_cast<const StackTraceResponse &>(response));
    } else if (response.command == REQ_RES_SCOPES) {
        ret = HandleScopesResponse(dynamic_cast<const ScopesResponse &>(response));
    } else if (response.command == REQ_RES_VARIABLES) {
        ret = HandleVariablesResponse(dynamic_cast<const VariablesResponse &>(response));
    } else {
        LOG_ERROR("Unexpected response in debug caching !");
        ret = -1;
    }
    return ret;
}

int DebugCacheHelper::HandleThreadsResponse(const ThreadsResponse &response)
{
    vector<Thread> threads = response.threads;
    if (threads.size() == 0) {
        LOG_ERROR("Unexpected response in debug caching !");
        StopCaching();
        return 0;
    }
    int64_t curThreadId = eventToSend.body.threadId.has_value() ? eventToSend.body.threadId.value() : -1;
    uint64_t num = min<size_t>(threads.size(), cacheDataConfigProperty.threadCount == 0 ?
        threads.size() : cacheDataConfigProperty.threadCount);
    for (uint64_t i = 0; i < threads.size() && threadIds.size() < (num - 1); i++) {
        if (threads[i].id == curThreadId) {
            continue;
        }
        threadIds.push(threads[i].id);
        threadCount++;
    }
    // To ensure that the thread is switched to the currently stopped thread after the automatic cache ends,
    // the current notification thread ID is the last to enqueue
    threadIds.push(curThreadId);
    threadCount++;
    HandleNextCaching(CacheDataType::THREAD);
    return 0;
}

int DebugCacheHelper::HandleStackTraceResponse(const StackTraceResponse &response)
{
    threadCount--;
    vector<StackFrame> stackFrames = response.stackFrames;
    if (stackFrames.size() == 0) {
        return 0;
    }
    uint64_t num = min<size_t>(stackFrames.size(), cacheDataConfigProperty.stackFrameCount ==
        0 ? stackFrames.size() : cacheDataConfigProperty.stackFrameCount);
    if (stackTraceIds.size() >= num) {
        return -1;
    }
    for (uint64_t i = num; i-- > 0;) {
        if (!stackFrames[i].source.has_value()) {
            continue;
        }
        Source source = stackFrames[i].source.value();
        if (source.path && stackFrames[i].id != -1) {
            string error;
            if (!CommonUtils::CheckFilePath(source.path.value(), error)) {
                continue;
            }
            stackTraceIds.push(stackFrames[i].id);
            stackTraceCount++;
        }
    }
    HandleNextCaching(CacheDataType::STACKTRACE);
    return 0;
}

int DebugCacheHelper::HandleScopesResponse(const ScopesResponse &response)
{
    stackTraceCount--;
    vector<Scope> scopes = response.scopes;
    for (uint64_t i = 0; i < scopes.size(); i++) {
        if (CheckCacheScope(scopes[i].name)) {
            scopeVarRefs.push(scopes[i].variablesReference);
            scopeCount++;
        }
    }
    HandleNextCaching(CacheDataType::SCOPE);
    return 0;
}

int DebugCacheHelper::HandleVariablesResponse(const VariablesResponse &response)
{
    variableCount--;
    if (response.request_seq == scopeSeq) {
        scopeCount--;
    }
    vector<Variable> variables = response.variables;
    if (variables.empty()) {
        HandleNextCaching(CacheDataType::VARIABLE);
        return 0;
    }
    if (requestSeqAndVarDepth.find(response.request_seq) == requestSeqAndVarDepth.end()) {
        return 0;
    }
    for (uint64_t i = 0; i < variables.size(); i++) {
        int variablesReference = variables[i].variablesReference;
        if (requestSeqAndVarDepth.find(response.request_seq) != requestSeqAndVarDepth.end()) {
            int layers = requestSeqAndVarDepth[response.request_seq];
            int64_t variableChildrenLayers = cacheDataConfigProperty.variableChildrenLayers;
            if (variablesReference <= 0 || layers + 1 > variableChildrenLayers) {
                continue;
            }
            int64_t indexedVariables = variables[i].indexedVariables.has_value() ?
                    variables[i].indexedVariables.value() : -1;
            if (indexedVariables != -1) {
                int cacheChildNum = min(indexedVariables, cacheDataConfigProperty.variableChildrenCount);
                SendPagedVariablesRequest(variablesReference, cacheChildNum, layers);
            } else {
                int64_t count = cacheDataConfigProperty.variableChildrenCount < defaultVariableChildrenNum ?
                        cacheDataConfigProperty.variableChildrenCount : defaultVariableChildrenNum;
                SendVariablesRequest(variablesReference, 0, count, layers + 1);
            }
            varRefAndVarDepth[variablesReference] = layers - 1;
        }
    }
    HandleNextCaching(CacheDataType::VARIABLE);
    return 0;
}

void DebugCacheHelper::SendPagedVariablesRequest(int64_t variablesReference, int64_t cacheChildNum, int64_t layers)
{
    int64_t variablePageSize = cacheDataConfigProperty.variablePageSize;
    if (variablePageSize <= 0 || cacheChildNum <= variablePageSize) {
        SendVariablesRequest(variablesReference, 0, cacheChildNum, layers + 1);
    } else {
        for (int j = 0; j <= cacheChildNum / variablePageSize; j++) {
            int64_t start = j * variablePageSize;
            int64_t count = (cacheChildNum - start) > variablePageSize ?
                            variablePageSize : (cacheChildNum - start);
            if (count > 0) {
                SendVariablesRequest(variablesReference, start, count, layers + 1);
            }
        }
    }
}

void DebugCacheHelper::SendThreadRequest()
{
    unique_ptr<Request> request = make_unique<ThreadsRequest>();
    request->seq = debugger->GetAndIncCurrentMessageSeq();
    SendRequest(std::move(request));
}

void DebugCacheHelper::SendStackTraceRequest(int64_t threadId, int64_t startFrame, int64_t levels)
{
    unique_ptr<StackTraceRequest> request = make_unique<StackTraceRequest>();
    request->seq = debugger->GetAndIncCurrentMessageSeq();
    request->arguments.threadId = threadId;
    request->arguments.startFrame = startFrame;
    request->arguments.levels = levels;
    SendRequest(std::move(request));
}

void DebugCacheHelper::SendScopesRequest(int64_t frameId)
{
    unique_ptr<ScopesRequest> request = make_unique<ScopesRequest>();
    request->seq = debugger->GetAndIncCurrentMessageSeq();
    request->arguments.frameId = frameId;
    SendRequest(std::move(request));
}

void DebugCacheHelper::SendVariablesRequest(int64_t variablesReference, int64_t start, int64_t count, int64_t layers)
{
    unique_ptr<VariablesRequest> request = make_unique<VariablesRequest>();
    request->seq = debugger->GetAndIncCurrentMessageSeq();
    request->arguments.variablesReference = variablesReference;
    request->arguments.start = start;
    request->arguments.count = count;
    requestSeqAndVarDepth[request->seq] = layers;
    if (layers == 0) {
        scopeSeq = request->seq;
    }
    variableCount++;
    SendRequest(std::move(request));
}

// Process the data waiting to be cached, if there is no data waiting to be cached, stop caching and send stoppedEvent
void DebugCacheHelper::HandleNextCaching(CacheDataType cacheDataType)
{
    switch (cacheDataType) {
        case CacheDataType::VARIABLE:
            if (variableCount != 0) {
                // there is still variable request that has not received a response, continue to wait
                break;
            }
            [[fallthrough]];
        case CacheDataType::SCOPE:
            if (scopeCount != 0) {
                // request next stackTrace info
                SendVariablesRequest(PopScopeVarRef(), 0, defaultVariableChildrenNum, 0);
                break;
            }
            [[fallthrough]];
        case CacheDataType::STACKTRACE:
            if (stackTraceCount != 0) {
                // request next scope info
                SendScopesRequest(PopStackTraceId());
                break;
            }
            [[fallthrough]];
        case CacheDataType::THREAD:
            if (threadCount != 0) {
                // request next thread info
                int64_t levels = cacheDataConfigProperty.stackFrameCount == 0 ?
                    defaultStackTraceNum : cacheDataConfigProperty.stackFrameCount;
                SendStackTraceRequest(PopThreadId(), 0, levels);
                break;
            }
            [[fallthrough]];
        default: {
            StopCaching();
        }
    }
}

bool DebugCacheHelper::CheckCacheScope(string scopeName)
{
    uint64_t scopeTypeFilters = cacheDataConfigProperty.scopeType;
    uint64_t scopeType;
    if (scopeName == "Locals") {
        scopeType = static_cast<uint64_t>(ScopeTypeClass::LOCALS);
    } else if (scopeName == "Globals") {
        scopeType = static_cast<uint64_t>(ScopeTypeClass::GLOBALS);
    } else if (scopeName == "Statics") {
        scopeType = static_cast<uint64_t>(ScopeTypeClass::STATICS);
    } else {
        scopeType = static_cast<uint64_t>(ScopeTypeClass::REGISTERS);
    }
    return (scopeTypeFilters & scopeType) != 0;
}