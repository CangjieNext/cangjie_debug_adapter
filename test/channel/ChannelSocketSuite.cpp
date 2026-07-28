// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#include <string>
#include <map>
#include "ChannelBaseSuite.h"
#include "Channel.h"
#include "ChannelThread.h"
#include "ChannelFactory.h"
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

class ChannelSocketSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        channelSocket = ChannelFactory::CreateChannelSocket(host, port);
        channelHandler = std::make_shared<ChannelSocketHandler>();
        channelSocket->RegChannelHandler(channelHandler);
        channelThread = ChannelFactory::CreateChannelThread(channelSocket);

        channelSocketServer = ChannelFactory::CreateChannelSocketServer(host, port);
        channelServerHandler = std::make_shared<ChannelSocketServerHandler>();
        channelSocketServer->RegChannelHandler(channelHandler);
        channelSocketServer->RegChannelServerHandler(channelServerHandler);
        channelServerThread = ChannelFactory::CreateChannelServerThread(channelSocketServer);
    }

    void TearDown() override
    {
        StopAll();
    }

    void StopAll()
    {
        const int startMillis = 200;
        channelThread->Stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(startMillis));
        channelServerThread->Stop();
    }

    void StartAll()
    {
        const int channelStartMillis = 200;
        const int channelServerStartMillis = 500;
        channelServerThread->Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(channelServerStartMillis));
        channelThread->Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(channelStartMillis));
    }

protected:
    std::shared_ptr<Channel> channelSocket = nullptr;
    std::shared_ptr<ChannelHandler> channelHandler = nullptr;
    std::shared_ptr<ChannelThread> channelThread = nullptr;
    std::shared_ptr<ChannelServer> channelSocketServer = nullptr;
    std::shared_ptr<ChannelServerHandler> channelServerHandler = nullptr;
    std::shared_ptr<ChannelServerThread> channelServerThread = nullptr;
    std::string host = "127.0.0.1";
    int port = 9999;
};

TEST_F(ChannelSocketSuite, channel_socket_server_start)
{
    const int startMillis = 200;
    channelServerThread->Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(startMillis));
    EXPECT_TRUE(channelSocketServer->IsStart());
}

TEST_F(ChannelSocketSuite, channel_socket_connect)
{
    StartAll();
    EXPECT_TRUE(channelSocket->IsConnected());
}

TEST_F(ChannelSocketSuite, channel_socket_async_send)
{
    StartAll();
    ASSERT_TRUE(channelSocket->IsConnected());
    channelSocket->AsyncSend("Hello socket channel, async send\n");
}

TEST_F(ChannelSocketSuite, channel_socket_async_send_empty)
{
    StartAll();
    ASSERT_TRUE(channelSocket->IsConnected());
    channelSocket->AsyncSend("");
}

TEST_F(ChannelSocketSuite, channel_socket_send_not_connect)
{
    MockChannelSocket channel(host, port);
    channel.SetConnectStatus(false);
    ASSERT_FALSE(channel.IsConnected());
    channel.Send("Hello socket channel, send\n");
}

TEST_F(ChannelSocketSuite, channel_socket_send_empty)
{
    MockChannelSocket channel(host, port);
    channel.SetConnectStatus(true);
    ASSERT_TRUE(channel.IsConnected());
    channel.Send("");
}

TEST_F(ChannelSocketSuite, channel_socket_server_send_no_channel_id)
{
    HelpChannelSocketServer channelServer(host, port);
    channelServer.Send(-1, "");
}

TEST_F(ChannelSocketSuite, channel_socket_server_send_by_channel_id)
{
    std::shared_ptr<MockChannelSocket> channel = std::make_shared<MockChannelSocket>(host, port);
    EXPECT_CALL(*channel, Connect()).WillRepeatedly(Return(true));
    std::shared_ptr<HelpChannelSocketServer> channelServer = std::make_shared<HelpChannelSocketServer>(host, port);
    channelServer->Accept(channel);
    channelServer->Send(channel->Id(), "");
}

TEST_F(ChannelSocketSuite, channel_socket_close)
{
    StartAll();
    ASSERT_TRUE(channelSocket->IsConnected());
    channelThread->Stop();  // if (isConnected)
    channelSocket->Close(); // if (!isConnected)
    EXPECT_FALSE(channelSocket->IsConnected());
}

TEST_F(ChannelSocketSuite, channel_socket_server_close)
{
    StartAll();
    ASSERT_TRUE(channelSocketServer->IsStart());
    channelServerThread->Stop(); // if (isStart)
    channelSocketServer->Stop(); // if (!isStart)
}

TEST_F(ChannelSocketSuite, channel_socket_server_accept_channel)
{
    std::shared_ptr<MockChannelSocket> channel = std::make_shared<MockChannelSocket>(host, port);
    EXPECT_CALL(*channel, Connect())
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*channel, Close()).WillRepeatedly(Return(true));
    HelpChannelSocketServer channelServer(host, port);
    bool res = channelServer.Accept(channel);
    EXPECT_FALSE(res);
    channel->SetConnectStatus(true);
    res = channelServer.Accept(channel);
    EXPECT_TRUE(res);
    channel->SetConnectStatus(false);
    res = channelServer.Accept(channel);
    EXPECT_TRUE(res);
    channel->SetConnectStatus(true);
    res = channelServer.Accept(channel);
    EXPECT_FALSE(res);
}

TEST_F(ChannelSocketSuite, channel_socket_connect_failed)
{
    std::shared_ptr<HelpChannelSocket> channel = std::make_shared<HelpChannelSocket>(host, port);
    channel->SetConnectStatus(true);
    bool res = channel->Connect();
    EXPECT_TRUE(res);
    channel->SetConnectStatus(false);
    channel->SetInitializeFlag(false);
    res = channel->Connect();
    EXPECT_FALSE(res);
}

TEST_F(ChannelSocketSuite, channel_socket_in_server_connect_failed)
{
    UvTcp server;
    std::shared_ptr<HelpChannelSocket> channel = std::make_shared<HelpChannelSocket>(server);
    channel->SetConnectStatus(true);
    bool res = channel->Connect();
    EXPECT_TRUE(res);
    channel->SetConnectStatus(false);
    channel->SetInitializeFlag(false);
    res = channel->Connect();
    EXPECT_FALSE(res);
}

TEST_F(ChannelSocketSuite, channel_socket_server_start_failed)
{
    std::shared_ptr<HelpChannelSocketServer> channelServer = std::make_shared<HelpChannelSocketServer>(host, port);
    channelServer->SetStartStatus(true);
    bool res = channelServer->Start();
    EXPECT_TRUE(res);
    channelServer->SetStartStatus(false);
    channelServer->SetInitializeFlag(false);
    res = channelServer->Start();
    EXPECT_FALSE(res);
}