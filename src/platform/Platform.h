// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_PLATFORM_H
#define DAP_SERVER_PLATFORM_H

#include <string>
#include <cstddef>  // size_t

#ifdef _WIN32
using FD = long int;
#else
using FD = int;
#endif


class Platform {
public:
    Platform() {}
    virtual ~Platform() {}

    // return: 0, success; other, error
    virtual int InterruptProcess(int debuggerPid, int debuggeePid) const = 0;
    virtual unsigned int GenRandomUInt() const = 0;
    virtual std::string GetShellNameByPID(int pid) const = 0;

    FD fdStdIn = -1;
    const int UINT_LEN = 4;
};

#endif // DAP_SERVER_PLATFORM_H
