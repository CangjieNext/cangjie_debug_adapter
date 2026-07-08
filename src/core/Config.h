// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_CONFIG_H
#define DAP_SERVER_CONFIG_H

namespace Dap {
enum class ChannelType {
    CHANNEL_STD,
    CHANNEL_SOCKET,
    CHANNEL_PIPE
};

enum class PlatformType {
    PLATFORM_WINDOWS,
    PLATFORM_LINUX,
    PLATFORM_DARWIN
};

enum class StartMode {
    ATTACH_MODE,
    LAUNCH_MODE
};

enum class RemoteClass {
    NO_REMOTE,
    DEBUGGER_SERVER_REMOTE
};

enum class DebugSessionMode {
    SINGLE,
    MULTIPLE
};
}
#endif // DAP_SERVER_CONFIG_H