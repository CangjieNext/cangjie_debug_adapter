// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_EXCEPTIONBREAKPOINTSET_H
#define DAP_SERVER_EXCEPTIONBREAKPOINTSET_H

#include <set>

namespace Dap {
class ExceptionBreakpointSet {
public:
    static ExceptionBreakpointSet &GetSetInstance()
    {
        static ExceptionBreakpointSet exceptionBreakpointSet;
        return exceptionBreakpointSet;
    }
    void RemoveAllExceptionBreakPoint()
    {
        std::lock_guard<std::mutex> lock(mutex);
        exceptionBreakpointSet.clear();
    }

    void RemoveExceptionBreakPoint(const int64_t &id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        exceptionBreakpointSet.erase(id);
    }

    void AddExceptionBreakPoint(const int64_t &id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        exceptionBreakpointSet.insert(id);
    }

    std::set<int> GetAllSet()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return exceptionBreakpointSet;
    }

    bool IsExceptionBreakPoint(int breakpointId)
    {
        return exceptionBreakpointSet.count(breakpointId);
    }

private:
    // exceptionBreakpoints id
    std::mutex mutex;
    std::set<int> exceptionBreakpointSet;
};
};

#endif // DAP_SERVER_EXCEPTIONBREAKPOINTSET_H
