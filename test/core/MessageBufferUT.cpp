// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CoreBaseSuite.h"
#include "EventManager.h"
#include "DebugSession.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

class MessageBufferUT : public CoreBaseSuite {
protected:
    void SetUp() override
    {
        requestManager.Register();
        responseManager.Register();
    }
    void TearDown() override
    {
        requestManager.UnRegister();
        responseManager.UnRegister();
    }
protected:
    Dap::Protocol::RequestManager &requestManager = Dap::Protocol::RequestManager::Instance();
    Dap::Protocol::ResponseManager &responseManager = Dap::Protocol::ResponseManager::Instance();
};

TEST_F(MessageBufferUT, Message_Request)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:56\r\n\r\n{\"seq\":5,"
                             "\"type\":\"request\",\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, Message_Response)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("Content-Length:76\r\n\r\n{\"type\":\"response\","
                             "\"command\":\"runInTerminal\",\"request_seq\":4,\"success\":true,\"seq\":1}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, Message_Null)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, Message_Event)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:54\r\n\r\n{\"seq\":5,"
                             "\"type\":\"event\",\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, Message_NullType)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:49\r\n\r\n{\"seq\":5,"
                             "\"type\":\"\",\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, Message_NoType)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:39\r\n\r\n{\"seq\":5,\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, ContentLength_Error)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("Content-Length:80\r\n\r\n{\"seq\":5,\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(CoreBaseSuite, No_LineFeed)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:40{\"seq\":5,\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}

TEST_F(MessageBufferUT, ContentLength_Right)
{
    MessageBuffer messageBuffer;
    messageBuffer.operator>>("11Content-Length:39\r\n\r\n{\"seq\"=5,\"command\":\"configurationDone\"}");
    messageBuffer.Pop();
}
