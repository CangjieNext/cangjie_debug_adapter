// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_THREAD_H
#define DAP_CHANNEL_THREAD_H

#include <thread>
#include <uv.h>
#include "Channel.h"
#include "ChannelObjectId.h"

namespace Dap {
class ChannelThread {
public:
    explicit ChannelThread(const std::shared_ptr<Channel> channelPtr);
    ~ChannelThread();

    bool Start();
    bool Stop();
    bool IsStart();

    int ThreadId() const;
    int ChannelId() const;
    std::shared_ptr<Channel> GetChannel() const;

private:
    static void ThreadFunc(ChannelThread &ct);

private:
    bool isStart = false;
    std::unique_ptr<std::thread> threadPtr;
    std::shared_ptr<Channel> channelPtr;
    std::mutex resultMutex;
    std::condition_variable resultCv;
    const int resultTimeout = 3000;
    int id = ChannelObjectId::ThreadId().Create();
};
}

#endif // DAP_CHANNEL_THREAD_H
