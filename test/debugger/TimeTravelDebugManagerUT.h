// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_TIMETRAVELDEBUGMANAGERUT_H
#define DAP_SERVER_TIMETRAVELDEBUGMANAGERUT_H

#include "ProtocolBase.h"
#include "TimeTravelDebugManager.h"
#include "ProtocolRequests.h"
#include "EnumUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

namespace TimeTravelDebugManagerTestData {
constexpr unsigned int threadsRequestSequence = 7;
constexpr unsigned int threadsResponseSequence = 57;
constexpr unsigned int stackTraceRequestSequence = 8;
constexpr unsigned int scopesRequestSequence = 9;
constexpr unsigned int variablesRequestSequence = 10;
constexpr unsigned int evaluateRequestSequence = 11;
constexpr int64_t firstThreadId = 1;
constexpr int64_t secondThreadId = 2;
constexpr int64_t firstFrameOffset = 0;
constexpr int64_t testFrameId = 1000;
constexpr int64_t testVariablesReference = 1005;
}

inline ThreadsRequest GetThreadsRequest()
{
    ThreadsRequest request;
    request.seq = TimeTravelDebugManagerTestData::threadsRequestSequence;
    return request;
}

inline ThreadsResponse GetThreadsResponse()
{
    ThreadsResponse response;
    response.request_seq = TimeTravelDebugManagerTestData::threadsRequestSequence;
    response.seq = TimeTravelDebugManagerTestData::threadsResponseSequence;
    response.success = true;
    Thread firstThread{};
    firstThread.id = TimeTravelDebugManagerTestData::firstThreadId;
    firstThread.name = "Thread 1";
    Thread secondThread{};
    secondThread.id = TimeTravelDebugManagerTestData::secondThreadId;
    secondThread.name = "Thread 2";
    response.threads = {firstThread, secondThread};
    return response;
}

inline StackTraceRequest GetStackTraceRequest()
{
    StackTraceRequest request;
    request.seq = TimeTravelDebugManagerTestData::stackTraceRequestSequence;
    request.arguments.threadId = TimeTravelDebugManagerTestData::firstThreadId;
    request.arguments.startFrame = TimeTravelDebugManagerTestData::firstFrameOffset;
    return request;
}

inline ScopesRequest GetScopesRequest()
{
    ScopesRequest request;
    request.seq = TimeTravelDebugManagerTestData::scopesRequestSequence;
    request.arguments.frameId = TimeTravelDebugManagerTestData::testFrameId;
    return request;
}

inline VariablesRequest GetVariablesRequest()
{
    VariablesRequest request;
    request.seq = TimeTravelDebugManagerTestData::variablesRequestSequence;
    request.arguments.variablesReference = TimeTravelDebugManagerTestData::testVariablesReference;
    return request;
}

inline EvaluateRequest GetEvaluateRequest()
{
    EvaluateRequest request;
    request.seq = TimeTravelDebugManagerTestData::evaluateRequestSequence;
    request.arguments.expression = "user not found";
    request.arguments.frameId = TimeTravelDebugManagerTestData::testFrameId;
    return request;
}

#endif // DAP_SERVER_TIMETRAVELDEBUGMANAGERUT_H
