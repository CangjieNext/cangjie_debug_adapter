// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_MULTIMSGHANDLER_H
#define DAP_SERVER_MULTIMSGHANDLER_H
#include <debugger/Debugger.h>

class MultiMsgHandler : public Dap::DebuggerMessageHandler {
public:
    void OnDebuggerResponse(const Dap::Protocol::Response &response) override;
    void OnDebuggerEvent(const Dap::Protocol::Event &event) override
    {
        (void)event;
    }
    void OnDebuggerError(const int &errorCode, const std::string &msg) const override
    {
        (void)errorCode;
        (void)msg;
    }
    void OnDebuggerExit(const int &exitCode) const override
    {
        (void)exitCode;
    }
    json_t GetJson();
private:
    json_t json;
};
#endif // DAP_SERVER_MULTIMSGHANDLER_H
