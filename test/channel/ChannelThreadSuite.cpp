// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "ChannelBaseSuite.h"
#include "MockChannel.h"
#include "ChannelThread.h"
#include "ChannelFactory.h"

using namespace ::testing;
using namespace Dap;

class ChannelThreadSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        mockChannel = std::make_shared<MockChannel>();
        channelThread = ChannelFactory::CreateChannelThread(mockChannel);
    }

    void TearDown() override {}

protected:
    std::shared_ptr<MockChannel> mockChannel = nullptr;
    std::shared_ptr<ChannelThread> channelThread = nullptr;
};

TEST_F(ChannelThreadSuite, start)
{
    EXPECT_CALL(*mockChannel, Connect()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    channelThread->Start();
    EXPECT_TRUE(channelThread->IsStart());
    channelThread->Start();
    EXPECT_TRUE(channelThread->IsStart());
    channelThread->Stop();
}

TEST_F(ChannelThreadSuite, start_already_connect)
{
    EXPECT_CALL(*mockChannel, Connect()).WillRepeatedly(Return(true));
    mockChannel->SetConnectStatus(true);
    channelThread->Start();
    EXPECT_FALSE(channelThread->IsStart());
}

TEST_F(ChannelThreadSuite, is_start)
{
    EXPECT_FALSE(channelThread->IsStart());
}

TEST_F(ChannelThreadSuite, thread_id)
{
    EXPECT_TRUE(channelThread->ThreadId() >= 0);
}

TEST_F(ChannelThreadSuite, thread_channel_id)
{
    EXPECT_TRUE(channelThread->ChannelId() >= 0);
}

TEST_F(ChannelThreadSuite, thread_get_channel)
{
    EXPECT_TRUE(channelThread->GetChannel() == mockChannel);
}