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

ThreadsRequest GetThreadsRequest()
{
    ThreadsRequest request;
    request.seq = 7;
    return request;
}

ThreadsResponse GetThreadsResponse()
{
    ThreadsResponse response;
    response.request_seq = 7;
    response.seq = 57;
    response.success = "success";
    Thread thread1;
    thread1.id = 1;
    thread1.name = "Thread 1";
    Thread thread2;
    thread1.id = 2;
    thread1.name = "Thread 2";
    response.threads[0] = thread1;
    response.threads[1] = thread2;
    return response;
}

StackTraceRequest GetStackTraceRequest()
{
    StackTraceRequest request;
    request.seq = 8;
    request.arguments.threadId = 1;
    request.arguments.startFrame = 0;
    return request;
}

ScopesRequest GetScopesRequest()
{
    ScopesRequest request;
    request.seq = 9;
    request.arguments.frameId = 1000;
    return request;
}

VariablesRequest GetVariablesRequest()
{
    VariablesRequest request;
    request.seq = 10;
    request.arguments.variablesReference = 1005;
    return request;
}

EvaluateRequest GetEvaluateRequest()
{
    EvaluateRequest request;
    request.seq = 11;
    request.arguments.expression = "user not found";
    request.arguments.frameId = 1000;
    return request;
}

#endif //DAP_SERVER_TIMETRAVELDEBUGMANAGERUT_H
