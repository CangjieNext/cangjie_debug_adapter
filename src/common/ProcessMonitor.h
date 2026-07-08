// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROCESS_MONITOR_H
#define DAP_PROCESS_MONITOR_H

#include <thread>
#include <map>
#include <vector>
#include <shared_mutex>
#include "ProcessMonitorHandler.h"

namespace Dap {
class ProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor();
    void AddProcess(const int &processId, const std::shared_ptr<ProcessMonitorHandler> &handler);
    void RemoveProcess(const int &processId);
    void ClearProcesses();
    bool Start();
    bool Stop();
    bool IsStart() const;

private:
    static void ThreadFunc(ProcessMonitor &monitor);
    int HandlerCount();
    std::vector<int> GetPidList();
    void CheckProcess();

private:
    bool isStart;
    std::unique_ptr<std::thread> thread;
    std::shared_mutex handlerMutex;
    std::map<int, std::shared_ptr<ProcessMonitorHandler>> handlerMap;
};
}

#endif // DAP_PROCESS_MONITOR_H
