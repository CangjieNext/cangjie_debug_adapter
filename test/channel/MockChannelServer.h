// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_MOCK_CHANNEL_SERVER_H
#define DAP_MOCK_CHANNEL_SERVER_H

#include <gmock/gmock.h>
#include "ChannelServer.h"
#include "ChannelSocketServer.h"

class MockChannelServer : public Dap::ChannelServer {
public:
    MOCK_METHOD0(Start, bool());
    MOCK_METHOD0(Stop, bool());
    MOCK_METHOD2(Send, int(const std::uint32_t &channelId, const std::string &data));
    bool Accept(const std::shared_ptr<Dap::Channel> channel) override
    {
        if (!acceptEnable) {
            return false;
        }
        if (channelMap.count(channel->Id()) > 0) {
            return true;
        }
        channelMap[channel->Id()] = channel;
        return true;
    }
    void SetAcceptEnable(bool enable)
    {
        acceptEnable = enable;
    }

protected:
    bool acceptEnable = true;
};

class HelpChannelSocketServer : public Dap::ChannelSocketServer {
public:
    explicit HelpChannelSocketServer(const std::string &host, const int &port) : Dap::ChannelSocketServer(host, port) {}
    ~HelpChannelSocketServer() {};
    void SetStartStatus(bool start)
    {
        isStart = start;
    }
    bool Initialize() override
    {
        return initializeFlag;
    }
    void SetInitializeFlag(bool flag)
    {
        initializeFlag = flag;
    }

protected:
    bool initializeFlag = true;
};

#endif // DAP_MOCK_CHANNEL_SERVER_H
