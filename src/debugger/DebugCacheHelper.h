// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUG_CACHE_HELPER_H
#define DAP_SERVER_DEBUG_CACHE_HELPER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <set>
#include "Protocol.h"

namespace Dap {

class Debugger;

struct CacheDataConfig {
    bool startCacheData = false;
    bool saveHistoryCache = true;
    // Number of cached threads, 0: cache all threads, n(n>1): cache n threads
    int64_t threadCount = 0;
    // Number of cached stackFrames, 0: cache all stackFrames, n(n>1): cache n stackFrames
    int64_t stackFrameCount = 0;
    // Number of cached variableChildrens, 0: do not cache variableChildren, n(n>1): cache n variableChildrens
    int64_t variableChildrenCount = 100;
    // The number of variableChildren expansion layers, 0: do not expand variableChildren, n(n>1): expand n layers
    int64_t variableChildrenLayers = 1;
    // Number of stack stackFrame pages, 0: no paging, n(n>1): display n stackFrame per page
    int64_t stackTracePageSize = 0;
    // Number of stack variable pages, 0: no paging, n(n>1): display n variable per page
    int64_t variablePageSize = 100;
    // Cache scope type, Four-digit binary number, 1 indicates that the cache corresponds to scope,
    // 1-Locals, 2-Globals, 4-Statics, 8-Registers
    uint64_t scopeType = 1;
};

enum class CacheDataType {
    THREAD,
    STACKTRACE,
    SCOPE,
    VARIABLE
};

enum class ScopeTypeClass {
    LOCALS = 0b0001,
    GLOBALS = 0b0010,
    STATICS = 0b0100,
    REGISTERS = 0b1000
};

class DebugCacheHelper {
public:
    explicit DebugCacheHelper(std::shared_ptr<Debugger> debugger);
    ~DebugCacheHelper() = default;

    void StartCaching(const Dap::Protocol::StoppedEvent &stoppecEvent);
    void SendRequest(std::unique_ptr<Dap::Protocol::Request> request);
    int HandleResponse(const Dap::Protocol::Response &response);

    void UpdateCacheDataConfig(CacheDataConfig &config)
    {
        cacheDataConfigProperty = config;
    }

    bool CacheInServer()
    {
        return cacheDataConfigProperty.startCacheData;
    }

private:
    void StopCaching();
    void CancelCaching();
    int HandleThreadsResponse(const Dap::Protocol::ThreadsResponse &response);
    int HandleStackTraceResponse(const Dap::Protocol::StackTraceResponse &response);
    int HandleScopesResponse(const Dap::Protocol::ScopesResponse &response);
    int HandleVariablesResponse(const Dap::Protocol::VariablesResponse &response);
    void SendPagedVariablesRequest(int64_t variablesReference, int64_t cacheChildNum, int64_t layers);
    void SendThreadRequest();
    void SendStackTraceRequest(int64_t threadId, int64_t startFrame, int64_t levels);
    void SendScopesRequest(int64_t frameId);
    void SendVariablesRequest(int64_t variablesReference, int64_t start, int64_t count, int64_t layers);
    void HandleNextCaching(CacheDataType cacheDataType);
    bool CheckCacheScope(std::string scopeName);

    int64_t PopThreadId()
    {
        int64_t threadId = threadIds.front();
        threadIds.pop();
        return threadId;
    }

    int64_t PopStackTraceId()
    {
        int64_t stackTraceId = stackTraceIds.front();
        stackTraceIds.pop();
        return stackTraceId;
    }

    int64_t PopScopeVarRef()
    {
        int64_t scopeVarRef = scopeVarRefs.front();
        scopeVarRefs.pop();
        return scopeVarRef;
    }

    void ClearThreadIds()
    {
        std::queue<int64_t> t;
        swap(t, threadIds);
    }

    void ClearStackTraceIds()
    {
        std::queue<int64_t> t;
        swap(t, stackTraceIds);
    }

    void ClearScopeVarRefs()
    {
        std::queue<int64_t> t;
        swap(t, scopeVarRefs);
    }

    void Clear()
    {
        ClearThreadIds();
        ClearStackTraceIds();
        ClearScopeVarRefs();
        threadCount = 0;
        stackTraceCount = 0;
        scopeCount = 0;
        variableCount = 0;
        scopeSeq = 0;
        requestSeqAndVarDepth.clear();
        varRefAndVarDepth.clear();
    }

private:
    int defaultVariableChildrenNum = 1000;
    int defaultStackTraceNum = 1000;
    const std::string scopeNameStr[4] = {"Locals", "Globals", "Statics", "Registers"};
    std::shared_ptr<Debugger> debugger = nullptr;
    Dap::Protocol::StoppedEvent eventToSend;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    std::queue<std::unique_ptr<Dap::Protocol::Request>> requestQueue;
    CacheDataConfig cacheDataConfigProperty;
    // thread ids to be requested
    std::queue<int64_t> threadIds;
    // the stack id to be requested
    std::queue<int64_t> stackTraceIds;
    // scope to be requested
    std::queue<int64_t> scopeVarRefs;
    // total number of thread to send
    int64_t threadCount = 0;
    // total number of stackTrace to send
    int64_t stackTraceCount = 0;
    // total number of scopes to send
    int64_t scopeCount = 0;
    // total number of variable to send
    int64_t variableCount = 0;
    // the seq of the first scopes request sent when caching data
    int64_t scopeSeq = 0;
    // <requestSeq - variablesDepth>
    std::map<int64_t, int64_t> requestSeqAndVarDepth;
    // <variableReference - variablesDepth>
    std::map<int64_t, int64_t> varRefAndVarDepth;
};
};

#endif // DAP_SERVER_DEBUG_CACHE_HELPER_H