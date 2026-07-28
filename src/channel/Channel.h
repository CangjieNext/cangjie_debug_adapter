// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_H
#define DAP_CHANNEL_H

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <uv.h>
#include "UvDefs.h"
#include "ChannelHandler.h"
#include "ChannelObjectId.h"

namespace Dap {
class Channel {
public:
    virtual ~Channel() = default;

    virtual bool IsConnected() const;

    virtual bool Connect() = 0;

    virtual bool Close() = 0;

    virtual int Send(const std::string &data) = 0;

    virtual void AsyncSend(const std::string &data);

    virtual void RegChannelHandler(std::shared_ptr<ChannelHandler> handler);

    virtual int Id() const;

    virtual void OnData(const std::string &data) const;

    virtual void OnError(const int &what, const std::string &error) const;

    virtual void OnMessage(const std::string &msg) const;

    virtual void OnClose() const;

    virtual void SetLoopId(const int &idNum);

    virtual int GetLoopId() const;

protected:
    virtual int WriteData(const uv_stream_t *stream, const std::string &data) const;

    virtual int ReadStop(const uv_stream_t *handle) const;

    virtual int ShutDown(const uv_stream_t *handle) const;

    virtual void CloseStream(const uv_handle_t *handle) const;

    virtual bool InitializeAsync();

    virtual void UnInitializeAsync();

    static void AsyncSendCb(uv_async_t *handle);

protected:
    std::shared_ptr<ChannelHandler> dataHandler = nullptr;
    bool isConnected = false;
    int id = ChannelObjectId::ChannelId().Create();
    int loopId = -1;

    UvAsync sendAsync;
    std::mutex sendMutex;
    std::condition_variable sendCv;
};
}

#endif // DAP_CHANNEL_H
