// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelFactory.h"
#include "ChannelSocket.h"
#include "ChannelSocketServer.h"

using namespace Dap;

std::shared_ptr<Channel> ChannelFactory::CreateChannelSocket(const std::string &host, const int &port)
{
    return std::shared_ptr<Channel>(std::make_shared<ChannelSocket>(host, port));
}

std::shared_ptr<Channel> ChannelFactory::CreateChannelSocket(const UvTcp &server)
{
    return std::shared_ptr<Channel>(std::make_shared<ChannelSocket>(server));
}

std::shared_ptr<ChannelServer> ChannelFactory::CreateChannelSocketServer(const std::string &host, const int &port)
{
    return std::shared_ptr<ChannelServer>(std::make_shared<ChannelSocketServer>(host, port));
}

std::shared_ptr<ChannelThread> ChannelFactory::CreateChannelThread(const std::shared_ptr<Channel> channel)
{
    return std::make_shared<ChannelThread>(channel);
}

std::shared_ptr<ChannelServerThread> ChannelFactory::CreateChannelServerThread(
    const std::shared_ptr<ChannelServer> channelServer)
{
    return std::make_shared<ChannelServerThread>(channelServer);
}