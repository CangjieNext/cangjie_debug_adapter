// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_BREAKPOINTMAP_H
#define DAP_SERVER_BREAKPOINTMAP_H

#include <map>
#include <set>
#include <iostream>

namespace Dap {
class BreakPointMap {
public:
    static BreakPointMap& GetMapInstance()
    {
        static BreakPointMap breakPointMapTemp;
        return breakPointMapTemp;
    }

    void AddBreakPoint(const std::string &path, const int &number)
    {
        breakPointMap[path].insert(number);
    }

    void RemoveBreakPoint(const std::string &path)
    {
        breakPointMap[path].clear();
    }

    void RemoveBreakPoint(const std::string &path, const int &number)
    {
        for (auto& breakpoint : breakPointMap) {
            if (breakpoint.first == path) {
                breakpoint.second.erase(number);
            }
        }
    }

    bool HasPath(const std::string &path) const
    {
        if (breakPointMap.count(path) == 0) {
            return false;
        } else {
            return true;
        }
    }

    std::set<int> GetSet(const std::string &path)
    {
        return breakPointMap[path];
    }

    bool IsSourceBreakPoint(int breakpointId)
    {
        for (auto it = breakPointMap.begin(); it != breakPointMap.end(); it++) {
            if (it->second.find(breakpointId) != it->second.end())
                return true;
        }
        return false;
    }

private:
    std::map<std::string, std::set<int>> breakPointMap;
};
};

#endif // DAP_SERVER_BREAKPOINTMAP_H
