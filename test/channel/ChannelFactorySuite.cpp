// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#include <string>
#include <vector>
#include "ChannelBaseSuite.h"
#include "Channel.h"
#include "ChannelServer.h"
#include "ChannelFactory.h"

using namespace ::testing;
using namespace Dap;

class ChannelFactorySuite : public ChannelBaseSuite {
protected:
    void SetUp() override {}

    void TearDown() override {}

protected:
    std::string host = "127.0.0.1";
    int port = 10100;
#ifdef _WIN32
    const std::string pipeName = "\\\\?\\pipe\\externals";
#else
    const std::string pipeName = "/tmp/pipe-test";
#endif
    std::string processPath = "";
    std::vector<std::string> processArgs;
};

TEST_F(ChannelFactorySuite, channel_socket_create)
{
    std::shared_ptr<Channel> channel = ChannelFactory::CreateChannelSocket(host, port);
    EXPECT_NE(channel, nullptr);
}

TEST_F(ChannelFactorySuite, channel_socket_server_create)
{
    std::shared_ptr<ChannelServer> channelServer = ChannelFactory::CreateChannelSocketServer(host, port);
    EXPECT_NE(channelServer, nullptr);
}

TEST_F(ChannelFactorySuite, channel_server_thread_create)
{
    std::shared_ptr<ChannelServer> channelServer = ChannelFactory::CreateChannelSocketServer(host, port);
    std::shared_ptr<ChannelServerThread> channelServerThread = ChannelFactory::CreateChannelServerThread(channelServer);
    EXPECT_NE(channelServerThread, nullptr);
}