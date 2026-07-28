// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_CALLBACKS_H
#define DAP_CHANNEL_CALLBACKS_H

#include <uv.h>

namespace Dap {
class ChannelCallbacks {
public:
    static void AllocBufferCb(uv_handle_t *handle, size_t suggestedSize, uv_buf_t *buf);

    static void ReadCbTty(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf);

    static void ListenCbSocket(uv_stream_t *server, int status);

    static void ConnectCbSocket(uv_connect_t *req, int status);

    static void ReadCbSocket(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf);

    static void WriteCb(uv_write_t *req, int status);

    static void ShutDownCb(uv_shutdown_t *req, int status);

    static void CloseCb(uv_handle_t *handle);

    static void ExitCbProcess(uv_process_t *, int64_t exitStatus, int termSignal);
};
}

#endif // DAP_CHANNEL_CALLBACKS_H
