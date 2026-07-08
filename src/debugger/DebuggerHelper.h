// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DEBUGGER_HELPER_H
#define DAP_SERVER_DEBUGGER_HELPER_H

#include "Option.h"
#include "Debugger.h"

namespace Dap {
class DebuggerHelper {
public:
    static std::shared_ptr<Debugger> CreateDebugger(const Dap::Option& option);
};
};

#endif // DAP_SERVER_DEBUGGER_HELPER_H