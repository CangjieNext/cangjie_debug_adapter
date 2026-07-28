// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_HANDLER_H
#define DAP_CHANNEL_HANDLER_H

#include <string>

namespace Dap {
class ChannelHandler {
public:
    virtual void OnChannelData(const int &channelId, const std::string &data) = 0;

    virtual void OnChannelError(const int &channelId, int what, const std::string &error) = 0;

    virtual void OnChannelClose(const int &channelId) = 0;

    virtual ~ChannelHandler() = default;
};
}
#endif // DAP_CHANNEL_HANDLER_H
