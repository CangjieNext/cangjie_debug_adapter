// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_WINDOWS_PLATFORM_H
#define DAP_SERVER_WINDOWS_PLATFORM_H

#ifdef _WIN32
#include "Platform.h"

class WindowsPlatform : public Platform {
public:
    WindowsPlatform()
    {
        fdStdIn = 0;
    }

    ~WindowsPlatform() {}

    int InterruptProcess(int debuggerPid, int debuggeePid) const override;
    unsigned int GenRandomUInt() const override;
    std::string GetShellNameByPID(int pid) const override;
};
#endif // _WIN32

#endif // DAP_SERVER_WINDOWS_PLATFORM_H