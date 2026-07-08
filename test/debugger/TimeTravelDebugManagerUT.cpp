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

std::shared_ptr<TimeTravelDebugManager> GetTimeTravelDebugManager()
{
    return std::make_shared<TimeTravelDebugManager>(nullptr);
}

class TimeTravelDebugManagerUT : public testing::Test {
protected:
    void SetUp() override {};
    void TearDown() override {};

protected:
    std::shared_ptr<TimeTravelDebugManager> managerTest;

    RequestArgumentsContent stackTraceArgCont;
    RequestArgumentsContent scopesArgCont;
    RequestArgumentsContent variablesArgCont;

    StackTraceArguments stackTraceArg;
    ScopesArguments scopesArg;
    VariablesArguments variablesArg;

    std::string threadsCmd = REQ_RES_THREADS;
    std::string stackTraceCmd = REQ_RES_STACK_TRACE;
    std::string scopesCmd = REQ_RES_SCOPES;
    std::string variablesCmd = REQ_RES_VARIABLES;
    std::shared_ptr<Response> respThreads = std::make_shared<Response>(REQ_RES_THREADS);
    std::shared_ptr<Response> respScopes = std::make_shared<Response>(REQ_RES_SCOPES);
    std::shared_ptr<Response> respStackTrace = std::make_shared<Response>(REQ_RES_STACK_TRACE);
    std::shared_ptr<Response> respVariables = std::make_shared<Response>(REQ_RES_VARIABLES);
};

TEST_F(TimeTravelDebugManagerUT, is_allowed_messages_in_undo)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_UNDO));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_DISCONNECT));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(managerTest->IsAllowedMessagesInUndo(REQ_RES_STEP_IN));
}

TEST_F(TimeTravelDebugManagerUT, is_triggered_undo_event)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(managerTest->IsTriggeredUndoEvent(REQ_RES_STEP_IN));
}

TEST_F(TimeTravelDebugManagerUT, is_not_allowed_in_normal)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(managerTest->IsNotAllowedInNormal(REQ_RES_NEXT));
}

TEST_F(TimeTravelDebugManagerUT, RecordThread)
{
    managerTest = GetTimeTravelDebugManager();
    Dap::Protocol::StoppedEventBody body;
    body.threadId = 1;
    body.reason = StoppedReason::BREAKPOINT;
    body.allThreadsStopped = true;
    managerTest->CreateRecordNode(body);

    RequestArgumentsContent threadsContent;
    ThreadsResponse threadsResponse;
    threadsResponse.success = true;
    Thread thread;
    thread.id = 1;
    thread.name = "main";
    threadsResponse.threads.push_back(thread);
    thread.id = 2;
    thread.name = "bugfix";
    threadsResponse.threads.push_back(thread);
    std::shared_ptr<ThreadsResponse> ThreadsPtr = std::make_shared<ThreadsResponse>(threadsResponse);
    managerTest->RecordResponse(threadsContent, threadsCmd, ThreadsPtr);

    string threadName = managerTest->GetCurrentRecordNode()->threadName;
    EXPECT_EQ(threadName, "main");
}

TEST_F(TimeTravelDebugManagerUT,GetDebugInfoResponseInUndo)
{
    managerTest = GetTimeTravelDebugManager();
    unsigned int seq = 10;

    stackTraceArgCont.content = stackTraceArg;
    scopesArgCont.content = scopesArg;
    variablesArgCont.content = variablesArg;

    std::string testStr = "1234";
    managerTest->CreateRecordNode("breakpoint");
    managerTest->RecordResponse(stackTraceArgCont,stackTraceCmd,respStackTrace);
    managerTest->RecordResponse(scopesArgCont,scopesCmd,respScopes);
    managerTest->CreateRecordNode("step");
    managerTest->RecordResponse(scopesArgCont,scopesCmd,respScopes);
    managerTest->CreateRecordNode("breakpoint");
    managerTest->RecordResponse(variablesArgCont,variablesCmd,respVariables);

    managerTest->SelectRecordNode("step");
    managerTest->SelectRecordNode("reverse step");
    managerTest->SelectRecordNode("breakpoint");
    managerTest->SelectRecordNode("reverse breakpoint");

    EXPECT_TRUE(managerTest->GetDebugInfoResponseInUndo(stackTraceArgCont,testStr,seq).get() == nullptr);
    EXPECT_TRUE(managerTest->GetDebugInfoResponseInUndo(stackTraceArgCont,stackTraceCmd,seq)->request_seq == seq);

    managerTest->SelectRecordNode("step");
    EXPECT_TRUE(managerTest->GetDebugInfoResponseInUndo(scopesArgCont,scopesCmd,seq)->command == scopesCmd);
}

