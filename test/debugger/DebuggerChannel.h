// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUGGERCHANNEL_H
#define DAP_SERVER_DEBUGGERCHANNEL_H
#include <channel/Channel.h>

class DebuggerChannel : public Dap::Channel {
public:
    bool Connect() override
    {
        return true;
    }
    bool Close() override
    {
        return true;
    }
    int Send(const std::string &data) override
    {
        (void)data;
        return 1;
    }
    void AsyncSend(const std::string &data) override
    {
        (void)data;
        channelJudge = true;
    }
    bool GetChannelJudge()
    {
        return channelJudge;
    }
    void SetChannelJudge()
    {
        channelJudge = !channelJudge;
    }
private:
   volatile bool channelJudge = false;
};
#endif // DAP_SERVER_DEBUGGERCHANNEL_H
