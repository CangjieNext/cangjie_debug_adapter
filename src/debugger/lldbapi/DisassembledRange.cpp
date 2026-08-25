// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DisassembledRange.h"

#include <climits>

#include "LLDB.h"
#include "CommonUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace lldb;

DisassembledRange::DisassembledRange(lldb::SBTarget &sbTarget, int64_t idx, lldb::SBAddress &startAddress,
    lldb::SBAddress &endAddress, lldb::SBInstructionList &insList)
    : rangeTarget(sbTarget), index(idx), startAddr(startAddress), endAddr(endAddress), instructionList(insList)
{
    startLoadAddr = startAddr.GetLoadAddress(rangeTarget);
    endLoadAddr = endAddr.GetLoadAddress(rangeTarget);
    SBSymbol symbol = startAddr.GetSymbol();
    if (symbol.IsValid()) {
        symbolName = "@" + string(symbol.GetName());
    } else {
        symbolName = "@" + CommonUtils::Dec2HexAddress(startLoadAddr, CommonUtils::hexAddressWidth64BitsCpu) + ".."
                     + CommonUtils::Dec2HexAddress(endLoadAddr, CommonUtils::hexAddressWidth64BitsCpu);
    }
    for (uint32_t i = 0; i < instructionList.GetSize(); i++) {
        SBInstruction instrt = instructionList.GetInstructionAtIndex(i);
        lldb::addr_t loadAddr = instrt.GetAddress().GetLoadAddress(rangeTarget);
        instructionAddresses.insert(loadAddr);
    }
}

std::string DisassembledRange::GetSourceContent()
{
    string description;
    SBSymbol sbSymbol = startAddr.GetSymbol();
    if (sbSymbol.IsValid()) {
        SBStream sbStream;
        sbSymbol.GetDescription(sbStream);
        description = string(sbStream.GetData());
    } else {
        description = "No Symbol Info";
    }
    string sourceLocation;
    SBLineEntry sbLineEntry = startAddr.GetLineEntry();
    if (sbLineEntry.IsValid()) {
        char dstPath[PATH_MAX + 1] = { 0x00 };
        sbLineEntry.GetFileSpec().GetPath(dstPath, PATH_MAX);
        sourceLocation = string(dstPath) + ":" + std::to_string(sbLineEntry.GetLine());
    } else {
        sourceLocation = "unknown";
    }
    string text = "; " + description + "\n; Source location: " + sourceLocation + "\n"; // line 1, line 2
    for (uint32_t i = 0; i < instructionList.GetSize(); i++) {
        SBInstruction sbInstruction = instructionList.GetInstructionAtIndex(i);
        lldb::addr_t loadAddr = sbInstruction.GetAddress().GetLoadAddress(rangeTarget);
        const char *mnemonic = sbInstruction.GetMnemonic(rangeTarget);
        const char *operands = sbInstruction.GetOperands(rangeTarget);
        const char *comment = sbInstruction.GetComment(rangeTarget);
        text.append(CommonUtils::Dec2HexAddress(loadAddr, CommonUtils::hexAddressWidth64BitsCpu));
        text.append(": ");
        text.append(CommonUtils::FormatCharPointer(mnemonic, MAX_MNEMONIC_LEN, ' ', true));
        if (operands != nullptr && operands[0] != '\0') {
            text.append(" ");
            if (comment != nullptr && comment[0] != '\0') {
                text.append(CommonUtils::FormatCharPointer(operands, MAX_OPERANDS_LEN, ' ', true));
            } else {
                text.append(operands);
            }
        }
        if (comment != nullptr && comment[0] != '\0') {
            text.append("; ");
            text.append(comment);
        }
        text.append("\n");
    }
    return text;
}

