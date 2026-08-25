// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DisassembleApiHandler.h"

#include <algorithm>
#include <limits>

#include "CommonUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

namespace {
constexpr uint64_t MAX_CONSECUTIVE_BAD_BYTES = 4096;
}

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
        return debugger->CompleteFailureResponse(std::make_unique<DisassembleResponse>(), error, request->seq);
    }
    int64_t offset = reqPtr->arguments.offset.value_or(0);
    uint64_t offsetMagnitude = offset < 0 ? static_cast<uint64_t>(-(offset + 1)) + 1 :
                                           static_cast<uint64_t>(offset);
    if ((offset < 0 && offsetMagnitude > address) ||
        (offset >= 0 && offsetMagnitude > std::numeric_limits<lldb::addr_t>::max() - address)) {
        string error = "ERROR: Memory reference offset is out of range.";
        return debugger->CompleteFailureResponse(std::make_unique<DisassembleResponse>(), error, request->seq);
    }
    startAddress = offset < 0 ? address - offsetMagnitude : address + offsetMagnitude;
    instructionOffset = reqPtr->arguments.instructionOffset.value_or(0);
    if (reqPtr->arguments.instructionCount < 0) {
        string error = "ERROR: Instruction count must not be negative.";
        return debugger->CompleteFailureResponse(std::make_unique<DisassembleResponse>(), error, request->seq);
    }
    if (instructionOffset > 0 &&
        reqPtr->arguments.instructionCount > std::numeric_limits<int64_t>::max() - instructionOffset) {
        string error = "ERROR: Instruction range is out of bounds.";
        return debugger->CompleteFailureResponse(std::make_unique<DisassembleResponse>(), error, request->seq);
    }
    instructionCount = static_cast<uint64_t>(reqPtr->arguments.instructionCount);
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
    if (instructionOffset < 0) {
        ReadInstructionsBackwards();
    }
    if (forwardCount > 0) {
        ReadForwardInstructions();
    }
}

void DisassembleApiHandler::ReadForwardInstructions()
{
    SBTarget &target = debugger->GetSbTarget();
    vector<SBInstruction> sbInstructions;
    uint64_t currentCount = static_cast<uint64_t>(forwardCount);
    uint64_t skippedBadBytes = 0;
    SBAddress baseSBAddress = SBAddress(startAddress, target);
    lldb::addr_t nextInstructionAddress = LLDB_INVALID_ADDRESS;
    while (static_cast<uint64_t>(sbInstructions.size()) < static_cast<uint64_t>(forwardCount) &&
           skippedBadBytes < MAX_CONSECUTIVE_BAD_BYTES) {
        uint32_t readCount = currentCount > std::numeric_limits<uint32_t>::max() ?
            std::numeric_limits<uint32_t>::max() : static_cast<uint32_t>(currentCount);
        SBInstructionList sbInstructionList = target.ReadInstructions(baseSBAddress, readCount);
        if (sbInstructionList.GetSize() == 0) {
            if (!HandleForwardBadByte(target, baseSBAddress, skippedBadBytes)) {
                break;
            }
            continue;
        }
        skippedBadBytes = 0;
        lldb::addr_t currentAddress = baseSBAddress.GetLoadAddress(target);
        bool invalidInstruction = false;
        for (size_t i = 0; i < sbInstructionList.GetSize(); i++) {
            SBInstruction sbInstruction = sbInstructionList.GetInstructionAtIndex(static_cast<uint32_t>(i));
            if (!IsInstructionValid(sbInstruction, currentAddress, nextInstructionAddress)) {
                invalidInstruction = true;
                break;
            }
            sbInstructions.emplace_back(sbInstruction);
            nextInstructionAddress = sbInstruction.GetAddress().GetLoadAddress(target) + sbInstruction.GetByteSize();
            if (static_cast<uint64_t>(sbInstructions.size()) == static_cast<uint64_t>(forwardCount)) {
                break;
            }
        }
        if (invalidInstruction) {
            LOG_INFO("Invalid or non-advancing instruction returned by LLDB.");
            break;
        }
        if (static_cast<uint64_t>(sbInstructions.size()) >= static_cast<uint64_t>(forwardCount)) {
            break;
        }
        currentCount = static_cast<uint64_t>(forwardCount) - static_cast<uint64_t>(sbInstructions.size());
        if (nextInstructionAddress == LLDB_INVALID_ADDRESS || nextInstructionAddress <= currentAddress) {
            LOG_INFO("Instruction address did not advance.");
            break;
        }
        baseSBAddress = SBAddress(nextInstructionAddress, target);
    }
    uint64_t firstInstruction = instructionOffset > 0 ? static_cast<uint64_t>(instructionOffset) : 0;
    uint64_t availableCount = std::min(static_cast<uint64_t>(sbInstructions.size()),
                                       static_cast<uint64_t>(forwardCount));
    AppendDapInstructions(sbInstructions, firstInstruction, availableCount);
}

