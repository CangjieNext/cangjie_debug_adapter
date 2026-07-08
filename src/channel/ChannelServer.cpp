// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelServer.h"
#include "ChannelCallbacks.h"
#include "UvLoopMgr.h"

using namespace Dap;

bool ChannelServer::IsStart() const
{
    return isStart;
}

void ChannelServer::RegChannelServerHandler(std::shared_ptr<ChannelServerHandler> handler)
{
    this->channelServerHandler = handler;
}

void ChannelServer::RegChannelHandler(std::shared_ptr<ChannelHandler> handler)
{
    this->channelHandler = handler;
}

std::shared_ptr<Channel> ChannelServer::GetChannel(const int &channelId)
{
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channelMap.count(channelId) == 0) {
        return nullptr;
    }
    return channelMap.at(channelId);
}

int ChannelServer::Id() const
{
    return id;
}

void ChannelServer::OnAccept(const int &channelId) const
{
    if (channelServerHandler != nullptr) {
        channelServerHandler->OnChannelAccepted(this->id, channelId);
    }
}

void ChannelServer::OnServerError(const int &what, const std::string &error) const
{
    if (channelServerHandler != nullptr) {
        channelServerHandler->OnChannelServerError(this->id, what, error);
    }
}

void ChannelServer::SetLoopId(const int &idNum)
{
    loopId = idNum;
}

int ChannelServer::GetLoopId() const
{
    return loopId;
}

void ChannelServer::Remove(const int &channelId)
{
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channelMap.count(channelId) == 0) {
        return;
    }
    // can not remove connected channel
    if (channelMap[channelId]->IsConnected()) {
        return;
    }
    channelMap.erase(channelId);
}

void ChannelServer::RemoveAll()
{
    std::lock_guard<std::mutex> lock(channelMutex);
    for (auto channel : channelMap) {
        channel.second->Close();
    }
    channelMap.clear();
}

int ChannelServer::ShutDown(const uv_stream_t *handle) const
{
    uv_shutdown_t *req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    if (req == nullptr) {
        return -1;
    }
    int ret = uv_shutdown(req, const_cast<uv_stream_t *>(handle), ChannelCallbacks::ShutDownCb);
    if (ret != 0) {
        free(req);
    }
    return ret;
}

void ChannelServer::CloseStream(const uv_handle_t *handle) const
{
    if (uv_is_closing(const_cast<uv_handle_t *>(handle)) == 0) {
        uv_close(const_cast<uv_handle_t *>(handle), ChannelCallbacks::CloseCb);
    }
}