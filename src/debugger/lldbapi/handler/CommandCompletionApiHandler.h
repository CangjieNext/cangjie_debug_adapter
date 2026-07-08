// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_COMMAND_COMPLETION_API_HANDLER_H
#define DAP_SERVER_COMMAND_COMPLETION_API_HANDLER_H

#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class CommandCompletionApiHandler : public RequestApiHandlerBase {
public:
    explicit CommandCompletionApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger)
        : RequestApiHandlerBase(apiDebugger) {};
    ~CommandCompletionApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;

protected:
    enum class CmdType : int {
        NONE = -1,
        DEBUGGER_CMD,
        SUB_CMD,
        OPTION_CMD
    };
    bool GetCommands(const std::vector<std::string> &commandFields);
    bool GetStrParams(const std::string &result, std::string &regexStr, std::string &spaceStr, std::string &flagStr);
    std::string GetTargetCmd(const std::string &cmdKey, const std::string &cmdTail, bool isSingle) const;
    bool Get0Commands();
    bool GetXCommands(const std::string &cmdKey, const std::string &cmdTail);
    void GetCommandResults(std::string &result, const std::vector<std::string> &params, bool isAllowNewObj);
    bool GetCommonCmdResults(const std::smatch &match, const std::string &cmdTail, bool isAllowNewObj);
    bool GetOptionalCmdResults(const std::smatch &match, const std::string &cmdTail, bool isAllowNewObj);

private:
    static std::vector<Protocol::CommandCompletionObject> cmdHelpObjects;
    std::vector<Protocol::CommandCompletionObject> cmdObjects;
    bool isCmdPatch = false;
    bool isSpaceTail = false;
    CmdType cmdType = CmdType::NONE;
    const int cmdNameIndex = 1;
    const int cmdSpecOrOptNameIndex = 2;
    std::string error;
    const std::string debuggerCmdFlag = "Debugger commands:";
    const std::string subCmdFlag = "The following subcommands are supported:";
    const std::string optCmdFlag = "Command Options Usage:";
    const std::string space1Str = std::string(1, ' ');
    const std::string space2Str = std::string(2, ' ');
    const std::string space10Str = std::string(10, ' ');
    const std::string space15Str = std::string(15, ' ');
    const std::string regexCmdStr = R"(\s+([\w-]+)\s+\-\-\s+([^\r\n\f]+))";
    const std::string regexOptCmdStr = R"(\s+(\-\S+)\s*\S*\s+\(\s+(\-\-\S+)\s+\S*\s*\))";
};
}
#endif // DAP_SERVER_COMMAND_COMPLETION_API_HANDLER_H