TEST_F(TimeTravelDebugManagerUT,GetArgumentsContent)
{
    managerTest = GetTimeTravelDebugManager();

    StackTraceRequest* stackTraceReq = new StackTraceRequest();
    RequestArgumentsContent cont = managerTest->GetArgumentsContent(stackTraceReq);
    StackTraceArguments stackTraceArg = std::get<0>(cont.content);
    EXPECT_TRUE(stackTraceReq->arguments.threadId == stackTraceArg.threadId);

    ScopesRequest* scopesReq = new ScopesRequest();
    cont = managerTest->GetArgumentsContent(scopesReq);
    ScopesArguments scopesArg = std::get<1>(cont.content);
    EXPECT_TRUE(scopesReq->arguments.frameId == scopesArg.frameId);

    VariablesRequest* variablesReq = new VariablesRequest();
    cont = managerTest->GetArgumentsContent(variablesReq);
    VariablesArguments variablesArg = std::get<2>(cont.content);
    EXPECT_TRUE(variablesReq->arguments.variablesReference == variablesArg.variablesReference);

    delete stackTraceReq;
    delete scopesReq;
    delete variablesReq;
}

TEST_F(TimeTravelDebugManagerUT,StateSwitchProcess)
{
    managerTest = GetTimeTravelDebugManager();
    managerTest->StateSwitchProcess(false);
    bool flag = managerTest->IsUndoEnable();
    EXPECT_TRUE(flag == false);

    managerTest->StateSwitchProcess(true);
    flag = managerTest->IsUndoEnable();
    EXPECT_TRUE(flag == true);
}

TEST_F(TimeTravelDebugManagerUT,IsAllowedMessagesInUndo)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_UNDO));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_DISCONNECT));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_DISASSEMBLE));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsAllowedMessagesInUndo(REQ_RES_STEP_IN_REVERSE));
}

TEST_F(TimeTravelDebugManagerUT,IsTriggeredUndoEvent)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsTriggeredUndoEvent(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(managerTest->IsTriggeredUndoEvent(REQ_RES_UNDO));
}

TEST_F(TimeTravelDebugManagerUT,IsNotAllowedInNormal)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_REVERSE_STEP));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_REVERSE_CONTINUE));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_CONTINUE_IN_REVERSE));
    EXPECT_TRUE(managerTest->IsNotAllowedInNormal(REQ_RES_STEP_IN_REVERSE));
    EXPECT_FALSE(managerTest->IsNotAllowedInNormal(REQ_RES_UNDO));
}

TEST_F(TimeTravelDebugManagerUT, IsAvailableOnCurrentContainer)
{
    managerTest = GetTimeTravelDebugManager();
    EXPECT_TRUE(managerTest->IsAvailableOnCurrentContainer());
}

TEST_F(TimeTravelDebugManagerUT, GotoRecordPoint)
{
    managerTest = GetTimeTravelDebugManager();
    bool success = managerTest->GotoRecordPoint(1);
    EXPECT_FALSE(success);
    managerTest->CreateRecordNode("breakpoint");

    stackTraceArgCont.content = stackTraceArg;
    scopesArgCont.content = scopesArg;
    variablesArgCont.content = variablesArg;
    managerTest->RecordResponse(stackTraceArgCont,stackTraceCmd,respStackTrace);

    success = managerTest->GotoRecordPoint(-1);
    EXPECT_FALSE(success);
    success = managerTest->GotoRecordPoint(1);
    EXPECT_TRUE(success);
}

TEST_F(TimeTravelDebugManagerUT, GetSupportedRecordPointTypes)
{
    managerTest = GetTimeTravelDebugManager();
    auto array = managerTest->GetSupportedRecordPointTypes();
    EXPECT_TRUE(array.size() > 0);
}

TEST_F(TimeTravelDebugManagerUT, GetSupportedRecordPoints)
{
    managerTest = GetTimeTravelDebugManager();
    managerTest->CreateRecordNode("breakpoint");
    managerTest->CreateRecordNode("my type");
    auto it = managerTest->GetSupportedRecordPoints();
    EXPECT_EQ(it.size(), 1);
}