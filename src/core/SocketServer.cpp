// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SocketServer.h"
#include "DebugSession.h"
#include "DebuggerHelper.h"
#include "DapLog.h"
#include "Option.h"
#include "ChannelFactory.h"
using namespace std;
using namespace Dap;

bool SocketServer::sessionCreated = false;

SocketServer::SocketServer(const Option &option) : globalOption(option), started(false)
{
    std::string host = globalOption.hostIP;
    channelServer = ChannelFactory::CreateChannelSocketServer(host, globalOption.port);
}

bool SocketServer::Start()
{
    if (socketChannelServerThread->IsStart()) {
        LOG_INFO("SocketServer already running!");
        return true;
    }
    if (!socketChannelServerThread->Start())  {
        LOG_INFO("SocketServer start failed!");
        return false;
    }
    started = true;
    return true;
}

bool SocketServer::Stop()
{
    for (auto iter = sessions.begin(); iter != sessions.end(); iter++) {
        (*iter)->Stop();
    }
    if (!socketChannelServerThread->IsStart()) {
        LOG_INFO("SocketServer is not running!");
        return true;
    }
    if (!socketChannelServerThread->Stop())  {
        LOG_INFO("SocketServer stop failed!");
        return false;
    }
    started = false;
    return true;
}

void SocketServer::OnChannelAccepted(const int &channelServerId, const int &channelId)
{
    LOG_INFO("SocketServer Accepted, channelServerId: ", channelServerId, ", channelId: ", channelId);
    std::shared_ptr<Channel> uiTunnel = channelServer->GetChannel(channelId);
    if (globalOption.debugSessionMode == DebugSessionMode::SINGLE && !sessions.empty()) {
        LOG_INFO("Single SessionMode Socket is already connected, close current channel, channelServerId: ",
                 channelServerId, ", channelId: ", channelId);
        if (uiTunnel != nullptr) {
            uiTunnel->Close();
        }
        return;
    }
    std::shared_ptr<Debugger> debugger = DebuggerHelper::CreateDebugger(globalOption);
    std::shared_ptr <DebugSession> session =
        std::make_shared<DebugSession>(uiTunnel, debugger, globalOption);
    session->Initialize();
    session->Start();
    sessionCreated = true;
    session->RegSessionHandler(GetSocketServerWeakPtr());
    sessions.push_back(std::move(session));
}

void SocketServer::OnChannelServerError(const int &channelServerId, const int &what, const std::string &error)
{
    LOG_INFO("channelServerId ", channelServerId, ": ChannelError(", what, "): ", error);
}

void SocketServer::OnSessionClosed(int sessionID)
{
    if (sessions.empty()) {
        LOG_ERROR("debug session is already cleared!");
        return;
    }
    unsigned pos = 0;
    bool sessionExited = false;
    for (; pos < sessions.size(); pos++) {
        if (sessions[pos]->GetSessionID() == sessionID) {
            sessionExited = true;
            break;
        }
    }
    if (sessionExited) {
        sessions.erase(sessions.begin() + pos);
    }
}
