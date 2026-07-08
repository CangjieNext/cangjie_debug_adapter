// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_STACKTRACE_APIHANDLER_H
#define DAP_LLDBAPI_STACKTRACE_APIHANDLER_H

#include <climits>
#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class StackTraceApiHandler : public RequestApiHandlerBase {
public:
    explicit StackTraceApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~StackTraceApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    using RequestApiHandlerBase::GenerateSuccessResponse;
    UniqueResponsePtr GenerateSuccessResponse(uint32_t totalFrames);
    void ParseFrameInfo(lldb::SBFrame &sbFrame, int64_t frameId);
    std::string GetFilePath(lldb::SBFrame &sbFrame, const std::string &unknownPath)
    {
        char pBuffer[PATH_MAX];
        uint32_t nBytes = sbFrame.GetLineEntry().GetFileSpec().GetPath(&pBuffer[0], sizeof(pBuffer));
        if (nBytes == 0) {
            return unknownPath;
        }
        std::string filePath = pBuffer;
        if (filePath.length() > nBytes) {
            filePath = filePath.substr(0, nBytes);
        }
        return filePath;
    }

private:
    std::vector<Dap::Protocol::StackFrame> vecStackFrame;
};
};

#endif // DAP_LLDBAPI_STACKTRACE_APIHANDLER_H
