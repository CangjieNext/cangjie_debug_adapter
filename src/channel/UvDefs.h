// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_UV_DEFS_H
#define DAP_UV_DEFS_H

#include <uv.h>

namespace Dap {
constexpr int UV_READABLE = 1;
constexpr int UV_BACKLOG = 128;
constexpr int UV_RET_OK = 0;
constexpr int UV_RET_FAIL = -1;
constexpr int UV_IPC_NO = 0;
constexpr int UV_IPC_YES = 1;

struct UvTty {
    uv_tty_t handle;
    void *arg;
};

struct UvTcp {
    uv_tcp_t handle;
    void *arg;
};

enum UvPipeType : int {
    T_STDIN = 0,
    T_STDOUT = 1,
    T_STDERR = 2
};

struct UvPipe {
    uv_pipe_t handle;
    void *arg;
    UvPipeType type;
};

struct UvAsync {
    uv_async_t handle;
    void *arg;
};

struct UvReqConnect {
    uv_connect_t req;
    void *arg;
};

enum class ChannelAccessType {
    READ,
    WRITE,
    BOTH
};
}

#endif // DAP_UV_DEFS_H
