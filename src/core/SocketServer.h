// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_SOCKET_SERVER_H
#define DAP_SERVER_SOCKET_SERVER_H

#include <memory>
#include <vector>
#include "DebugSession.h"
#include "ChannelServer.h"
#include "ChannelServerHandler.h"
#include "ChannelServerThread.h"
#include "Option.h"
#include "SessionHandler.h"
#include "ChannelFactory.h"

namespace Dap {
class SocketServer : public Dap::ChannelServerHandler, public SessionHandler,
    public std::enable_shared_from_this<SocketServer> {
public:
    explicit SocketServer(const Dap::Option &option);
    ~SocketServer() override = default;

    std::weak_ptr<SocketServer> GetSocketServerWeakPtr()
    {
        return weak_from_this();
    }
    virtual void Initialize()
    {
        channelServer->RegChannelServerHandler(shared_from_this());
        socketChannelServerThread = Dap::ChannelFactory::CreateChannelServerThread(channelServer);
    }
    bool Start();
    bool Stop();
    bool IsStarted() const
    {
        return started;
    }
    bool AllSessionExited() const
    {
        return (sessionCreated && sessions.empty());
    }

    void OnChannelAccepted(const int &channelServerId, const int &channelId) override;
    void OnChannelServerError(const int &channelServerId, const int &what, const std::string &error) override;
    void OnSessionClosed(int sessionID) override;

protected:
    static bool sessionCreated;
    Dap::Option globalOption;
    std::shared_ptr<Dap::ChannelServer> channelServer;
    std::vector<std::shared_ptr<DebugSession>> sessions;
    bool started;
    std::shared_ptr<Dap::ChannelServerThread> socketChannelServerThread;
};
}

#endif // DAP_SERVER_SOCKET_SERVER_H