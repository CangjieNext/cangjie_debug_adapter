// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelSocket.h"
#include "ChannelCallbacks.h"
#include "UvLoopMgr.h"

using namespace Dap;

ChannelSocket::ChannelSocket(const std::string &host, const int &port) : host(host), port(port) {}

ChannelSocket::ChannelSocket(const UvTcp &server) : uvTcpServer(&server) {}

ChannelSocket::~ChannelSocket() {}

bool ChannelSocket::Initialize()
{
    if (!this->InitializeAsync()) {
        this->OnError(UV_RET_FAIL, "Channel - socket: Initialize async handle failed.");
        return false;
    }

    uvTcpClient.arg = this;
    int ret = uv_tcp_init(UvLoopMgr::Instance().Loop(loopId), &uvTcpClient.handle);
    if (ret != 0) {
        this->OnError(ret, "Initialize tcp client error.");
        return false;
    }
    return true;
}

void ChannelSocket::UnInitialize()
{
    this->UnInitializeAsync();
    this->ReadStop(reinterpret_cast<uv_stream_t *>(&uvTcpClient.handle));
    this->ShutDown(reinterpret_cast<uv_stream_t *>(&uvTcpClient.handle));
    this->CloseStream(reinterpret_cast<uv_handle_t *>(&uvTcpClient.handle));
}

bool ChannelSocket::IsPassive() const
{
    return (this->uvTcpServer != nullptr);
}

bool ChannelSocket::Connect()
{
    if (IsPassive()) {
        return ConnectPassive();
    } else {
        return ConnectActive();
    }
}

bool ChannelSocket::Close()
{
    if (!this->isConnected) {
        return true;
    }
    this->isConnected = false;
    this->UnInitialize();
    this->OnClose();
    return true;
}

int ChannelSocket::Send(const std::string &data)
{
    if (!isConnected) {
        this->OnError(UV_RET_FAIL, "Channel - tcp client: Not connect.");
        return UV_RET_FAIL;
    }
    if (data.empty()) {
        // filter empty data
        return UV_RET_OK;
    }
    int ret = WriteData((uv_stream_t *)&uvTcpClient.handle, data);
    if (ret != 0) {
        this->OnError(ret, "Channel - tcp client: Write Data error.");
    }
    return ret;
}

bool ChannelSocket::ConnectActive()
{
    if (this->isConnected) {
        return true;
    }
    if (!this->Initialize()) {
        return false;
    }
    int ret = ConnectToServer();
    if (ret != 0) {
        this->OnError(ret, "Channel - tcp client: Connect error.");
        return false;
    }
    isConnected = true;
    return true;
}

bool ChannelSocket::ConnectPassive()
{
    if (this->isConnected) {
        return true;
    }
    if (!this->Initialize()) {
        return false;
    }
    int ret = uv_accept((uv_stream_t *)(&this->uvTcpServer->handle), (uv_stream_t *)&this->uvTcpClient.handle);
    if (ret != 0) {
        this->OnError(ret, "Channel - tcp client: Accept error.");
        return false;
    }
    ret = uv_read_start((uv_stream_t *)&this->uvTcpClient.handle, ChannelCallbacks::AllocBufferCb,
        ChannelCallbacks::ReadCbSocket);
    if (ret != 0) {
        this->OnError(ret, "Channel - tcp client: Read start error.");
        return false;
    }
    this->isConnected = true;
    return true;
}

int ChannelSocket::ConnectToServer()
{
    sockaddr_in addr;
    if (uv_ip4_addr(host.c_str(), port, &addr) != 0) {
        return -1;
    }
    UvReqConnect *connReq = (UvReqConnect *)malloc(sizeof(UvReqConnect));
    if (connReq == nullptr) {
        return -1;
    }
    connReq->arg = this;
    int ret =
        uv_tcp_connect(&connReq->req, &uvTcpClient.handle, (const sockaddr *)&addr, ChannelCallbacks::ConnectCbSocket);
    if (ret != 0) {
        free(connReq);
    }
    return ret;
}