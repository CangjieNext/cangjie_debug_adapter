// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DapLog.h"
#include "LLDB.h"
#include "DisassemblyManager.h"

using namespace std;
using namespace Dap;
using namespace lldb;

std::optional<DisassembledRange> DisassemblyManager::GetDisassembledRange(lldb::addr_t loadAddr)
{
    std::optional<DisassembledRange> optionalRange = FindRangeByAddress(loadAddr);
    if (optionalRange.has_value()) {
        return optionalRange.value();
    }

    SBAddress sbAddress(loadAddr, disassemblyTarget);
    if (!sbAddress.IsValid()) {
        LOG_INFO("SBAddress is invalid !");
        return std::nullopt;
    }

    SBAddress startAddr;
    SBAddress endAddr;
    SBInstructionList insList;
    GetSymbolAddrRangeAndInstructionList(loadAddr, startAddr, endAddr, insList);

    return AddDisassembledRange(startAddr, endAddr, insList);
}

std::optional<DisassembledRange> DisassemblyManager::FindRangeByAddress(lldb::addr_t loadAddr)
{
    auto it = disassemblySet.upper_bound(DisassembledRange(disassemblyTarget, loadAddr));
    if (it == disassemblySet.end()) {
        return std::nullopt;
    } else {
        DisassembledRange possibleRange = *it;
        // EndLoadAddress is not included
        if (possibleRange.GetStartLoadAddress() <= loadAddr && possibleRange.GetEndLoadAddress() > loadAddr) {
            return possibleRange;
        } else {
            return std::nullopt;
        }
    }
}

DisassembledRange DisassemblyManager::AddDisassembledRange(SBAddress &startAddr, SBAddress &endAddr,
                                                           SBInstructionList &insList)
{
    int64_t idx = disassemblyMap.size() + RANGE_INDEX_BASE;
    DisassembledRange range(disassemblyTarget, idx, startAddr, endAddr, insList);
    disassemblyMap.emplace(std::make_pair(idx, range));
    disassemblySet.emplace(range);
    return range;
}

void DisassemblyManager::GetSymbolAddrRangeAndInstructionList(lldb::addr_t loadAddr, lldb::SBAddress &startAddr,
                                                              lldb::SBAddress &endAddr, SBInstructionList& insList)
{
    SBAddress sbAddress(loadAddr, disassemblyTarget);
    SBSymbol symbol = sbAddress.GetSymbol();
    if (symbol.IsValid()) {
        startAddr = symbol.GetStartAddress();
        endAddr = symbol.GetEndAddress();
    } else {
        startAddr = sbAddress;
        insList = disassemblyTarget.ReadInstructions(startAddr, DisassemblyManager::NO_SYMBOL_INSTRUCTIONS + 1);
        endAddr = insList.GetSize() > 0 ? insList.GetInstructionAtIndex(insList.GetSize() - 1).GetAddress()
                                        : startAddr;
    }
}
