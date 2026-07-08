// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROCESS_MONITOR_HANDLER_H
#define DAP_PROCESS_MONITOR_HANDLER_H

namespace Dap {
class ProcessMonitorHandler {
public:
    virtual void OnExited(int processId) = 0;
};
}

#endif // DAP_PROCESS_MONITOR_HANDLER_H
