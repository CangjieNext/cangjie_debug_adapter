// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_CONTAINER_THREAD_H
#define DAP_CHANNEL_CONTAINER_THREAD_H

#include <thread>
#include "ChannelContainer.h"

namespace Dap {
class ChannelContainerThread {
public:
    static ChannelContainerThread &Instance();

    bool Start();
    bool Stop();
    bool IsStart();

private:
    ChannelContainerThread();
    ~ChannelContainerThread();
    static void ThreadFunc();

private:
    bool isStart;
    std::unique_ptr<std::thread> threadPtr;
    int id = ChannelObjectId::ThreadId().Create(); // as a tag id
};
}

#endif // DAP_CHANNEL_CONTAINER_THREAD_H
