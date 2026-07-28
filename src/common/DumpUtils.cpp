// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DumpUtils.h"

using namespace Dap;

#ifdef _WIN32
int EnableCreateDump(LPEXCEPTION_POINTERS exceptionPtrs)
{
    HMODULE module = ::LoadLibrary("C:\\Windows\\System32\\dbghelp.dll");
    if (module == nullptr) {
        LOG_WARN("Load dbghelp module failed.");
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    FuncCreateDump func = reinterpret_cast<FuncCreateDump>(
        reinterpret_cast<void*>(::GetProcAddress(module, "MiniDumpWriteDump")));
    if (func == nullptr) {
        ::FreeLibrary(module);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    std::string dumpFilePath = DumpUtils::Instance().GetDumpFilePath();
    HANDLE hFile = ::CreateFile(dumpFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
        0, CREATE_ALWAYS, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        ::FreeLibrary(module);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;
    dumpExceptionInfo.ThreadId = ::GetCurrentThreadId();
    dumpExceptionInfo.ExceptionPointers = exceptionPtrs;
    dumpExceptionInfo.ClientPointers = FALSE;
    if (exceptionPtrs == nullptr) {
        func(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, _MINIDUMP_TYPE::MiniDumpWithDataSegs, 0, 0, 0);
    } else {
        func(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, _MINIDUMP_TYPE::MiniDumpWithDataSegs,
            &dumpExceptionInfo, 0, 0);
    }
    ::CloseHandle(hFile);
    ::FreeLibrary(module);
    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
    if (IsDebuggerPresent()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return EnableCreateDump(lpExceptionInfo);
}
#endif

const std::string DumpUtils::PREFIX = "dap_server_";

DumpUtils &DumpUtils::Instance()
{
    static DumpUtils instance;
    return instance;
}

DumpUtils &DumpUtils::SetDumpFileDir(const std::string &dir)
{
    this->dumpFilePath = PREFIX + TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_3) + ".dmp";
    if (CommonUtils::IsFolderExist(dir)) {
#ifdef _WIN32
        this->dumpFilePath = dir + "\\" + this->dumpFilePath;
#else
        this->dumpFilePath = dir + "/" + this->dumpFilePath;
#endif
    }
    return *this;
}

const std::string DumpUtils::GetDumpFilePath()
{
    if (this->dumpFilePath.empty()) {
        this->dumpFilePath = PREFIX + TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_3) + ".dmp";
    }
    return this->dumpFilePath;
}

void DumpUtils::StartRecordDumpInfo()
{
#ifdef _WIN32
    ::SetUnhandledExceptionFilter(ExceptionFilter);
#endif
}