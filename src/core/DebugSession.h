// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUG_SESSION_H
#define DAP_SERVER_DEBUG_SESSION_H

#include "Channel.h"
#include "ChannelHandler.h"
#include "ChannelThread.h"
#include "Debugger.h"
#include "Protocol.h"
#include "Option.h"
#include "SessionHandler.h"
#include "MessageBuffer.h"

namespace Dap {
class DebugSession : public DebuggerMessageHandler, public Dap::ChannelHandler,
    public std::enable_shared_from_this<DebugSession> {
public:
    DebugSession(const std::shared_ptr<Dap::Channel> &uiChannel, const std::shared_ptr<Debugger> &dbg,
                 const Dap::Option &option);
    ~DebugSession() override = default;
    void RegSessionHandler(const std::weak_ptr<SessionHandler> &handler)
    {
        this->sessionHandler = handler;
    };
    virtual void Initialize()
    {
        debugger->RegisterDebuggerMessageHandler(shared_from_this());
        channelClient->RegChannelHandler(shared_from_this());
    }
    virtual void Start();
    void Stop();
    bool IsStarted() const
    {
        return started;
    }
    int GetSessionID() const
    {
        return sessionID;
    }
    void SetSessionID()
    {
        std::lock_guard<std::mutex> lock(debugSessionMutex);
        currentSessionID %= std::numeric_limits<int>::max();
        sessionID = currentSessionID++;
    }

    void OnChannelData(const int &channelId, const std::string &data) override;
    void OnChannelError(const int &channelId, int what, const std::string &error) override;
    void OnChannelClose(const int &channelId) override;

    void OnDebuggerResponse(const Dap::Protocol::Response &response) override;
    void OnDebuggerEvent(const Dap::Protocol::Event &event) override;
    void OnDebuggerRequest(const Dap::Protocol::Request &request) override;
    void OnDebuggerError(const int &errorCode, const std::string &msg) const override;
    void OnDebuggerExit(const int &exitCode) const override;

protected:
    static void OnMessageBuffer(DebugSession &session);
    static std::string GenerateProtocolHead(const std::string &message);

    bool started;
    std::shared_ptr<Dap::Channel> channelClient;
    std::shared_ptr<Debugger> debugger;
    Dap::Option globalOption;
    std::weak_ptr<SessionHandler> sessionHandler;
    std::shared_ptr<Dap::ChannelThread> channelThread;
    std::mutex debugSessionMutex;
    static int currentSessionID;
    int sessionID;
    static const int INDENTED_NUMBER = 4;

    Dap::MessageBuffer msgBuffer;
    std::unique_ptr<std::thread> onMsgThread;
    std::mutex onMsgMutex;
    std::condition_variable onMsgCv;
    std::mutex onMsgExitMutex;
    std::condition_variable onMsgExitCv;
    const int onMsgExitWaitTime = 3000;
};
}

#endif // DAP_SERVER_DEBUG_SESSION_H