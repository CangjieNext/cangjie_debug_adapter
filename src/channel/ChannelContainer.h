// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_CONTAINER_H
#define DAP_CHANNEL_CONTAINER_H

#include <map>
#include <memory>
#include <mutex>
#include "Channel.h"
#include "ChannelServer.h"

namespace Dap {
class ChannelContainer {
public:
    static ChannelContainer &Instance();

    void AddChannel(const std::shared_ptr<Channel> &channel);

    void AddChannelServer(const std::shared_ptr<ChannelServer> &server);

    void RemoveChannel(const int &channelId);

    void RemoveChannelServer(const int &channelServerId);

    const std::shared_ptr<Channel> GetChannel(const int &channelId);

    const std::shared_ptr<ChannelServer> GetChannelServer(const int &channelServerId);

    const std::shared_ptr<Channel> GetChannelFromServer(const int &channelServerId, const int &channelId);

    void Start();

    void Stop();

private:
    ChannelContainer();
    ~ChannelContainer();

private:
    std::mutex channelMutex;
    std::map<std::uint32_t, std::shared_ptr<Channel>> channelMap;
    std::mutex channelServerMutex;
    std::map<std::uint32_t, std::shared_ptr<ChannelServer>> channelServerMap;
};
}

#endif // DAP_CHANNEL_CONTAINER_H
