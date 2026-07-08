// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_TIME_TRAVEL_DEBUG_MANAGER_H
#define DAP_TIME_TRAVEL_DEBUG_MANAGER_H

#include <string>
#include <map>
#include <stack>
#include <queue>
#include <functional>
#include <utility>
#include <mutex>
#include <optional>
#include <variant>
#include <vector>
#include <deque>

#include "../defs.h"
#include "Protocol.h"
#include "TimeUtils.h"
#include "DapParamParser.h"
#include "Debugger.h"

namespace Dap {
struct ResponseInfo {
    std::shared_ptr<Dap::Protocol::Response> threads;

    // key - threadId
    std::map<int64_t, std::shared_ptr<Dap::Protocol::Response>> stackTrace;

    // key - frameId
    std::map<int64_t, std::shared_ptr<Dap::Protocol::Response>> scopes;

    // key - variablesReference,start
    std::map<std::pair<int64_t, int64_t>, std::shared_ptr<Dap::Protocol::Response>> variables;
};

struct RecordNode {
    unsigned int id = 0;
    std::string dataTimeStamp;
    uint64_t timeStamp = 0;
    Dap::Protocol::StoppedEventBody stoppedEventBody;
    std::string reason;
    int64_t threadId = -1;
    std::string threadName;
    ResponseInfo responses;
    Dap::Protocol::SteppingGranularity steppingGranularity = Dap::Protocol::SteppingGranularity::LINE;
    int64_t coroutineId = -1;

    Dap::Protocol::Thread GetStoppedThread() const;
    Dap::Protocol::StackFrame GetTopFrame() const;
};

class TimeTravelDebugManager {
public:
    explicit TimeTravelDebugManager(std::shared_ptr<Debugger> debugger);
    ~TimeTravelDebugManager() = default;

    bool IsUndoEnable() const;
    std::shared_ptr<Dap::Protocol::Response>
    GetDebugInfoResponseInUndo(const RequestArgumentsContent &content, const std::string &command, unsigned int seq);
    std::shared_ptr<Dap::Protocol::Response>
    GetResponseFromLatestNode(const RequestArgumentsContent &content, const std::string &command, unsigned int seq);
    void RecordResponse(RequestArgumentsContent content, std::string &command,
                        std::shared_ptr<Dap::Protocol::Response> res);
    void CreateRecordNode(const std::string &reason);
    void CreateRecordNode(const Dap::Protocol::StoppedEventBody &body, const int64_t coroutineId = -1);
    void SelectRecordNode(const std::string &key, bool isGotoType = false);

    RequestArgumentsContent GetArgumentsContent(Dap::Protocol::Request *req) const;

    void StateSwitchProcess(bool flag);
    bool IsAllowedMessagesInUndo(const std::string &command) const;
    bool RequestNeedRecord(const std::string &command) const;
    bool IsTriggeredUndoEvent(const std::string &command) const;
    bool IsNotAllowedInNormal(const std::string &command) const;
    bool IsAvailableOnCurrentContainer() const;
    void SetCurrentContainerAvailable(bool status);
    std::vector<Dap::Protocol::RecordPoint> GetSupportedRecordPoints(uint64_t start = 0, uint64_t count = 0);
    int GetCurrentNodeId() const;
    bool GotoRecordPoint(int id);
    bool IsSupportedRecordPointType(Dap::Protocol::StoppedReason reason) const;
    void UpdateVariablesCache(const std::string &varName, const int &parentRef, const std::string &newValue);

    std::shared_ptr<RecordNode> GetRecordNodeById(int recordPointId) const
    {
        int index = GetRecordNodeIndexById(recordPointId);
        if (index == -1 || (size_t)index >= recordNodeQueue.size()) {
            return nullptr;
        }
        return recordNodeQueue[index];
    }

    std::shared_ptr<RecordNode> GetCurrentRecordNode() const
    {
        if (recordNodeQueue.size() > 0) {
            return recordNodeQueue[currentRecordNodeIndex];
        }
        return nullptr;
    }

    std::vector<std::string> GetSupportedRecordPointTypes() const
    {
        return supportedNodeTypeArray;
    }

    void SetFilters(std::vector<std::string> filters)
    {
        recordNodeFilters = filters;
    }

    uint64_t GetRecodeNodeQueueSize()
    {
        if (!recordNodeQueue.empty()) {
            return recordNodeQueue.size();
        }
        return 0;
    }

    Protocol::RecordPointsUpdateEventBody GetRecordPointsUpdateEventBody();

    void ClearRecordPointsUpdateCache()
    {
        std::lock_guard<std::mutex> lock(mutex);
        removedRecordPointIds.clear();
        newRecordPointIds.clear();
    }

private:
    unsigned int GetAndIncRecordId()
    {
        std::lock_guard<std::mutex> lock(recordIdMutex);
        currentRecordId++;
        if (currentRecordId > RECORD_ID_MAX) {
            currentRecordId = (currentRecordId) % RECORD_ID_MAX;
            cacheOverflow = true;
        }
        return currentRecordId;
    }

