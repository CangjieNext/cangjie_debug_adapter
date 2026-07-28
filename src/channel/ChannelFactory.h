// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_FACTORY_H
#define DAP_CHANNEL_FACTORY_H

#include <vector>
#include "UvDefs.h"
#include "Channel.h"
#include "ChannelServer.h"
#include "ChannelThread.h"
#include "ChannelServerThread.h"

namespace Dap {
class ChannelFactory {
public:
    static std::shared_ptr<Channel> CreateChannelSocket(const std::string &host, const int &port);

    static std::shared_ptr<Channel> CreateChannelSocket(const UvTcp &server);

    static std::shared_ptr<ChannelServer> CreateChannelSocketServer(const std::string &host, const int &port);

    static std::shared_ptr<ChannelThread> CreateChannelThread(const std::shared_ptr<Channel> channel);

    static std::shared_ptr<ChannelServerThread> CreateChannelServerThread(
        const std::shared_ptr<ChannelServer> channelServer);

private:
    ChannelFactory() = default;
    ~ChannelFactory() = default;
};
}

#endif // DAP_CHANNEL_FACTORY_H
