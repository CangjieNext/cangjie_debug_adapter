// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_INSTRUCTION_BREAKPOINTMAP_H
#define DAP_LLDBAPI_INSTRUCTION_BREAKPOINTMAP_H

#include <map>
#include <iostream>

namespace Dap {
class InstructionBreakPointMap {
public:
    static InstructionBreakPointMap& GetMapInstance()
    {
        static InstructionBreakPointMap breakPointMapInstance;
        return breakPointMapInstance;
    }

    void AddBreakPoint(int breakpointId, const std::string &instructionReference)
    {
        breakPointMap[breakpointId] = instructionReference;
    }

    void RemoveBreakPoint(int breakpointId)
    {
        breakPointMap[breakpointId].clear();
    }

    void RemoveAllBreakPoint()
    {
        breakPointMap.clear();
    }

    std::string GetInstructionReference(int breakpointId)
    {
        return breakPointMap[breakpointId];
    }

    std::vector<int> GetAllBreakpointId() const
    {
        std::vector<int> breakpointIds;
        for (auto it = breakPointMap.begin(); it != breakPointMap.end(); it++) {
            breakpointIds.emplace_back(it->first);
        }
        return breakpointIds;
    }

    bool IsInstructionBreakPoint(int breakpointId)
    {
        return breakPointMap.count(breakpointId) != 0;
    }

private:
    // int : breakpoint id;  std::string : instructionReference
    std::map<int, std::string> breakPointMap;
};
};

#endif // DAP_LLDBAPI_INSTRUCTION_BREAKPOINTMAP_H
