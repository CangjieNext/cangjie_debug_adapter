// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DATA_BREAKPOINT_SET_H
#define DAP_SERVER_DATA_BREAKPOINT_SET_H

#include <set>

namespace Dap {
class DataBreakpointSet {
public:
    static DataBreakpointSet &GetSetInstance()
    {
        static DataBreakpointSet dataBreakPointSet;
        return dataBreakPointSet;
    }
    void RemoveAllDataBreakPoint()
    {
        std::lock_guard<std::mutex> lock(mutex);
        dataBreakPointSet.clear();
    }

    void RemoveDataBreakPoint(const int64_t &id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        dataBreakPointSet.erase(id);
    }

    void AddDataBreakPoint(const int64_t &id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        dataBreakPointSet.insert(id);
    }

    std::set<int> GetAllSet()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return dataBreakPointSet;
    }

private:
    // dataBreakpoints id
    std::mutex mutex;
    std::set<int> dataBreakPointSet;
};
};

#endif // DAP_SERVER_DATA_BREAKPOINT_SET_H
