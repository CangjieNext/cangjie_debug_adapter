// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "iostream"
#include "DebuggerChannelServer.h"

DebuggerChannelServer::DebuggerChannelServer(const std::string &name) : name(name) {}

DebuggerChannelServer::~DebuggerChannelServer() {}

bool DebuggerChannelServer::Start()
{
    return true;
}

bool DebuggerChannelServer::Stop()
{
    return true;
}

bool DebuggerChannelServer::Accept(const std::shared_ptr<Dap::Channel> channel)
{
    channelMap[-1] = channel;
    return true;
}

int DebuggerChannelServer::Send(const std::uint32_t &channelId, const std::string &data)
{
    (void)channelId;
    (void)data;
    return 0;
}