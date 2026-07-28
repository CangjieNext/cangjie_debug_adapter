// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DUMP_UTILS_H
#define DAP_SERVER_DUMP_UTILS_H

#include <string>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#endif
#include "TimeUtils.h"
#include "DapLog.h"
#include "CommonUtils.h"

#ifdef _WIN32
typedef BOOL(WINAPI *FuncCreateDump)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION,
    PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);
int EnableCreateDump(LPEXCEPTION_POINTERS exceptionPtrs);
LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo);
#endif

namespace Dap {
class DumpUtils {
public:
    static DumpUtils &Instance();
    DumpUtils &SetDumpFileDir(const std::string &dir);
    const std::string GetDumpFilePath();
    void StartRecordDumpInfo();

private:
    DumpUtils() = default;
    ~DumpUtils() = default;

private:
    const static std::string PREFIX;
    std::string dumpFilePath;
};
}

#endif // DAP_SERVER_DUMP_UTILS_H
