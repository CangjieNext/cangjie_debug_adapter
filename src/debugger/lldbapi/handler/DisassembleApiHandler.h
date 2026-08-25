// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_DISASSEMBLE_APIHANDLER_H
#define DAP_LLDBAPI_DISASSEMBLE_APIHANDLER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class DisassembleApiHandler : public RequestApiHandlerBase {
public:
    explicit DisassembleApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~DisassembleApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

private:
    void ReadInstructions();
    void ReadForwardInstructions();
    bool CollectForwardInstructions(lldb::SBTarget &target, const lldb::SBInstructionList &sbInstructionList,
                                    std::vector<lldb::SBInstruction> &sbInstructions, lldb::addr_t currentAddress,
                                    lldb::addr_t &nextInstructionAddress);
    bool HandleForwardBadByte(lldb::SBTarget &target, lldb::SBAddress &baseSBAddress,
                              uint64_t &skippedBadBytes);
    void AppendDapInstructions(const std::vector<lldb::SBInstruction> &sbInstructions, uint64_t firstInstruction,
                               uint64_t availableCount);
    void ReadInstructionsBackwards();
    bool ProcessBackwardInstruction(lldb::SBTarget &target, const lldb::SBInstructionList &instrList,
                                    uint32_t readCount, std::vector<lldb::SBInstructionList> &sbInstrLists,
                                    lldb::SBAddress &startAddrBackwards);
    bool HandleBackwardBadByte(lldb::SBTarget &target, lldb::SBAddress &startAddrBackwards,
                               uint64_t &skippedBadBytes);
    void CollectBackwardInstructions(const std::vector<lldb::SBInstructionList> &sbInstrLists,
                                     std::vector<lldb::SBInstruction> &sbInstructions);
    void AppendBackwardDapInstructions(const std::vector<lldb::SBInstruction> &sbInstructions,
                                       uint64_t requestedCount);
    bool IsInstructionValid(const lldb::SBInstruction &sbInstruction, lldb::addr_t currentAddress,
                            lldb::addr_t nextInstructionAddress) const;
    bool GenStartAddrBackwards(lldb::SBAddress &startAddr);
    Dap::Protocol::DisassembledInstruction GenDapInstruction(lldb::SBInstruction &sbInstruction);
    void AddDapInstrSourceInfo(lldb::SBInstruction &sbInstruction,
                               Dap::Protocol::DisassembledInstruction &dapInstruction);

    std::vector<Dap::Protocol::DisassembledInstruction> dapInstructions;
    std::string preInstrSourcePath;
    std::string preInstrSymbol;
    lldb::addr_t startAddress = 0;
    int64_t instructionOffset = 0;
    int64_t forwardCount = 0;
    uint64_t instructionCount = 0;
    int64_t proLine = 0; // cache line number of last instruction code to adapt line number follow
};
};

#endif // DAP_LLDBAPI_DISASSEMBLE_APIHANDLER_H
