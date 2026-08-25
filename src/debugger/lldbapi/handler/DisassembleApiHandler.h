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
    void ReadInstructionsBackwards();
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
