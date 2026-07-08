// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DebugSession.h"
#include "RequestManager.h"
#include "ResponseManager.h"
#include "EventManager.h"
#include "defs.h"
#include "DapLog.h"
#include "ChannelFactory.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

int DebugSession::currentSessionID = 1;

DebugSession::DebugSession(const std::shared_ptr<Channel> &uiChannel, const std::shared_ptr<Debugger> &dbg,
    const Option &option) : started(false), channelClient(uiChannel), debugger(dbg), globalOption(option)
{
    SetSessionID();
}

std::string DebugSession::GenerateProtocolHead(const std::string &message)
{
    string result = "Content-Length: ";
    result += to_string(message.size());
    result = result + "\r\n\r\n";
    return result;
}

void DebugSession::OnChannelData(const int &channelId, const std::string &data)
{
    LOG_DEBUG("channel ", channelId, " receive data: \n", data);
    msgBuffer >> data;
    onMsgCv.notify_one();
}

void DebugSession::OnMessageBuffer(DebugSession &session)
{
    LOG_INFO("OnMessageThread start.");
    const int oneMsgTime = 5;
    while (session.IsStarted()) {
        std::unique_ptr<ProtocolMessage> msg = session.msgBuffer.Pop();
        if (msg == nullptr) {
            std::unique_lock<std::mutex> lock(session.onMsgMutex);
            session.onMsgCv.wait_for(lock, std::chrono::milliseconds(oneMsgTime));
        } else if (msg->type == ProtocolMessage::Type::REQUEST) {
            Request* reqPtr = dynamic_cast<Request*>(msg.release());
            if (reqPtr == nullptr) {
                LOG_ERROR("Pop request, request is null.");
                continue;
            }
            while (session.debugger->GetRequestHandleMode() != RequestHandleMode::EXTERNAL) {
                std::this_thread::sleep_for(std::chrono::milliseconds(oneMsgTime));
            }
            std::unique_ptr<Request> req(reqPtr);
            LOG_INFO("Pop request, command = ", req->command, ", seq = ", req->seq);
            int ret = session.debugger->OnDapRequest(std::move(req));
            if (ret == -1) {
                LOG_ERROR("Request is not supported.");
            }
        } else if (msg->type == ProtocolMessage::Type::RESPONSE) {
            Response* resPtr = dynamic_cast<Response*>(msg.release());
            if (resPtr == nullptr) {
                LOG_ERROR("Pop response, response is null.");
                continue;
            }
            std::unique_ptr<Response> res(resPtr);
            LOG_INFO("Pop response, command = ", res->command, ", seq = ", res->seq,
                     ", request_seq = ", res->request_seq);
            session.debugger->OnDapResponse(std::move(res));
        } else {
            LOG_ERROR("Unidentified message type.");
        }
    }
    session.onMsgExitCv.notify_one();
    LOG_INFO("OnMessageThread exited.");
}

// error: only message (what == 0)
// error: real error   (what != 0error)
void DebugSession::OnChannelError(const int &channelId, int what, const std::string &error)
{
    if (what == 0) {
        LOG_INFO("channel ", channelId, ": ChannelError(", what, "): ", error);
    } else {
        LOG_ERROR("channel ", channelId, ": ChannelError(", what, "): ", error);
    }
}

void DebugSession::OnChannelClose(const int &channelId)
{
    LOG_INFO("DebugSession channel ", channelId, " closed");
    Stop();
    if (!sessionHandler.expired()) {
        LOG_INFO("DebugSession session ", sessionID, ": channel closed, session closed");
        sessionHandler.lock()->OnSessionClosed(sessionID);
    }
}

void DebugSession::OnDebuggerExit(const int &exitCode) const
{
    if (!sessionHandler.expired()) {
        LOG_INFO("DebugSession session ", sessionID, ": debugger exited, exit code is ", exitCode, ", session closed");
        sessionHandler.lock()->OnSessionClosed(sessionID);
    }
}

void DebugSession::OnDebuggerError(const int &errorCode, const std::string &msg) const
{
    LOG_INFO("Error occurred: ", errorCode, " , ", msg);
}

void DebugSession::OnDebuggerEvent(const Event &event)
{
    std::optional<json_t> json = EventManager::Instance().ToJson(event);
    if (!json.has_value()) {
        LOG_ERROR("JSON converted from event has no value!");
        return;
    }
    string content = json.value().dump(INDENTED_NUMBER);
    string head = GenerateProtocolHead(content);
    LOG_INFO("send event: \n", head + content);
    std::lock_guard<std::mutex> lock(debugSessionMutex);
    channelClient->AsyncSend(head + content);
}

void DebugSession::OnDebuggerResponse(const Response &response)
{
    std::optional<json_t> json = ResponseManager::Instance().ToJson(response);
    if (!json.has_value()) {
        LOG_ERROR("JSON converted from response has no value!");
        return;
    }
    string content = CommonUtils::JsonDump(json.value());
    if (content.empty()) {
        LOG_ERROR("json dump string is empty, maybe some fields of json are illegal.");
        return;
    }

    string head = GenerateProtocolHead(content);
    // Record REQ_RES_GET_BITMAP response in File "GetBitmapApiHandler.cpp"
    if (response.command != REQ_RES_GET_BITMAP) {
        LOG_INFO("send response: \n", head + content);
    }
    std::lock_guard<std::mutex> lock(debugSessionMutex);
    channelClient->AsyncSend(head + content);
}

void DebugSession::OnDebuggerRequest(const Dap::Protocol::Request &request)
{
    std::optional<json_t> json = RequestManager::Instance().ToJson(request);
    if (!json.has_value()) {
        LOG_ERROR("JSON converted from request has no value!");
        return;
    }
    string content = json.value().dump(INDENTED_NUMBER);
    string head = GenerateProtocolHead(content);
    LOG_INFO("send request: \n", head + content);
    std::lock_guard<std::mutex> lock(debugSessionMutex);
    channelClient->AsyncSend(head + content);
}

void DebugSession::Start()
{
    RequestManager::Instance().Register();
    ResponseManager::Instance().Register();
    EventManager::Instance().Register();
    debugger->Start();
    started = true;
    // onMsgThread start after started is true
    onMsgThread = std::make_unique<std::thread>(OnMessageBuffer, std::ref(*this));
    onMsgThread->detach();
    LOG_INFO("Session start, sessionId = ", sessionID);
}

void DebugSession::Stop()
{
    started = false;
    std::unique_lock<std::mutex> lock(onMsgExitMutex);
    onMsgExitCv.wait_for(lock, std::chrono::milliseconds(onMsgExitWaitTime));
    channelClient->Close();
    debugger->Stop();
    LOG_INFO("Session stop, sessionId = ", sessionID);
}
