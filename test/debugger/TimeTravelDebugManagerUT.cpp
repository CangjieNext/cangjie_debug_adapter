// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TimeTravelDebugManagerUT.h"

#include "gtest/gtest.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

class TimeTravelDebugManagerUT : public testing::Test {
protected:
    void SetUp() override
    {
        manager = std::make_shared<TimeTravelDebugManager>(nullptr);
    }

    static constexpr int64_t mainThreadId = TimeTravelDebugManagerTestData::firstThreadId;
    static constexpr int64_t secondThreadId = TimeTravelDebugManagerTestData::secondThreadId;
    static constexpr unsigned int testRequestSequence =
        TimeTravelDebugManagerTestData::variablesRequestSequence;
    static constexpr int firstRecordPointId = 1;
    static constexpr int invalidRecordPointId = -1;
    static constexpr size_t expectedSupportedRecordPointCount = 1;

    std::shared_ptr<TimeTravelDebugManager> manager;

    RequestArgumentsContent stackTraceContent;
    RequestArgumentsContent scopesContent;
    RequestArgumentsContent variablesContent;

    StackTraceArguments stackTraceArguments;
    ScopesArguments scopesArguments;
    VariablesArguments variablesArguments;

    std::string threadsCommand = REQ_RES_THREADS;
    std::string stackTraceCommand = REQ_RES_STACK_TRACE;
    std::string scopesCommand = REQ_RES_SCOPES;
    std::string variablesCommand = REQ_RES_VARIABLES;
    std::shared_ptr<Response> scopesResponse = std::make_shared<Response>(REQ_RES_SCOPES);
    std::shared_ptr<Response> stackTraceResponse = std::make_shared<Response>(REQ_RES_STACK_TRACE);
    std::shared_ptr<Response> variablesResponse = std::make_shared<Response>(REQ_RES_VARIABLES);
};

TEST_F(TimeTravelDebugManagerUT, AllowedMessagesInUndoWithUnsupportedCommand)
{
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_UNDO));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_DISCONNECT));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(manager->IsAllowedMessagesInUndo(REQ_RES_STEP_IN));
}

TEST_F(TimeTravelDebugManagerUT, TriggeredUndoEventWithUnsupportedCommand)
{
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(manager->IsTriggeredUndoEvent(REQ_RES_STEP_IN));
}

TEST_F(TimeTravelDebugManagerUT, NotAllowedInNormalWithNextCommand)
{
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(manager->IsNotAllowedInNormal(REQ_RES_NEXT));
}

TEST_F(TimeTravelDebugManagerUT, RecordThread)
{
    Dap::Protocol::StoppedEventBody body;
    body.threadId = mainThreadId;
    body.reason = StoppedReason::BREAKPOINT;
    body.allThreadsStopped = true;
    manager->CreateRecordNode(body);

    RequestArgumentsContent threadsContent;
    ThreadsResponse threadsResponse;
    threadsResponse.success = true;
    Thread thread;
    thread.id = mainThreadId;
    thread.name = "main";
    threadsResponse.threads.push_back(thread);
    thread.id = secondThreadId;
    thread.name = "bugfix";
    threadsResponse.threads.push_back(thread);
    std::shared_ptr<ThreadsResponse> threadsResponsePtr = std::make_shared<ThreadsResponse>(threadsResponse);
    manager->RecordResponse(threadsContent, threadsCommand, threadsResponsePtr);

    string threadName = manager->GetCurrentRecordNode()->threadName;
    EXPECT_EQ(threadName, "main");
}

TEST_F(TimeTravelDebugManagerUT, GetDebugInfoResponseInUndo)
{
    unsigned int sequence = testRequestSequence;

    stackTraceContent.content = stackTraceArguments;
    scopesContent.content = scopesArguments;
    variablesContent.content = variablesArguments;

    std::string unsupportedCommand = "unsupported command";
    manager->CreateRecordNode("breakpoint");
    manager->RecordResponse(stackTraceContent, stackTraceCommand, stackTraceResponse);
    manager->RecordResponse(scopesContent, scopesCommand, scopesResponse);
    manager->CreateRecordNode("step");
    manager->RecordResponse(scopesContent, scopesCommand, scopesResponse);
    manager->CreateRecordNode("breakpoint");
    manager->RecordResponse(variablesContent, variablesCommand, variablesResponse);

    manager->SelectRecordNode("step");
    manager->SelectRecordNode("reverse step");
    manager->SelectRecordNode("breakpoint");
    manager->SelectRecordNode("reverse breakpoint");

    EXPECT_EQ(manager->GetDebugInfoResponseInUndo(stackTraceContent, unsupportedCommand, sequence), nullptr);
    EXPECT_EQ(manager->GetDebugInfoResponseInUndo(stackTraceContent, stackTraceCommand, sequence)->request_seq,
              sequence);

    manager->SelectRecordNode("step");
    EXPECT_EQ(manager->GetDebugInfoResponseInUndo(scopesContent, scopesCommand, sequence)->command, scopesCommand);
}

