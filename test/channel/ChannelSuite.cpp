// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#include <gtest/gtest.h>
#include "ChannelBaseSuite.h"
#include "MockChannel.h"
#include "ChannelHandler.h"

using namespace ::testing;
using namespace Dap;

namespace {
constexpr int EXPECTED_MESSAGE_COUNT = 2;
}

class MockChannelHandler : public ChannelHandler {
public:
    MockChannelHandler() {}
    ~MockChannelHandler() = default;

    void OnChannelData(const int &channelId, const std::string &data) override
    {
        (void)channelId;
        (void)data;
    }

    void OnChannelError(const int &channelId, int what, const std::string &error) override
    {
        (void)channelId;
        (void)error;
        if (what == 0) {
            msgCnt++;
        } else {
            errorCnt++;
        }
    }

    void OnChannelClose(const int &channelId) override
    {
        (void)channelId;
    }

    int ErrorCount() const
    {
        return errorCnt;
    }

    int MsgCount() const
    {
        return msgCnt;
    }

private:
    int msgCnt = 0;
    int errorCnt = 0;
};

class ChannelSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        channel = std::make_shared<MockChannel>();
        handler = std::make_shared<MockChannelHandler>();
        channel->RegChannelHandler(handler);
    }

    void TearDown() override {}

protected:
    std::shared_ptr<MockChannel> channel;
    std::shared_ptr<MockChannelHandler> handler;
};

TEST_F(ChannelSuite, on_error)
{
    channel->OnError(-1, "error message.");
    EXPECT_EQ(handler->ErrorCount(), 1);
}

TEST_F(ChannelSuite, on_message)
{
    channel->OnError(0, "normal message.");
    EXPECT_EQ(handler->MsgCount(), 1);
    channel->OnMessage("normal message.");
    EXPECT_EQ(handler->MsgCount(), EXPECTED_MESSAGE_COUNT);
}

TEST_F(ChannelSuite, loop_id)
{
    const int loopId = 0;
    channel->SetLoopId(loopId);
    EXPECT_EQ(channel->GetLoopId(), loopId);
}