    void Configure();
    void SetRecordNodeContainer(const std::shared_ptr<RecordNode> &node);
    static void SetThreadsHandler(const std::shared_ptr<RecordNode> &node,
                                  const std::shared_ptr<Dap::Protocol::Response> &res,
                                  const RequestArgumentsContent &content);
    static void SetStackTraceHandler(const std::shared_ptr<RecordNode> &node,
                                     const std::shared_ptr<Dap::Protocol::Response> &res,
                                     RequestArgumentsContent content);
    static void SetScopesHandler(const std::shared_ptr<RecordNode> &node,
                                 const std::shared_ptr<Dap::Protocol::Response> &res,
                                 RequestArgumentsContent content);
    static void SetVariablesHandler(const std::shared_ptr<RecordNode> &node,
                                    const std::shared_ptr<Dap::Protocol::Response> &res,
                                    RequestArgumentsContent content);
    static std::shared_ptr<Dap::Protocol::Response> GetThreadsHandler(const std::shared_ptr<RecordNode> &node,
                                                                      const RequestArgumentsContent &content);
    static std::shared_ptr<Dap::Protocol::Response> GetStackTraceHandler(const std::shared_ptr<RecordNode> &node,
                                                                         RequestArgumentsContent content);
    static std::shared_ptr<Dap::Protocol::Response> GetScopesHandler(const std::shared_ptr<RecordNode> &node,
                                                                     RequestArgumentsContent content);
    static std::shared_ptr<Dap::Protocol::Response> GetVariablesHandler(const std::shared_ptr<RecordNode> &node,
                                                                        RequestArgumentsContent content);

    void SelectBreakpointRecordNode(bool forward);
    void AdjustRecordContainerOnStopUndo();
    std::shared_ptr<Dap::Protocol::Response> GetResponse(const std::string &command,
                                                         const RequestArgumentsContent &content);
    void RecordPointStorage(std::deque<std::shared_ptr<RecordNode>> *container,
                            const std::shared_ptr<RecordNode> &node) const;
    bool IsBreakpoint(const std::string &reason) const;
    int GetRecordNodeIndexById(int id) const;
    bool IsSupportedRecordPointType(std::string type) const;

private:
    const std::vector<std::string> recordRequests = {Protocol::REQ_RES_THREADS, Protocol::REQ_RES_STACK_TRACE,
                                                     Protocol::REQ_RES_SCOPES, Protocol::REQ_RES_VARIABLES};
    const std::vector<std::string> allowedMessagesInUndo = {Protocol::REQ_RES_UNDO,
        Protocol::REQ_RES_REVERSE_STEP, Protocol::REQ_RES_REVERSE_CONTINUE,
        Protocol::REQ_RES_CONTINUE_IN_REVERSE, Protocol::REQ_RES_STEP_IN_REVERSE,
        Protocol::REQ_RES_DISCONNECT, Protocol::REQ_RES_RECORD_POINTS,
        Protocol::REQ_RES_GOTO_RECORD_POINT, Protocol::REQ_RES_DISASSEMBLE, Protocol::REQ_RES_SET_BREAKPOINTS,
        Protocol::REQ_RES_SET_FUNCTION_BREAKPOINTS, Protocol::REQ_RES_SET_DATA_BREAKPOINTS,
        Protocol::REQ_RES_SET_INSTRUCTION_BREAKPOINTS, Protocol::REQ_RES_SET_EXCEPTION_BREAKPOINTS,
        Protocol::REQ_RES_CACHE_DATA_CONFIG, Protocol::REQ_RES_SOURCE, Protocol::REQ_RES_RECORD_MESSAGE};
    const std::vector<std::string> triggeredUndoEvents = {Protocol::REQ_RES_REVERSE_STEP,
                                                          Protocol::REQ_RES_REVERSE_CONTINUE,
                                                          Protocol::REQ_RES_CONTINUE_IN_REVERSE,
                                                          Protocol::REQ_RES_STEP_IN_REVERSE,
                                                          Protocol::REQ_RES_GOTO_RECORD_POINT};
    const std::vector<std::string> notAllowedInNormal = {Protocol::REQ_RES_REVERSE_STEP,
                                                         Protocol::REQ_RES_REVERSE_CONTINUE,
                                                         Protocol::REQ_RES_CONTINUE_IN_REVERSE,
                                                         Protocol::REQ_RES_STEP_IN_REVERSE};
    using SetResponseHandler = std::function<void(std::shared_ptr<RecordNode>,
                                                  std::shared_ptr<Dap::Protocol::Response>,
                                                  RequestArgumentsContent)>;
    using GetResponseHandler = std::function<std::shared_ptr<Dap::Protocol::Response>(std::shared_ptr<RecordNode>,
                                                                                      RequestArgumentsContent)>;
    std::map<std::string, SetResponseHandler> setResponseHandlerMap;
    std::map<std::string, GetResponseHandler> getResponseHandlerMap;
    std::deque<std::shared_ptr<RecordNode>> recordNodeQueue;
    unsigned int currentRecordNodeIndex = 0;
    std::vector<std::string> supportedNodeTypeArray;
    std::vector<std::string> recordNodeFilters;

    std::mutex mutex;
    std::mutex recordIdMutex;
    bool isConfigure = false;
    bool undoEnable = false;
    bool isAvailableOnCurrentContainer = true;
    bool isGotoRecordPointType = false;
    unsigned int currentRecordId = 0;
    std::vector<int64_t> removedRecordPointIds;
    std::vector<int64_t> newRecordPointIds;
    bool cacheOverflow = false;
    std::string currentWorkKey = "reverse step";

    static constexpr int RECORD_ID_MAX = 10000;
    const std::string STEP = "step";
    const std::string REVERSE_STEP = "reverse step";
    const std::string BREAKPOINT = "breakpoint";
    const std::string REVERSE_BREAKPOINT = "reverse breakpoint";
    std::shared_ptr<Debugger> debugger = nullptr;
};
};

#endif // DAP_TIME_TRAVEL_DEBUG_MANAGER_H