TEST_F(TimeTravelDebugManagerUT, GetArgumentsContent)
{
    StackTraceRequest stackTraceRequest;
    RequestArgumentsContent content = manager->GetArgumentsContent(&stackTraceRequest);
    StackTraceArguments actualStackTraceArguments = std::get<StackTraceArguments>(content.content);
    EXPECT_EQ(stackTraceRequest.arguments.threadId, actualStackTraceArguments.threadId);

    ScopesRequest scopesRequest;
    content = manager->GetArgumentsContent(&scopesRequest);
    ScopesArguments actualScopesArguments = std::get<ScopesArguments>(content.content);
    EXPECT_EQ(scopesRequest.arguments.frameId, actualScopesArguments.frameId);

    VariablesRequest variablesRequest;
    content = manager->GetArgumentsContent(&variablesRequest);
    VariablesArguments actualVariablesArguments = std::get<VariablesArguments>(content.content);
    EXPECT_EQ(variablesRequest.arguments.variablesReference, actualVariablesArguments.variablesReference);
}

TEST_F(TimeTravelDebugManagerUT, StateSwitchProcess)
{
    manager->StateSwitchProcess(false);
    bool flag = manager->IsUndoEnable();
    EXPECT_FALSE(flag);

    manager->StateSwitchProcess(true);
    flag = manager->IsUndoEnable();
    EXPECT_TRUE(flag);
}

TEST_F(TimeTravelDebugManagerUT, IsAllowedMessagesInUndo)
{
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_UNDO));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_DISCONNECT));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_DISASSEMBLE));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsAllowedMessagesInUndo(REQ_RES_STEP_IN_REVERSE));
}

TEST_F(TimeTravelDebugManagerUT, IsTriggeredUndoEvent)
{
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsTriggeredUndoEvent(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(manager->IsTriggeredUndoEvent(REQ_RES_UNDO));
}

TEST_F(TimeTravelDebugManagerUT, IsNotAllowedInNormal)
{
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(manager->IsNotAllowedInNormal(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(manager->IsNotAllowedInNormal(REQ_RES_UNDO));
}

TEST_F(TimeTravelDebugManagerUT, IsAvailableOnCurrentContainer)
{
    EXPECT_TRUE(manager->IsAvailableOnCurrentContainer());
}

TEST_F(TimeTravelDebugManagerUT, GotoRecordPoint)
{
    bool success = manager->GotoRecordPoint(firstRecordPointId);
    EXPECT_FALSE(success);
    manager->CreateRecordNode("breakpoint");

    stackTraceContent.content = stackTraceArguments;
    scopesContent.content = scopesArguments;
    variablesContent.content = variablesArguments;
    manager->RecordResponse(stackTraceContent, stackTraceCommand, stackTraceResponse);

    success = manager->GotoRecordPoint(invalidRecordPointId);
    EXPECT_FALSE(success);
    success = manager->GotoRecordPoint(firstRecordPointId);
    EXPECT_TRUE(success);
}

TEST_F(TimeTravelDebugManagerUT, GetSupportedRecordPointTypes)
{
    auto recordPointTypes = manager->GetSupportedRecordPointTypes();
    EXPECT_FALSE(recordPointTypes.empty());
}

TEST_F(TimeTravelDebugManagerUT, GetSupportedRecordPoints)
{
    manager->CreateRecordNode("breakpoint");
    manager->CreateRecordNode("my type");
    auto recordPoints = manager->GetSupportedRecordPoints();
    EXPECT_EQ(recordPoints.size(), expectedSupportedRecordPointCount);
}
