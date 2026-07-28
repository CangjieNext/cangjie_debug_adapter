// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Channel.h"
#include "ChannelCallbacks.h"
#include "UvLoopMgr.h"

using namespace Dap;

bool Channel::IsConnected() const
{
    return isConnected;
}

void Channel::RegChannelHandler(std::shared_ptr<ChannelHandler> handler)
{
    dataHandler = handler;
}

int Channel::Id() const
{
    return this->id;
}

void Channel::OnData(const std::string &data) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelData(this->id, data);
    }
}

void Channel::OnError(const int &what, const std::string &error) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelError(this->id, what, error);
    }
}

void Channel::OnMessage(const std::string &msg) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelError(this->id, 0, msg);
    }
}

void Channel::OnClose() const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelClose(this->id);
    }
}

void Channel::SetLoopId(const int &idNum)
{
    loopId = idNum;
}

int Channel::GetLoopId() const
{
    return loopId;
}

int Channel::WriteData(const uv_stream_t *stream, const std::string &data) const
{
    uv_write_t *writeReq = (uv_write_t *)malloc(sizeof(uv_write_t));
    if (writeReq == nullptr) {
        return -1;
    }
    uv_buf_t buf;
    buf.base = (char *)data.c_str();
    buf.len = data.length();
    int bufCount = 1;
    int ret = uv_write(writeReq, const_cast<uv_stream_t *>(stream), &buf, bufCount, ChannelCallbacks::WriteCb);
    if (ret != 0) {
        free(writeReq);
    }
    return ret;
}

int Channel::ReadStop(const uv_stream_t *handle) const
{
    return uv_read_stop(const_cast<uv_stream_t *>(handle));
}

int Channel::ShutDown(const uv_stream_t *handle) const
{
    uv_shutdown_t *req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    if (req == nullptr) {
        return -1;
    }
    int ret = uv_shutdown(req, const_cast<uv_stream_t *>(handle), ChannelCallbacks::ShutDownCb);
    if (ret != 0) {
        free(req);
    }
    return ret;
}

void Channel::CloseStream(const uv_handle_t *handle) const
{
    if (uv_is_closing(const_cast<uv_handle_t *>(handle)) == 0) {
        uv_close(const_cast<uv_handle_t *>(handle), ChannelCallbacks::CloseCb);
    }
}

bool Channel::InitializeAsync()
{
    sendAsync.arg = this;
    int ret = uv_async_init(UvLoopMgr::Instance().Loop(loopId), &sendAsync.handle, AsyncSendCb);
    return (ret == 0);
}

void Channel::UnInitializeAsync()
{
    if (uv_is_closing((uv_handle_t *)(&sendAsync.handle)) == 0) {
        uv_close((uv_handle_t *)(&sendAsync.handle), ChannelCallbacks::CloseCb);
    }
}

void Channel::AsyncSendCb(uv_async_t *handle)
{
    UvAsync *asyncHandle = (UvAsync *)handle;
    Channel *channel = (Channel *)asyncHandle->arg;
    std::string data((char *)handle->data);
    channel->Send(data);
    std::unique_lock<std::mutex> lock(channel->sendMutex);
    channel->sendCv.notify_one();
}

void Channel::AsyncSend(const std::string &data)
{
    if (!this->isConnected || data.empty()) {
        return;
    }
    std::unique_lock<std::mutex> lock(sendMutex);
    sendAsync.handle.data = (char *)data.c_str();
    uv_async_send(&sendAsync.handle);
    sendCv.wait(lock);
}