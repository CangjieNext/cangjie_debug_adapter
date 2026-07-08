// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_FUNCTION_BREAKPOINTMAP_H
#define DAP_SERVER_FUNCTION_BREAKPOINTMAP_H

#include <map>
#include <set>
#include <iostream>

namespace Dap {
class FunctionBreakPointMap {
public:
    static FunctionBreakPointMap& GetMapInstance()
    {
        static FunctionBreakPointMap breakPointMapInstance;
        return breakPointMapInstance;
    }

    void AddBreakPoint(const std::string &function, int number)
    {
        funcBreakPointMap[function].insert(number);
    }

    void RemoveBreakPoint(const std::string &function)
    {
        funcBreakPointMap[function].clear();
    }

    void RemoveBreakPoint(const std::string &function,  const int &number)
    {
        for (auto& breakpoint : funcBreakPointMap) {
            if (breakpoint.first == function) {
                breakpoint.second.erase(number);
            }
        }
    }

    void RemoveAllBreakPoint()
    {
        funcBreakPointMap.clear();
    }

    bool HasFunction(const std::string &function) const
    {
        if (funcBreakPointMap.count(function) == 0) {
            return false;
        } else {
            return true;
        }
    }

    std::set<int> GetSet(const std::string &function)
    {
        return funcBreakPointMap[function];
    }

    std::vector<std::set<int>> GetAllSet() const
    {
        std::vector<std::set<int>> allSet;
        for (auto it = funcBreakPointMap.begin(); it != funcBreakPointMap.end(); it++) {
            allSet.emplace_back(it->second);
        }
        return allSet;
    }

    bool IsFunctionBreakPoint(int breakpointId)
    {
        for (auto it = funcBreakPointMap.begin(); it != funcBreakPointMap.end(); it++) {
            if (it->second.find(breakpointId) != it->second.end())
                return true;
        }
        return false;
    }

private:
    // use std::set to support mutiple function breakpoints for one function
    // std::string : function;  std::set<int> : break numbers
    std::map<std::string, std::set<int>> funcBreakPointMap;
};
};

#endif // DAP_SERVER_FUNCTION_BREAKPOINTMAP_H
