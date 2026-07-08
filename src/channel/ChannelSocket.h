// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_SOCKET_H
#define DAP_CHANNEL_SOCKET_H

#include "Channel.h"
#include "UvDefs.h"

namespace Dap {
class ChannelSocket : public Channel {
public:
    ChannelSocket(const std::string &host, const int &port);

    explicit ChannelSocket(const UvTcp &server);

    ~ChannelSocket() override;

    bool Connect() override;

    bool Close() override;

    int Send(const std::string &data) override;

protected:
    virtual bool Initialize();

    virtual void UnInitialize();

private:
    bool IsPassive() const;

    bool ConnectActive();

    bool ConnectPassive();

    int ConnectToServer();

private:
    const UvTcp *uvTcpServer = nullptr;
    UvTcp uvTcpClient;
    std::string host;
    int port = 0;
};
}
#endif // DAP_CHANNEL_SOCKET_H
