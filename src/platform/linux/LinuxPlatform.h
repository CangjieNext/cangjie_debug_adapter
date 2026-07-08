// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_LINUX_PLATFORM_H
#define DAP_SERVER_LINUX_PLATFORM_H

#ifndef _WIN32

#include <unistd.h>
#include <random>
#include "Platform.h"
#include "uv.h"

class LinuxPlatform : public Platform {
public:
    LinuxPlatform()
    { fdStdIn = STDIN_FILENO; }

    ~LinuxPlatform()
    {}

    int InterruptProcess(int debuggerPid, int debuggeePid) const override
    {
        (void)debuggeePid;
        return uv_kill(debuggerPid, SIGINT);
    }

    unsigned int GenRandomUInt() const override
    {
        std::random_device rd("/dev/urandom");
        return rd();
    }

    std::string GetShellNameByPID(int pid) const override;
};

#endif // _WIN32

#endif // DAP_SERVER_LINUX_PLATFORM_H
