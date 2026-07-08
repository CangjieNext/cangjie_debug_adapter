// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_OBJECT_ID_H
#define DAP_CHANNEL_OBJECT_ID_H

#include <mutex>

namespace Dap {
class ChannelObjectId {
public:
    static ChannelObjectId &ChannelId();
    static ChannelObjectId &ChannelServerId();
    static ChannelObjectId &ThreadId();
    int Create();

private:
    ChannelObjectId() = default;
    ~ChannelObjectId() = default;

private:
    const static int MAX_ID = 0x7dffffff;
    int id = 0;
    std::mutex mtx;
};
}

#endif // DAP_CHANNEL_OBJECT_ID_H
