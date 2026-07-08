// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelCallbacks.h"
#include <cstdio>
#include <uv.h>
#include "Channel.h"
#include "ChannelServer.h"
#include "ChannelFactory.h"
#include "DapLog.h"

using namespace Dap;

void ChannelCallbacks::AllocBufferCb(uv_handle_t * /* handle */, size_t suggestedSize, uv_buf_t *buf)
{
    constexpr size_t maxSize = 64 * 1024 * 1024; // 64M
    if (suggestedSize > maxSize) {
        LOG_ERROR("error, allocated data is greater than max size");
        return;
    }
    if (suggestedSize == 0 || buf == nullptr) {
        LOG_ERROR("error, no data to read, buffer size is 0!");
        return;
    }
    buf->base = (char *)malloc(suggestedSize);
    if (buf->base == nullptr) {
        buf->len = 0;
        return;
    }
    buf->len = suggestedSize;
}

void ChannelCallbacks::ReadCbTty(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf)
{
    if (buf == nullptr) {
        return;
    }
    UvTty *tty = reinterpret_cast<UvTty *>(client);
    if (tty == nullptr || tty->arg == nullptr) {
        return;
    }
    Channel *channel = reinterpret_cast<Channel *>(tty->arg);
    if (channel == nullptr) {
        return;
    }
    if (readSize > 0) {
        if (buf->base != nullptr && (size_t)readSize <= buf->len) {
            channel->OnData(std::string(buf->base, readSize));
        } else {
            channel->OnError(-1, "Channel - std: Invalid buffer parameters");
        }
    } else if (readSize < 0) {
        if (readSize != UV_EOF) {
            channel->OnError(-1, "Channel - std: Read size < 0.");
        }
        channel->Close();
    }
    if (buf->base != nullptr) {
        free(buf->base);
    }
}

void ChannelCallbacks::ListenCbSocket(uv_stream_t *server, int status)
{
    UvTcp *tcp = reinterpret_cast<UvTcp *>(server);
    if (tcp == nullptr || tcp->arg == nullptr) {
        return;
    }
    ChannelServer *channelServer = reinterpret_cast<ChannelServer *>(tcp->arg);
    if (channelServer == nullptr) {
        return;
    }
    if (status < 0) {
        channelServer->OnServerError(status, "Channel - tcp server: Listen status error.");
        return;
    }
    std::shared_ptr<Channel> client = ChannelFactory::CreateChannelSocket(*tcp);
    if (client == nullptr) {
        return;
    }
    client->SetLoopId(channelServer->GetLoopId());
    if (!channelServer->Accept(client)) {
        channelServer->OnServerError(UV_RET_FAIL, "Channel - tcp server: Accept client error.");
        return;
    }
}

void ChannelCallbacks::ConnectCbSocket(uv_connect_t *req, int status)
{
    UvReqConnect *connReq = reinterpret_cast<UvReqConnect *>(req);
    if (connReq == nullptr || connReq->arg == nullptr) {
        return;
    }
    Channel *channel = reinterpret_cast<Channel *>(connReq->arg);
    if (channel == nullptr) {
        return;
    }
    if (status < 0) {
        channel->OnError(status, "Channel - tcp client: Connect status error.");
        channel->Close();
    } else {
        int ret = uv_read_start(req->handle, ChannelCallbacks::AllocBufferCb, ChannelCallbacks::ReadCbSocket);
        if (ret != 0) {
            channel->OnError(ret, "Channel - tcp client: Connect read error.");
            channel->Close();
        }
    }
    free(connReq);
}

void ChannelCallbacks::ReadCbSocket(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf)
{
    if (buf == nullptr) {
        return;
    }
    UvTcp *tcp = reinterpret_cast<UvTcp *>(client);
    if (tcp == nullptr || tcp->arg == nullptr) {
        return;
    }
    Channel *channel = reinterpret_cast<Channel *>(tcp->arg);
    if (channel == nullptr) {
        return;
    }
    if (readSize > 0) {
        if (buf->base != nullptr && (size_t)readSize <= buf->len) {
            channel->OnData(std::string(buf->base, readSize));
        } else {
            channel->OnError(UV_RET_FAIL, "Channel - tcp client: Invalid buffer parameters");
        }
    } else if (readSize < 0) {
        if (readSize != UV_EOF) {
            channel->OnError(UV_RET_FAIL, "Channel - tcp client: Read error.");
        }
        channel->Close();
    }
    if (buf->base != nullptr) {
        free(buf->base);
    }
}

void ChannelCallbacks::WriteCb(uv_write_t *req, int /* status */)
{
    // note: release req
    free(req);
}

void ChannelCallbacks::ShutDownCb(uv_shutdown_t *req, int /* status */)
{
    free(req);
}

void ChannelCallbacks::CloseCb(uv_handle_t * /* handle */) {}

void ChannelCallbacks::ExitCbProcess(uv_process_t *, int64_t /* exitStatus */, int /* termSignal */) {}