bool DisassembleApiHandler::HandleForwardBadByte(SBTarget &target, SBAddress &baseSBAddress,
                                                 uint64_t &skippedBadBytes)
{
    lldb::addr_t addr = baseSBAddress.GetLoadAddress(target);
    LOG_INFO("Maybe bad byte: ",
             CommonUtils::Dec2HexAddress(addr, CommonUtils::hexAddressWidth64BitsCpu));
    if (addr == LLDB_INVALID_ADDRESS || addr == std::numeric_limits<lldb::addr_t>::max()) {
        return false;
    }
    baseSBAddress = SBAddress(addr + 1, target);
    ++skippedBadBytes;
    return true;
}

void DisassembleApiHandler::AppendDapInstructions(const vector<SBInstruction> &sbInstructions,
                                                  uint64_t firstInstruction, uint64_t availableCount)
{
    for (uint64_t k = firstInstruction; k < availableCount; k++) {
        SBInstruction sbInstruction = sbInstructions[static_cast<size_t>(k)];
        DisassembledInstruction dapInstruction = GenDapInstruction(sbInstruction);
        dapInstructions.emplace_back(dapInstruction);
    }
}

bool DisassembleApiHandler::IsInstructionValid(const SBInstruction &sbInstruction, lldb::addr_t currentAddress,
    lldb::addr_t nextInstructionAddress) const
{
    SBTarget &target = debugger->GetSbTarget();
    lldb::addr_t instructionAddress = sbInstruction.GetAddress().GetLoadAddress(target);
    size_t instructionSize = sbInstruction.GetByteSize();
    if (!sbInstruction.IsValid() || instructionAddress == LLDB_INVALID_ADDRESS || instructionSize == 0 ||
        instructionSize > std::numeric_limits<lldb::addr_t>::max() - instructionAddress ||
        currentAddress == LLDB_INVALID_ADDRESS || instructionAddress < currentAddress ||
        (nextInstructionAddress != LLDB_INVALID_ADDRESS && instructionAddress < nextInstructionAddress)) {
        return false;
    }
    return true;
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
                                                                CommonUtils::hexAddressWidth64BitsCpu));
    uint64_t requestedCount = static_cast<uint64_t>(-(instructionOffset + 1)) + 1;
    uint64_t skippedBadBytes = 0;
    while (skippedBadBytes < MAX_CONSECUTIVE_BAD_BYTES) {
        uint32_t readCount = requestedCount > std::numeric_limits<uint32_t>::max() ?
            std::numeric_limits<uint32_t>::max() : static_cast<uint32_t>(requestedCount);
        SBInstructionList instrList = target.ReadInstructions(startAddrBackwards, readCount);
        if (instrList.GetSize() == 0) {
            if (!HandleBackwardBadByte(target, startAddrBackwards, skippedBadBytes)) {
                break;
            }
            continue;
        }
        skippedBadBytes = 0;
        SBInstruction instruction = instrList.GetInstructionAtIndex(
            static_cast<uint32_t>(instrList.GetSize() - 1));
        lldb::addr_t instructionAddress = instruction.GetAddress().GetLoadAddress(target);
        size_t instructionSize = instruction.GetByteSize();
        lldb::addr_t currentAddress = startAddrBackwards.GetLoadAddress(target);
        if (instructionAddress == LLDB_INVALID_ADDRESS || instructionSize == 0 ||
            instructionSize - 1 > std::numeric_limits<lldb::addr_t>::max() - instructionAddress) {
            LOG_INFO("Unable to advance while reading instructions backwards.");
            break;
        }
        lldb::addr_t endAddress = instructionAddress + instructionSize - 1;
        bool endsBeforeTarget = endAddress < startAddress && startAddress - endAddress > 1;
        if (endsBeforeTarget) {
            lldb::addr_t nextAddress = endAddress + 1;
            if (currentAddress == LLDB_INVALID_ADDRESS || nextAddress <= currentAddress) {
                LOG_INFO("Instruction address did not advance while reading backwards.");
                break;
            }
            if (instrList.GetSize() == readCount) {
                sbInstrLists.clear(); // out of target range
            }
            sbInstrLists.emplace_back(instrList); // maybe in target range
            startAddrBackwards = SBAddress(nextAddress, target);
        } else {
            sbInstrLists.emplace_back(instrList);
            break;
        }
    }
    CollectBackwardInstructions(sbInstrLists, sbInstructions);
    AppendBackwardDapInstructions(sbInstructions, requestedCount);
}

