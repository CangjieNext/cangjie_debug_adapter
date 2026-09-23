// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DebuggerHelper.h"
#include "LLDBApiDebugger.h"
#include "TimeTravelDebugManager.h"

using namespace Dap;

std::shared_ptr<Debugger> DebuggerHelper::CreateDebugger(const Option& option)
{
    (void)option;
    Dap::LLDBApiDebuggerOption debuggerOption;
    debuggerOption.filterInvalidLocals = true;
    std::shared_ptr<Dap::LLDBApiDebugger> debugger = std::make_shared<Dap::LLDBApiDebugger>(debuggerOption);
    // initialize time travel manager
    debugger->SetTimeTravelManager(std::make_shared<TimeTravelDebugManager>(debugger));
    debugger->SetDebugCacheHelper(std::make_unique<DebugCacheHelper>(std::static_pointer_cast<Debugger>(debugger)));
    return debugger;
}

