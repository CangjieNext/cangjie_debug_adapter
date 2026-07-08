// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelContainer.h"
#include "UvLoopMgr.h"

using namespace Dap;

ChannelContainer::ChannelContainer()
{
    UvLoopMgr::Instance().LoopInit();
}

ChannelContainer::~ChannelContainer()
{
    this->Stop();
}

ChannelContainer &ChannelContainer::Instance()
{
    static ChannelContainer instance;
    return instance;
}

void ChannelContainer::AddChannel(const std::shared_ptr<Channel> &channel)
{
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channelMap.count(channel->Id()) == 0) {
        channelMap[channel->Id()] = channel;
    }
}

void ChannelContainer::AddChannelServer(const std::shared_ptr<ChannelServer> &server)
{
    std::lock_guard<std::mutex> lock(channelServerMutex);
    if (channelServerMap.count(server->Id()) == 0) {
        channelServerMap[server->Id()] = server;
    }
}

void ChannelContainer::RemoveChannel(const int &channelId)
{
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channelMap.count(channelId) > 0) {
        channelMap[channelId]->Close();
        channelMap.erase(channelId);
    }
}

void ChannelContainer::RemoveChannelServer(const int &channelServerId)
{
    std::lock_guard<std::mutex> lock(channelServerMutex);
    if (channelServerMap.count(channelServerId) > 0) {
        channelServerMap[channelServerId]->Stop();
        channelServerMap.erase(channelServerId);
    }
}

const std::shared_ptr<Channel> ChannelContainer::GetChannel(const int &channelId)
{
    {
        std::lock_guard<std::mutex> lock(channelMutex);
        if (channelMap.count(channelId)) {
            return channelMap.at(channelId);
        }
    }
    {
        std::lock_guard<std::mutex> lock(channelServerMutex);
        for (auto iter : channelServerMap) {
            if (iter.second->GetChannel(channelId) != nullptr) {
                return iter.second->GetChannel(channelId);
            }
        }
    }
    return nullptr;
}

const std::shared_ptr<ChannelServer> ChannelContainer::GetChannelServer(const int &channelServerId)
{
    std::lock_guard<std::mutex> lock(channelServerMutex);
    if (channelServerMap.count(channelServerId) == 0) {
        return nullptr;
    }
    return channelServerMap.at(channelServerId);
}

const std::shared_ptr<Channel> ChannelContainer::GetChannelFromServer(const int &channelServerId, const int &channelId)
{
    std::lock_guard<std::mutex> lock(channelServerMutex);
    if (channelServerMap.count(channelServerId) == 0) {
        return nullptr;
    }
    return channelServerMap.at(channelServerId)->GetChannel(channelId);
}

void ChannelContainer::Start()
{
    int startCount = 0;
    {
        std::lock_guard<std::mutex> lock(channelServerMutex);
        for (auto iter : channelServerMap) {
            if (iter.second->Start()) {
                startCount++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(channelMutex);
        for (auto iter : channelMap) {
            if (iter.second->Connect()) {
                startCount++;
            }
        }
    }
    if (startCount > 0) {
        UvLoopMgr::Instance().LoopStart();
    }
}

void ChannelContainer::Stop()
{
    {
        std::lock_guard<std::mutex> lock(channelMutex);
        for (auto iter : channelMap) {
            iter.second->Close();
        }
        this->channelMap.clear();
    }
    {
        std::lock_guard<std::mutex> lock(channelServerMutex);
        for (auto iter : channelServerMap) {
            iter.second->Stop();
        }
        this->channelServerMap.clear();
    }
    UvLoopMgr::Instance().LoopStop();
}