// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "ChannelBaseSuite.h"
#include "MockChannel.h"
#include "MockChannelServer.h"
#include "ChannelServerThread.h"
#include "ChannelFactory.h"

using namespace ::testing;
using namespace Dap;

class ChannelServerThreadSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        mockChannel = std::make_shared<MockChannel>();
        mockChannelServer = std::make_shared<MockChannelServer>();
        channelServerThread = ChannelFactory::CreateChannelServerThread(mockChannelServer);
    }

    void TearDown() override {}

protected:
    std::shared_ptr<MockChannel> mockChannel = nullptr;
    std::shared_ptr<MockChannelServer> mockChannelServer = nullptr;
    std::shared_ptr<ChannelServerThread> channelServerThread = nullptr;
};

TEST_F(ChannelServerThreadSuite, start)
{
    EXPECT_CALL(*mockChannel, Connect()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Start()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Stop()).WillRepeatedly(Return(true));
    channelServerThread->Start();
    EXPECT_TRUE(channelServerThread->IsStart());
    channelServerThread->Start();
    EXPECT_TRUE(channelServerThread->IsStart());
    channelServerThread->Stop();
    EXPECT_FALSE(channelServerThread->IsStart());
}

TEST_F(ChannelServerThreadSuite, is_start)
{
    EXPECT_FALSE(channelServerThread->IsStart());
}

TEST_F(ChannelServerThreadSuite, server_thread_id)
{
    EXPECT_TRUE(channelServerThread->ThreadId() >= 0);
}

TEST_F(ChannelServerThreadSuite, server_thread_channel_server_id)
{
    EXPECT_TRUE(channelServerThread->ChannelServerId() >= 0);
}

TEST_F(ChannelServerThreadSuite, server_thread_get_channel_server)
{
    EXPECT_TRUE(channelServerThread->GetChannelServer() == mockChannelServer);
}

TEST_F(ChannelServerThreadSuite, server_thread_get_channel)
{
    mockChannelServer->SetAcceptEnable(true);
    mockChannelServer->Accept(mockChannel);
    EXPECT_TRUE(channelServerThread->GetChannel(-1) == nullptr);
    EXPECT_TRUE(channelServerThread->GetChannel(mockChannel->Id()) == mockChannel);
}