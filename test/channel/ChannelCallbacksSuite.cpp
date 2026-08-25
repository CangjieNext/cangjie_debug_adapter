// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <uv.h>
#include <cstdlib>
#include "ChannelBaseSuite.h"
#include "ChannelCallbacks.h"
#include "MockChannel.h"
#include "MockChannelServer.h"

using namespace Dap;
using namespace ::testing;

class ChannelCallbacksSuite : public ChannelBaseSuite {
protected:
    void SetUp() override
    {
        mockChannel = std::make_shared<MockChannel>();
        mockChannel->RegChannelHandler(nullptr);
        mockChannelServer = std::make_shared<MockChannelServer>();
        mockChannelServer->RegChannelHandler(nullptr);
        mockChannelServer->RegChannelServerHandler(nullptr);
    }

    void TearDown() override {}

protected:
    std::shared_ptr<MockChannel> mockChannel = nullptr;
    std::shared_ptr<MockChannelServer> mockChannelServer = nullptr;
};

TEST_F(ChannelCallbacksSuite, alloc_buffer_cb_ok)
{
    size_t suggestedSize = 1;
    uv_buf_t buf;
    ChannelCallbacks::AllocBufferCb(nullptr, suggestedSize, &buf);
    EXPECT_EQ(buf.len, suggestedSize);
    EXPECT_NE(buf.base, nullptr);
}

TEST_F(ChannelCallbacksSuite, alloc_buffer_cb_failed)
{
    size_t suggestedSize = -1;
    uv_buf_t buf{};
    ChannelCallbacks::AllocBufferCb(nullptr, suggestedSize, &buf);
    EXPECT_EQ(buf.len, 0);
    EXPECT_EQ(buf.base, nullptr);
}

TEST_F(ChannelCallbacksSuite, read_cb_tty_normal)
{
    UvTty ttyClient;
    ttyClient.arg = mockChannel.get();
    ssize_t readSize = 2;
    uv_buf_t buf;
    ChannelCallbacks::AllocBufferCb(nullptr, readSize, &buf);
    ChannelCallbacks::ReadCbTty(reinterpret_cast<uv_stream_t*>(&ttyClient.handle), readSize, &buf);
}

TEST_F(ChannelCallbacksSuite, read_cb_tty_fail)
{
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    UvTty ttyClient;
    ttyClient.arg = mockChannel.get();
    ssize_t allocSize = 2;
    uv_buf_t buf1;
    ssize_t readSize = UV_EOF;
    ChannelCallbacks::AllocBufferCb(nullptr, allocSize, &buf1);
    ChannelCallbacks::ReadCbTty(reinterpret_cast<uv_stream_t*>(&ttyClient.handle), readSize, &buf1);
    uv_buf_t buf2;
    readSize = UV_EOF - 1;
    ChannelCallbacks::AllocBufferCb(nullptr, allocSize, &buf2);
    ChannelCallbacks::ReadCbTty(reinterpret_cast<uv_stream_t*>(&ttyClient.handle), readSize, &buf2);
}

TEST_F(ChannelCallbacksSuite, listen_cb_socket_status_error)
{
    UvTcp tcpServer;
    tcpServer.arg = mockChannelServer.get();
    int status = -1;
    ChannelCallbacks::ListenCbSocket(reinterpret_cast<uv_stream_t*>(&tcpServer.handle), status);
}

TEST_F(ChannelCallbacksSuite, listen_cb_socket_accept_failed)
{
    mockChannelServer->SetAcceptEnable(false);
    UvTcp tcpServer;
    tcpServer.arg = mockChannelServer.get();
    int status = 0;
    ChannelCallbacks::ListenCbSocket(reinterpret_cast<uv_stream_t*>(&tcpServer.handle), status);
}

TEST_F(ChannelCallbacksSuite, connect_cb_socket_status_error)
{
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    UvReqConnect *connReq = static_cast<UvReqConnect *>(std::malloc(sizeof(UvReqConnect)));
    ASSERT_NE(connReq, nullptr);
    connReq->arg = mockChannel.get();
    int status = -1;
    ChannelCallbacks::ConnectCbSocket(&connReq->req, status);
}

TEST_F(ChannelCallbacksSuite, read_cb_socket_fail)
{
    EXPECT_CALL(*mockChannel, Close()).WillRepeatedly(Return(true));
    UvTcp tcpClient;
    tcpClient.arg = mockChannel.get();
    ssize_t allocSize = 2;
    uv_buf_t buf;
    ssize_t readSize = UV_EOF - 1;
    ChannelCallbacks::AllocBufferCb(nullptr, allocSize, &buf);
    ChannelCallbacks::ReadCbSocket(reinterpret_cast<uv_stream_t*>(&tcpClient.handle), readSize, &buf);
}

TEST_F(ChannelCallbacksSuite, exit_cb_process)
{
    ChannelCallbacks::ExitCbProcess(nullptr, 0, 0);
}
