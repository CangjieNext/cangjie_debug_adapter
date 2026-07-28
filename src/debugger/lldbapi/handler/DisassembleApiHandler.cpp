// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DisassembleApiHandler.h"

#include <climits>

#include "CommonUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr DisassembleApiHandler::HandleRequest(UniqueRequestPtr request)
{
    auto reqPtr = dynamic_cast<DisassembleRequest*>(request.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: DisassembleRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<DisassembleResponse>(), error, request->seq);
    }
    lldb::addr_t address;
    if (!CommonUtils::HexStringToUnsignedLongLong(reqPtr->arguments.memoryReference, address)) {
        string error = "ERROR: Invalid memory reference.";
        return debugger->CompleteFailureResponse(std::make_unique<ReadMemoryResponse>(), error, request->seq);
    }
    int64_t offset = reqPtr->arguments.offset.has_value() ? reqPtr->arguments.offset.value() : 0;
    startAddress = address + offset;
    instructionOffset = reqPtr->arguments.offset.has_value() ? reqPtr->arguments.instructionOffset.value() : 0;
    forwardCount = reqPtr->arguments.instructionCount + instructionOffset;
    proLine = 0;

    ReadInstructions();
    std::unique_ptr<DisassembleResponse> responsePointer = std::make_unique<DisassembleResponse>();
    debugger->InitResponse(*responsePointer,  request->seq, true);
    responsePointer->instructions = std::move(dapInstructions);
    return responsePointer;
}

void DisassembleApiHandler::ReadInstructions()
{
    SBTarget &target = debugger->GetSbTarget();
    if (instructionOffset < 0) {
        ReadInstructionsBackwards();
    }
    if (forwardCount > 0) {
        vector<SBInstruction> sbInstructions;
        int64_t currentCount = forwardCount;
        SBAddress baseSBAddress = SBAddress(startAddress, target);
        while (true) {
            SBInstructionList sbInstructionList = target.ReadInstructions(baseSBAddress, currentCount);
            if (sbInstructionList.GetSize() == 0) {
                lldb::addr_t addr = baseSBAddress.GetLoadAddress(target);
                LOG_INFO("Maybe bad byte: ",
                         CommonUtils::Dec2HexAddress(addr, CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU));
                baseSBAddress = SBAddress(addr + 1, target);
                continue;
            }
            for (uint32_t i = 0; i < (uint32_t)sbInstructionList.GetSize(); i++) {
                SBInstruction sbInstruction = sbInstructionList.GetInstructionAtIndex(i);
                sbInstructions.emplace_back(sbInstruction);
            }
            if (sbInstructions.size() >= (size_t)forwardCount) {
                break;
            } else {
                currentCount = forwardCount - sbInstructions.size();
                SBInstruction instruction = sbInstructionList.GetInstructionAtIndex(sbInstructionList.GetSize() - 1);
                lldb::addr_t newAddress = instruction.GetAddress().GetLoadAddress(target) + instruction.GetByteSize();
                baseSBAddress = SBAddress(newAddress, target);
            }
        }
        for (uint32_t k = instructionOffset > 0 ? instructionOffset : 0; k < forwardCount; k++) {
            SBInstruction sbInstruction = sbInstructions.at(k);
            DisassembledInstruction dapInstruction = GenDapInstruction(sbInstruction);
            dapInstructions.emplace_back(dapInstruction);
        }
    }
}

void DisassembleApiHandler::ReadInstructionsBackwards()
{
    SBTarget &target = debugger->GetSbTarget();
    vector<SBInstructionList> sbInstrLists;
    vector<SBInstruction> sbInstructions;
    SBAddress startAddrBackwards;
    if (!GenStartAddrBackwards(startAddrBackwards)) {
        return;
    }
    LOG_INFO("startAddrBackwards=", CommonUtils::Dec2HexAddress(startAddrBackwards.GetLoadAddress(target),
                                                                CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU));
    while (true) {
        SBInstructionList instrList = target.ReadInstructions(startAddrBackwards, -instructionOffset);
        if (instrList.GetSize() == 0) {
            lldb::addr_t addr = startAddrBackwards.GetLoadAddress(target);
            LOG_INFO("Maybe bad byte: ", CommonUtils::Dec2HexAddress(addr, CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU));
            startAddrBackwards =  SBAddress(addr + 1, target);
            continue;
        }
        SBInstruction instruction = instrList.GetInstructionAtIndex(instrList.GetSize() - 1);
        lldb::addr_t endAddress = instruction.GetAddress().GetLoadAddress(target) + instruction.GetByteSize() - 1;
        if (endAddress < startAddress - 1) {
            if ((int)instrList.GetSize() == -instructionOffset) {
                sbInstrLists.clear(); // out of target range
            }
            sbInstrLists.emplace_back(instrList); // maybe in target range
            startAddrBackwards =  SBAddress(endAddress + 1, target);
        } else {
            sbInstrLists.emplace_back(instrList);
            break;
        }
    }
    for (auto sbInstrList : sbInstrLists) {
        for (uint32_t i = 0; i < sbInstrList.GetSize(); i++) {
            SBInstruction sbInstruction = sbInstrList.GetInstructionAtIndex(i);
            lldb::addr_t loadAddr = sbInstruction.GetAddress().GetLoadAddress(target);
            if (loadAddr >= startAddress) {
                break; // already reached forward range
            }
            sbInstructions.emplace_back(sbInstruction);
        }
    }
    int start = sbInstructions.size() > (size_t)(-instructionOffset) ? instructionOffset : -(int)sbInstructions.size();
    for (int j = start; j < 0; j++) {
        DisassembledInstruction dapInstruction = GenDapInstruction(sbInstructions[sbInstructions.size() + (size_t)j]);
        dapInstructions.emplace_back(dapInstruction);
    }
}

