// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
#ifdef _WIN32
#include "WindowsPlatform.h"
#include <thread>
#include <windows.h>
#include "DapLog.h"

using namespace Dap;

int WindowsPlatform::InterruptProcess(int /* debuggerPid */, int debuggeePid) const
{
    HMODULE hmodule = ::LoadLibrary("C:\\Windows\\System32\\kernel32.dll");
    if (hmodule) {
        BOOL ret = FALSE;
        FARPROC procAddress = GetProcAddress(hmodule, "DebugBreakProcess");
        if (procAddress != nullptr) {
            HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)debuggeePid);
            if (process != nullptr) {
                typedef BOOL WINAPI (*DebugBreakProcessFunc)(HANDLE);
                DebugBreakProcessFunc pDebugBreakProcessFunc =
                    reinterpret_cast<DebugBreakProcessFunc>(reinterpret_cast<void*>(procAddress));
                ret = (*pDebugBreakProcessFunc)(process);
                CloseHandle(process);
            }
        } else {
            LOG_WARN("Running on Windows 2000 or earlier, DebugBreakProcess is not available !");
        }
        FreeLibrary(hmodule);
        hmodule = nullptr;

        if (ret == TRUE) {
            return 0;
        } else {
            LOG_ERROR("Call DebugBreakProcess failed !");
            return -1;
        }
    }
    LOG_ERROR("Kernel32.dll not loaded !");
    return -1;
}

unsigned int WindowsPlatform::GenRandomUInt() const
{
    HCRYPTPROV hCryptProv;
    CryptAcquireContext((HCRYPTPROV*)&hCryptProv, nullptr, nullptr, PROV_RSA_FULL, 0);
    unsigned int rnd;
    if (!CryptGenRandom(hCryptProv, UINT_LEN, (BYTE*)&rnd)) {
        rnd = 0;
    }
    CryptReleaseContext(hCryptProv, 0);
    return rnd;
}

std::string WindowsPlatform::GetShellNameByPID(int /* pid */) const
{
    return "";
}

#endif // _WIN32