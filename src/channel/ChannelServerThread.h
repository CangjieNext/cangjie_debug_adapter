// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_SERVER_THREAD_H
#define DAP_CHANNEL_SERVER_THREAD_H

#include <thread>
#include <uv.h>
#include "ChannelServer.h"
#include "ChannelObjectId.h"

namespace Dap {
class ChannelServerThread {
public:
    explicit ChannelServerThread(const std::shared_ptr<ChannelServer> channelServerPtr);
    ~ChannelServerThread();

    bool Start();
    bool Stop();
    bool IsStart() const;

    int ThreadId() const;
    int ChannelServerId() const;
    std::shared_ptr<ChannelServer> GetChannelServer() const;
    std::shared_ptr<Channel> GetChannel(const int &channelId) const;

private:
    static void ThreadFunc(ChannelServerThread &ct);

private:
    bool isStart;
    std::unique_ptr<std::thread> threadPtr;
    std::shared_ptr<ChannelServer> channelServerPtr;
    std::mutex resultMutex;
    std::condition_variable resultCv;
    const int resultTimeout = 3000;
    int id = ChannelObjectId::ThreadId().Create();
};
}

#endif // DAP_CHANNEL_SERVER_THREAD_H