bool DisassembleApiHandler::GenStartAddrBackwards(SBAddress& startAddr)
{
    if (instructionOffset > 0) {
        return false;
    }
    constexpr int64_t maxOffset = INT64_MAX / INSTRUCTION_BYTES;
    if (instructionOffset < -maxOffset) {
        LOG_ERROR("instructionOffset is too large, potential overflow");
        return false;
    }
    lldb::addr_t tryAddress;
    uint64_t positiveOffset = static_cast<uint64_t>(-instructionOffset);
    if (positiveOffset * INSTRUCTION_BYTES > startAddress) {
        tryAddress = startAddress;
    } else {
        tryAddress = startAddress + instructionOffset * INSTRUCTION_BYTES;
    }
    SBAddress adjustingSBAddress(tryAddress, debugger->GetSbTarget());
    SBSymbol symbol = adjustingSBAddress.GetSymbol();
    // need a aligned address
    if (symbol.IsValid()) {
        LOG_INFO("tartAddrBackwards: head of symbol ", symbol.GetName(), ", type is ", symbol.GetType());
        startAddr = symbol.GetStartAddress();
        return true;
    } else {
        if (!adjustingSBAddress.GetSection().IsValid()) {
            adjustingSBAddress = SBAddress(startAddress, debugger->GetSbTarget());
            if (!adjustingSBAddress.GetSection().IsValid()) {
                LOG_INFO("Invalid section !");
                return false;
            }
        }
        LOG_INFO("tartAddrBackwards: head of section ", adjustingSBAddress.GetSection().GetName(),
                 ", type is ", adjustingSBAddress.GetSection().GetSectionType());
        startAddr = SBAddress(adjustingSBAddress.GetSection().GetLoadAddress(debugger->GetSbTarget()),
                              debugger->GetSbTarget());
        return true;
    }
}

DisassembledInstruction DisassembleApiHandler::GenDapInstruction(SBInstruction &sbInstruction)
{
    constexpr static int rawdataWidth = 2;
    DisassembledInstruction dapInstruction;
    SBTarget &target = debugger->GetSbTarget();
    lldb::addr_t loadAddr = sbInstruction.GetAddress().GetLoadAddress(target);
    dapInstruction.address = CommonUtils::Dec2HexAddress(loadAddr, CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU);
    SBError sbError;
    size_t instrBytes = sbInstruction.GetByteSize();
    unique_ptr<unsigned char[]> buf = make_unique<unsigned char[]>(instrBytes);
    sbInstruction.GetData(target).ReadRawData(sbError, 0, buf.get(), instrBytes);
    if (sbError.Fail()) {
        LOG_INFO("SBInstruction ReadRawData failed!");
    } else {
        dapInstruction.instructionBytes = "";
        for (size_t j = 0; j < instrBytes; j++) {
            if (j > 0) {
                dapInstruction.instructionBytes->append(" ");
            }
            std::stringstream ss;
            ss << std::setw(rawdataWidth)  << std::setfill('0') << std::hex <<
                static_cast<unsigned int>(buf[j]);
            dapInstruction.instructionBytes->append(ss.str());
        }
    }
    const char *mnemonic = sbInstruction.GetMnemonic(target);
    const char *operands = sbInstruction.GetOperands(target);
    const char *comment = sbInstruction.GetComment(target);
    dapInstruction.instruction =
        CommonUtils::FormatCharPointer(mnemonic, DisassembledRange::MAX_MNEMONIC_LEN, ' ', true);
    if (operands != nullptr && operands[0] != '\0') {
        dapInstruction.instruction.append(" ");
        if (comment != nullptr && comment[0] != '\0') {
            dapInstruction.instruction.append(
                CommonUtils::FormatCharPointer(operands, DisassembledRange::MAX_OPERANDS_LEN, ' ', true));
        } else {
            dapInstruction.instruction.append(operands);
        }
    }
    if (comment != nullptr && comment[0] != '\0') {
        dapInstruction.instruction.append("; ");
        dapInstruction.instruction.append(comment);
    }
    AddDapInstrSourceInfo(sbInstruction, dapInstruction);
    return dapInstruction;
}

void DisassembleApiHandler::AddDapInstrSourceInfo(SBInstruction &sbInstruction, DisassembledInstruction &dapInstruction)
{
    SBLineEntry sbLineEntry = sbInstruction.GetAddress().GetLineEntry();
    if (sbLineEntry.IsValid()) {
        char dstPath[PATH_MAX + 1] = { 0x00 };
        sbLineEntry.GetFileSpec().GetPath(dstPath, PATH_MAX);
        // omitted afterwards if this instruction maps to the same source file as the previous instruction
        if (preInstrSourcePath != dstPath) {
            dapInstruction.location = Source();
            dapInstruction.location->path = dstPath;
            preInstrSourcePath = dstPath;
        }
        unsigned line = sbLineEntry.GetLine();
        if (line == 0) {
            line = proLine;
        } else {
            proLine = line;
        }
        dapInstruction.line = debugger->GetOption().linesStartAt1 ? line : line - 1;
        dapInstruction.column = sbLineEntry.GetColumn();
    }
    SBSymbol symbol = sbInstruction.GetAddress().GetSymbol();
    if (symbol.IsValid()) {
        string symbolName = symbol.GetName();
        // only set symbol info at the first instruction
        if (preInstrSymbol != symbolName) {
            dapInstruction.symbol = symbolName;
            preInstrSymbol = symbolName;
        }
    }
}
