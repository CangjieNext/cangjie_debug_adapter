// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_SOCKET_SERVER_H
#define DAP_CHANNEL_SOCKET_SERVER_H

#include "ChannelServer.h"

namespace Dap {
class ChannelSocketServer : public ChannelServer {
public:
    ChannelSocketServer(const std::string &host, const int &port);

    ~ChannelSocketServer() override;

    bool Start() override;

    bool Stop() override;

    bool Accept(const std::shared_ptr<Channel> channel) override;

    int Send(const std::uint32_t &channelId, const std::string &data) override;

protected:
    virtual bool Initialize();

    virtual void UnInitialize();

private:
    UvTcp uvTcpServer;
    std::string host;
    int port;
};
}

#endif // DAP_CHANNEL_SOCKET_SERVER_H
