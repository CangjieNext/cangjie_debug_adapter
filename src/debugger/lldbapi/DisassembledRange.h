// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_DISASSEMBLED_RANGE_H
#define DAP_LLDBAPI_DISASSEMBLED_RANGE_H

#include <string>
#include <memory>
#include <set>
#include "LLDB.h"

namespace Dap {

class DisassembledRange {
public:
    DisassembledRange() = default;
    // only for DisassemblyManager::FindRangeByAddress
    DisassembledRange(lldb::SBTarget &sbTarget, lldb::addr_t endLoadAddress)
        : rangeTarget(sbTarget), endLoadAddr(endLoadAddress) {}
    DisassembledRange(lldb::SBTarget &sbTarget, int64_t idx, lldb::SBAddress &startAddress, lldb::SBAddress &endAddress,
        lldb::SBInstructionList &insList);
    virtual ~DisassembledRange() = default;

    std::string GetSourceContent();

    // for DisassemblyManager::disassemblySet
    bool operator < (const DisassembledRange &other) const
    {
        return endLoadAddr < other.endLoadAddr;
    }

    lldb::addr_t GetStartLoadAddress()
    {
        return startLoadAddr;
    }

    lldb::addr_t GetEndLoadAddress()
    {
        return endLoadAddr;
    }

    int64_t GetIndex() const
    {
        return index;
    }

    std::string GetSymbolName() const
    {
        return symbolName;
    }

    int64_t GetLineNumByAddress(lldb::addr_t loadAddr)
    {
        auto it = instructionAddresses.lower_bound(loadAddr);
        if (it == instructionAddresses.end()) {
            return -1;
        } else {
            return std::distance(instructionAddresses.begin(), it) + DESCRIPTION_LINES + 1;
        }
    }

public:
    constexpr static int MAX_MNEMONIC_LEN = 7;
    constexpr static int MAX_OPERANDS_LEN = 28;

private:
    lldb::SBTarget rangeTarget;
    int64_t index = 0;
    lldb::addr_t startLoadAddr = 0;
    lldb::addr_t  endLoadAddr = 0;
    lldb::SBAddress startAddr;
    lldb::SBAddress endAddr;
    std::string symbolName;
    lldb::SBInstructionList instructionList;
    std::set<lldb::addr_t> instructionAddresses;

    constexpr static int DESCRIPTION_LINES = 2;
};
};

#endif // DAP_LLDBAPI_DISASSEMBLED_RANGE_H