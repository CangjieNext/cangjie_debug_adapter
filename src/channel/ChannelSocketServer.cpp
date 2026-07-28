// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelSocketServer.h"
#include "UvDefs.h"
#include "UvLoopMgr.h"
#include "ChannelCallbacks.h"

using namespace Dap;

ChannelSocketServer::ChannelSocketServer(const std::string &host, const int &port) : host(host), port(port) {}

ChannelSocketServer::~ChannelSocketServer()
{
    this->Stop();
}

bool ChannelSocketServer::Initialize()
{
    uvTcpServer.arg = this;
    int ret = uv_tcp_init(UvLoopMgr::Instance().Loop(loopId), &uvTcpServer.handle);
    if (ret != 0) {
        return false;
    }
    return true;
}

void ChannelSocketServer::UnInitialize()
{
    this->ShutDown(reinterpret_cast<uv_stream_t *>(&uvTcpServer.handle));
    this->CloseStream(reinterpret_cast<uv_handle_t *>(&uvTcpServer.handle));
}

bool ChannelSocketServer::Start()
{
    if (isStart) {
        return true;
    }
    if (!this->Initialize()) {
        this->OnServerError(UV_RET_FAIL, "Channel Server - socket server: Initialize error.");
        return false;
    }
    sockaddr_in addr;
    if (uv_ip4_addr(host.c_str(), port, &addr) != 0) {
        this->OnServerError(UV_RET_FAIL, "Channel Server - socket server: Get ip address error.");
        return false;
    }
    int ret = uv_tcp_bind(&uvTcpServer.handle, (const struct sockaddr *)&addr, 0);
    if (ret != 0) {
        this->OnServerError(UV_RET_FAIL, "Channel Server - socket server: Bind error.");
        return false;
    }
    ret = uv_listen((uv_stream_t *)&uvTcpServer.handle, UV_BACKLOG, ChannelCallbacks::ListenCbSocket);
    if (ret != 0) {
        this->OnServerError(UV_RET_FAIL, "Channel Server - socket server: Listen error.");
        return false;
    }
    isStart = true;
    return true;
}

bool ChannelSocketServer::Stop()
{
    if (!isStart) {
        return true;
    }
    this->RemoveAll();
    this->UnInitialize();
    isStart = false;
    return true;
}

bool ChannelSocketServer::Accept(const std::shared_ptr<Channel> channel)
{
    if (!channel->Connect()) {
        return false;
    }
    if (channelMap.count(channel->Id()) > 0) {
        if (!channelMap.at(channel->Id())->IsConnected()) {
            this->Remove(channel->Id());
        } else {
            channel->Close();
            this->OnServerError(UV_RET_FAIL,
                "Channel Server - socket server: The channel with same id has already exist.");
            return false;
        }
    }
    channel->RegChannelHandler(channelHandler);
    channelMap[channel->Id()] = channel;
    this->OnAccept(channel->Id());
    return true;
}

int ChannelSocketServer::Send(const std::uint32_t &channelId, const std::string &data)
{
    if (channelMap.count(channelId) == 0) {
        this->OnServerError(UV_RET_FAIL, "ChannelServer - tcp server: Can not find channel.");
        return UV_RET_FAIL;
    }
    return channelMap[channelId]->Send(data);
}