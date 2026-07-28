// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_DISASSEMBLY_MANAGER_H
#define DAP_LLDBAPI_DISASSEMBLY_MANAGER_H

#include <string>
#include <memory>
#include <map>
#include <set>
#include <optional>
#include "LLDB.h"
#include "DisassembledRange.h"

namespace Dap {

class DisassemblyManager {
public:
    DisassemblyManager() = default;
    explicit DisassemblyManager(const lldb::SBTarget &target) : disassemblyTarget(target) {}
    ~DisassemblyManager() = default;

    std::optional<DisassembledRange> GetDisassembledRange(lldb::addr_t loadAddr);
    std::optional<DisassembledRange> FindRangeByAddress(lldb::addr_t loadAddr);
    void GetSymbolAddrRangeAndInstructionList(lldb::addr_t loadAddr, lldb::SBAddress &startAddr,
                                              lldb::SBAddress &endAddr, lldb::SBInstructionList& insList);

    std::optional<DisassembledRange> FindRangeBySourceRef(int64_t sourceRef)
    {
        if (disassemblyMap.count(sourceRef) != 0) {
            return disassemblyMap[sourceRef];
        } else {
            return std::nullopt;
        }
    }

public:
    // How many instructions to put into DisassembledRange if the address is not in scope of any symbol.
    const static int NO_SYMBOL_INSTRUCTIONS = 32;

private:
    DisassembledRange AddDisassembledRange(lldb::SBAddress &startAddr, lldb::SBAddress &endAddr,
                                           lldb::SBInstructionList &insList);

private:
    std::set<DisassembledRange> disassemblySet;
    std::map<int64_t, DisassembledRange> disassemblyMap;
    lldb::SBTarget disassemblyTarget;
    const static int RANGE_INDEX_BASE = 1000;
};
};

#endif // DAP_LLDBAPI_DISASSEMBLY_MANAGER_H