bool DisassembleApiHandler::HandleBackwardBadByte(SBTarget &target, SBAddress &startAddrBackwards,
                                                  uint64_t &skippedBadBytes)
{
    lldb::addr_t addr = startAddrBackwards.GetLoadAddress(target);
    LOG_INFO("Maybe bad byte: ", CommonUtils::Dec2HexAddress(addr, CommonUtils::hexAddressWidth64BitsCpu));
    if (addr == LLDB_INVALID_ADDRESS || addr == std::numeric_limits<lldb::addr_t>::max()) {
        return false;
    }
    startAddrBackwards = SBAddress(addr + 1, target);
    ++skippedBadBytes;
    return true;
}

void DisassembleApiHandler::CollectBackwardInstructions(const vector<SBInstructionList> &sbInstrLists,
                                                       vector<SBInstruction> &sbInstructions)
{
    SBTarget &target = debugger->GetSbTarget();
    for (auto sbInstrList : sbInstrLists) {
        for (size_t i = 0; i < sbInstrList.GetSize(); i++) {
            SBInstruction sbInstruction = sbInstrList.GetInstructionAtIndex(static_cast<uint32_t>(i));
            lldb::addr_t loadAddr = sbInstruction.GetAddress().GetLoadAddress(target);
            if (loadAddr >= startAddress) {
                break; // already reached forward range
            }
            sbInstructions.emplace_back(sbInstruction);
        }
    }
}

void DisassembleApiHandler::AppendBackwardDapInstructions(const vector<SBInstruction> &sbInstructions,
                                                          uint64_t requestedCount)
{
    uint64_t availableCount = static_cast<uint64_t>(sbInstructions.size());
    uint64_t firstInstruction = availableCount > requestedCount ? availableCount - requestedCount : 0;
    uint64_t resultCount = std::min(instructionCount, availableCount - firstInstruction);
    uint64_t endInstruction = firstInstruction + resultCount;
    for (uint64_t j = firstInstruction; j < endInstruction; j++) {
        DisassembledInstruction dapInstruction = GenDapInstruction(sbInstructions[static_cast<size_t>(j)]);
        dapInstructions.emplace_back(dapInstruction);
    }
}

bool DisassembleApiHandler::GenStartAddrBackwards(SBAddress& startAddr)
{
    if (instructionOffset > 0) {
        return false;
    }
    lldb::addr_t tryAddress;
    uint64_t positiveOffset = static_cast<uint64_t>(-(instructionOffset + 1)) + 1;
    if (positiveOffset > startAddress / INSTRUCTION_BYTES) {
        tryAddress = startAddress;
    } else {
        tryAddress = startAddress - positiveOffset * INSTRUCTION_BYTES;
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
    dapInstruction.address = CommonUtils::Dec2HexAddress(loadAddr, CommonUtils::hexAddressWidth64BitsCpu);
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
