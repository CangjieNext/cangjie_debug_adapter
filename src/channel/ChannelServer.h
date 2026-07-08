// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_SERVER_H
#define DAP_CHANNEL_SERVER_H

#include <map>
#include <uv.h>
#include "ChannelHandler.h"
#include "ChannelServerHandler.h"
#include "UvDefs.h"
#include "Channel.h"
#include "ChannelObjectId.h"

namespace Dap {
class ChannelServer {
public:
    virtual ~ChannelServer() = default;

    virtual bool IsStart() const;

    virtual bool Start() = 0;

    virtual bool Stop() = 0;

    virtual bool Accept(const std::shared_ptr<Channel> channel) = 0;

    virtual int Send(const std::uint32_t &channelId, const std::string &data) = 0;

    virtual std::shared_ptr<Channel> GetChannel(const int &channelId);

    virtual void RegChannelHandler(std::shared_ptr<ChannelHandler> handler);

    virtual void RegChannelServerHandler(std::shared_ptr<ChannelServerHandler> handler);

    virtual int Id() const;

    virtual void OnAccept(const int &channelId) const;

    virtual void OnServerError(const int &what, const std::string &error) const;

    virtual void SetLoopId(const int &idNum);

    virtual int GetLoopId() const;

protected:
    virtual void Remove(const int &channelId);

    virtual void RemoveAll();

    virtual int ShutDown(const uv_stream_t *handle) const;

    virtual void CloseStream(const uv_handle_t *handle) const;

protected:
    std::shared_ptr<ChannelServerHandler> channelServerHandler = nullptr;
    std::shared_ptr<ChannelHandler> channelHandler = nullptr;
    bool isStart = false;
    std::mutex channelMutex;
    std::map<int, std::shared_ptr<Channel>> channelMap;
    int loopId = -1;
    int id = ChannelObjectId::ChannelServerId().Create();
};
}

#endif // DAP_CHANNEL_SERVER_H
