// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_MOCK_CHANNEL_H
#define DAP_MOCK_CHANNEL_H

#include <gmock/gmock.h>
#include "Channel.h"
#include "ChannelSocket.h"
#include "UvDefs.h"

class MockChannel : public Dap::Channel {
public:
    MOCK_METHOD0(Connect, bool());
    MOCK_METHOD0(Close, bool());
    MOCK_METHOD1(Send, int(const std::string &data));
    MOCK_METHOD1(AsyncSend, void(const std::string &data));
    void SetConnectStatus(bool connected)
    {
        isConnected = connected;
    }
};

class MockChannelSocket : public Dap::ChannelSocket {
public:
    MockChannelSocket(const std::string &host, const int &port) : ChannelSocket(host, port) {}
    ~MockChannelSocket() {}
    MOCK_METHOD0(Connect, bool());
    MOCK_METHOD0(Close, bool());

    void SetConnectStatus(bool connected)
    {
        isConnected = connected;
    }
};

class HelpChannelSocket : public Dap::ChannelSocket {
public:
    HelpChannelSocket(const std::string &host, const int &port) : ChannelSocket(host, port) {}
    explicit HelpChannelSocket(const Dap::UvTcp &server) : ChannelSocket(server) {}
    ~HelpChannelSocket() {}
    bool InitializeAsync() override
    {
        return initializeAsyncFlag;
    }
    void SetInitializeFlag(bool flag)
    {
        initializeAsyncFlag = flag;
    }
    void SetConnectStatus(bool connected)
    {
        isConnected = connected;
    }

protected:
    bool initializeAsyncFlag = true;
};

#endif // DAP_MOCK_CHANNEL_H
