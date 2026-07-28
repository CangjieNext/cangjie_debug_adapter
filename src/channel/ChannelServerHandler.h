// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_SERVER_HANDLER_H
#define DAP_CHANNEL_SERVER_HANDLER_H

#include <string>

namespace Dap {
class ChannelServerHandler {
public:
    virtual void OnChannelAccepted(const int &channelServerId, const int &channelId) = 0;

    virtual void OnChannelServerError(const int &channelServerId, const int &what, const std::string &error) = 0;

    virtual ~ChannelServerHandler() = default;
};
}

#endif // DAP_CHANNEL_SERVER_HANDLER_H
