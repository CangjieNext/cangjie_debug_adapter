// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#include <string>
#include "ChannelBaseSuite.h"
#include "Channel.h"
#include "ChannelServer.h"
#include "ChannelContainer.h"
#include "ChannelContainerThread.h"
#include "ChannelFactory.h"
#include "ChannelHandler.h"
#include "ChannelServerHandler.h"
#include "MockChannel.h"
#include "MockChannelServer.h"

using namespace ::testing;
using namespace Dap;

class ChannelSocketHandler : public ChannelHandler {
public:
    void OnChannelData(const int &channelId, const std::string &data) override
    {
        (void)channelId;
        (void)data;
    }
    void OnChannelError(const int &channelId, int what, const std::string &error) override
    {
        (void)channelId;
        (void)what;
        (void)error;
    }
    void OnChannelClose(const int &channelId) override
    {
        (void)channelId;
    }
};

class ChannelSocketServerHandler : public ChannelServerHandler {
public:
    void OnChannelAccepted(const int &channelServerId, const int &channelId) override
    {
        (void)channelServerId;
        (void)channelId;
    }
    void OnChannelServerError(const int &channelServerId, const int &what, const std::string &error) override
    {
        (void)channelServerId;
        (void)what;
        (void)error;
    }
};

class ChannelContainerSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        channelSocket = ChannelFactory::CreateChannelSocket(host, port);
        channelSocketServer = ChannelFactory::CreateChannelSocketServer(host, port);
        channelHandler = std::make_shared<ChannelSocketHandler>();
        channelServerHandler = std::make_shared<ChannelSocketServerHandler>();
        channelSocket->RegChannelHandler(channelHandler);
        channelSocketServer->RegChannelHandler(channelHandler);
        channelSocketServer->RegChannelServerHandler(channelServerHandler);
        mockChannel = std::make_shared<MockChannel>();
        mockChannelServer = std::make_shared<MockChannelServer>();
    }

    void TearDown() override {}

protected:
    std::shared_ptr<Channel> channelSocket = nullptr;
    std::shared_ptr<ChannelServer> channelSocketServer = nullptr;
    std::shared_ptr<ChannelHandler> channelHandler = nullptr;
    std::shared_ptr<ChannelServerHandler> channelServerHandler = nullptr;
    std::shared_ptr<MockChannel> mockChannel = nullptr;
    std::shared_ptr<MockChannelServer> mockChannelServer = nullptr;
    std::string host = "127.0.0.1";
    int port = 10999;
};

TEST_F(ChannelContainerSuite, channel_container_add_channel)
{
    ChannelContainer::Instance().AddChannel(channelSocket);
    std::shared_ptr<Channel> channel = ChannelContainer::Instance().GetChannel(channelSocket->Id());
    EXPECT_EQ(channel->Id(), channelSocket->Id());
    EXPECT_EQ(channel, channelSocket);
}

TEST_F(ChannelContainerSuite, channel_container_remove_channel)
{
    ChannelContainer::Instance().AddChannel(channelSocket);
    ChannelContainer::Instance().RemoveChannel(channelSocket->Id());
    std::shared_ptr<Channel> channel = ChannelContainer::Instance().GetChannel(channelSocket->Id());
    EXPECT_EQ(channel, nullptr);
}

TEST_F(ChannelContainerSuite, channel_container_add_channel_server)
{
    ChannelContainer::Instance().AddChannelServer(channelSocketServer);
    std::shared_ptr<ChannelServer> channelServer =
        ChannelContainer::Instance().GetChannelServer(channelSocketServer->Id());
    EXPECT_EQ(channelServer->Id(), channelSocketServer->Id());
    EXPECT_EQ(channelServer, channelSocketServer);
}

TEST_F(ChannelContainerSuite, channel_container_remove_channel_server)
{
    ChannelContainer::Instance().AddChannelServer(channelSocketServer);
    ChannelContainer::Instance().RemoveChannelServer(channelSocketServer->Id());
    std::shared_ptr<ChannelServer> channelServer =
        ChannelContainer::Instance().GetChannelServer(channelSocketServer->Id());
    EXPECT_EQ(channelServer, nullptr);
}

TEST_F(ChannelContainerSuite, channel_container_thread_start)
{
    ChannelContainer::Instance().AddChannel(channelSocket);
    ChannelContainer::Instance().AddChannelServer(channelSocketServer);
    ChannelContainerThread::Instance().Start();
    EXPECT_TRUE(ChannelContainerThread::Instance().IsStart());
    ChannelContainerThread::Instance().Stop();
}

TEST_F(ChannelContainerSuite, channel_container_thread_stop)
{
    ChannelContainerThread::Instance().Stop();
    EXPECT_TRUE(!ChannelContainerThread::Instance().IsStart());
}

TEST_F(ChannelContainerSuite, channel_container_channel_get_from_server)
{
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Stop()).WillRepeatedly(Return(true));
    mockChannelServer->Accept(mockChannel);
    ChannelContainer::Instance().AddChannelServer(mockChannelServer);
    std::shared_ptr<MockChannel> channelTmp = std::dynamic_pointer_cast<MockChannel>(
            ChannelContainer::Instance().GetChannel(mockChannel->Id()));
    EXPECT_TRUE(channelTmp == mockChannel);
}

TEST_F(ChannelContainerSuite, channel_container_start)
{
    EXPECT_CALL(*mockChannel, Connect()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Start()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Stop()).WillRepeatedly(Return(true));
    ChannelContainer::Instance().AddChannel(mockChannel);
    ChannelContainer::Instance().AddChannelServer(mockChannelServer);
    ChannelContainerThread::Instance().Start();
    const int runTime = 200;
    std::this_thread::sleep_for(std::chrono::milliseconds(runTime));
    ChannelContainerThread::Instance().Stop();
}

TEST_F(ChannelContainerSuite, channel_container_server_channel_get_from_server)
{
    ChannelContainer::Instance().AddChannel(mockChannel);
    ChannelContainer::Instance().AddChannelServer(mockChannelServer);
    EXPECT_CALL(*mockChannel, Connect()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannelServer, Stop()).WillRepeatedly(Return(true));
    mockChannelServer->Accept(mockChannel);
    std::shared_ptr<MockChannel> channelTmp = std::dynamic_pointer_cast<MockChannel>(
        ChannelContainer::Instance().GetChannelFromServer(mockChannelServer->Id(), mockChannel->Id()));
    bool cmpChannel = mockChannel == channelTmp;
    EXPECT_TRUE(cmpChannel);
    cmpChannel = ChannelContainer::Instance().GetChannelFromServer(mockChannelServer->Id(), -1) == nullptr;
    EXPECT_TRUE(cmpChannel);
    cmpChannel = ChannelContainer::Instance().GetChannelFromServer(-1, -1) == nullptr;
    EXPECT_TRUE(cmpChannel);
}