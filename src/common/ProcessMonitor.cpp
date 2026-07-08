// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include "CommonUtils.h"
#include "DapLog.h"
#include "ProcessMonitor.h"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#elif __APPLE__
#include <libProc.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

using namespace Dap;

ProcessMonitor::ProcessMonitor() : isStart(false), thread(nullptr) {}

ProcessMonitor::~ProcessMonitor()
{
    ClearProcesses();
}

void ProcessMonitor::ThreadFunc(ProcessMonitor &monitor)
{
    static int timeSpanMillis = 500;
    while (true) {
        if (!monitor.isStart) {
            break;
        }
        if (monitor.HandlerCount() > 0) {
            monitor.CheckProcess();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeSpanMillis));
    }
}

int ProcessMonitor::HandlerCount()
{
    std::shared_lock<std::shared_mutex> lock(handlerMutex);
    return handlerMap.size();
}

std::vector<int> ProcessMonitor::GetPidList()
{
    std::vector<int> res;
#ifdef _WIN32
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcess == INVALID_HANDLE_VALUE) {
        return res;
    }
    bool hasMore = Process32First(hProcess, &pe32);
    while (hasMore) {
        res.push_back(pe32.th32ProcessID);
        hasMore = Process32Next(hProcess, &pe32);
    }
    CloseHandle(hProcess);
#elif __APPLE__
    pid_t pids[2048];
    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    int n_proc = bytes / sizeof(pids[0]);
    for (int i = 0; i < n_proc; i++) {
        struct proc_bsdinfo proc;
        int st = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, &proc, PROC_PIDTBSDINFO_SIZE);
        if (st == PROC_PIDTBSDINFO_SIZE) {
            res.push_back(pids[i]);
        }
    }
#else
    DIR *dir = opendir("/proc");
    if (dir == nullptr) {
        return res;
    }
    dirent *tmp = nullptr;
    int pid = -1;
    while ((tmp = readdir(dir))) {
        if (CommonUtils::StringToInt(std::string(tmp->d_name), pid)) {
            res.push_back(pid);
        }
    }
    closedir(dir);
#endif
    return res;
}

void ProcessMonitor::CheckProcess()
{
    std::vector<int> pidList = GetPidList();
    if (pidList.size() > 0) {
        std::vector<int> closedPidList;
        std::unique_lock<std::shared_mutex> lock(handlerMutex);
        for (auto e : handlerMap) {
            if (find(pidList.begin(), pidList.end(), e.first) != pidList.end()) {
                continue;
            }
            if (e.second != nullptr) {
                e.second->OnExited(e.first);
            }
            closedPidList.push_back(e.first);
        }
        for (auto id : closedPidList) {
            // erase invalid handler
            handlerMap.erase(id);
        }
    } else {
        LOG_ERROR("failed to get pid list.");
    }
}

void ProcessMonitor::AddProcess(const int &processId, const std::shared_ptr<ProcessMonitorHandler> &handler)
{
    std::unique_lock<std::shared_mutex> lock(handlerMutex);
    if (handlerMap.count(processId) == 0) {
        handlerMap[processId] = handler;
    }
}

void ProcessMonitor::RemoveProcess(const int &processId)
{
    std::unique_lock<std::shared_mutex> lock(handlerMutex);
    if (handlerMap.count(processId) != 0) {
        handlerMap.erase(processId);
    }
}

void ProcessMonitor::ClearProcesses()
{
    std::unique_lock<std::shared_mutex> lock(handlerMutex);
    handlerMap.clear();
}

bool ProcessMonitor::Start()
{
    isStart = true;
    thread = std::make_unique<std::thread>(ThreadFunc, std::ref(*this));
    thread->detach();
    return true;
}

bool ProcessMonitor::Stop()
{
    isStart = false;
    ClearProcesses();
    return true;
}

bool ProcessMonitor::IsStart() const
{
    return isStart;
}