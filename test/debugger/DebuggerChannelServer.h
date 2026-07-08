// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUGGERCHANNELSERVER_H
#define DAP_SERVER_DEBUGGERCHANNELSERVER_H

#include "ChannelServer.h"

class DebuggerChannelServer : public Dap::ChannelServer {
public:
    explicit DebuggerChannelServer(const std::string &name);

    ~DebuggerChannelServer() override;

    bool Start() override;

    bool Stop() override;

    bool Accept(const std::shared_ptr<Dap::Channel> channel) override;

    int Send(const std::uint32_t &channelId, const std::string &data) override;

private:
    std::string name;
};
#endif // DAP_SERVER_DEBUGGERCHANNELSERVER_H
