// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CommandCompletionApiHandler.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr CommandCompletionApiHandler::HandleRequest(UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto req = dynamic_cast<CommandCompletionRequest*>(currentRequest.get());
    if (req == nullptr) {
        error = "ERROR: Invalid parameter, CommandCompletionRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<CommandCompletionResponse>(), error,
                                                 Protocol::SEQ_INVALID);
    }
    std::unique_ptr<CommandCompletionResponse> responsePtr= std::make_unique<CommandCompletionResponse>();
    debugger->InitResponse(*responsePtr, req->seq, true);
    std::string userCommand = req->arguments.command;
    isSpaceTail = !userCommand.empty() && CommonUtils::EndWithString(userCommand, space1Str);
    userCommand = CommonUtils::TrimString(userCommand, space1Str);
    userCommand = CommonUtils::ReplaceStringEx(userCommand, space2Str, space1Str);
    std::vector<std::string> commandFields = CommonUtils::SplitString(userCommand, space1Str);
    if (!GetCommands(commandFields)) {
        responsePtr->success = false;
        responsePtr->message = error;
    } else {
        responsePtr->objects = cmdObjects;
        responsePtr->isPatch = isCmdPatch;
    }
    return responsePtr;
}

std::string GetCommandKey(const std::vector<std::string> &commands)
{
    std::string key;
    const size_t size = commands.size() - 1;
    for (size_t idx = 0; idx < size; idx++) {
        if (idx != size - 1) {
            key += commands.at(idx) + " ";
        } else {
            key += commands.at(idx);
        }
    }
    return key;
}

bool CommandCompletionApiHandler::GetCommands(const std::vector<std::string> &commandFields)
{
    if (commandFields.empty()) {
        return Get0Commands();
    }
    if (commandFields.size() == 1) {
        return GetXCommands("", commandFields.at(0));
    } else {
        std::string cmdKey = GetCommandKey(commandFields);
        std::string cmdTail = *commandFields.rbegin();
        return GetXCommands(cmdKey, cmdTail);
    }
}

bool CommandCompletionApiHandler::GetCommonCmdResults(const std::smatch &match, const std::string &cmdTail,
                                                      bool isAllowNewObj)
{
    std::string command = match[cmdNameIndex].str();
    if (!isAllowNewObj && !CommonUtils::StartWithString(command, cmdTail)) {
        return false;
    }
    CommandCompletionObject obj;
    obj.command = command;
    obj.spec = match[cmdSpecOrOptNameIndex].str();
    cmdObjects.emplace_back(obj);
    return true;
}

bool CommandCompletionApiHandler::GetOptionalCmdResults(const std::smatch &match, const std::string &cmdTail,
                                                        bool isAllowNewObj)
{
    std::string command;
    if (cmdTail == "-") {
        command = match[cmdNameIndex].str();
    } else if (CommonUtils::StartWithString(cmdTail, "--")) {
        if (!isSpaceTail) {
            command = match[cmdSpecOrOptNameIndex].str();
        } else {
            command = match[cmdNameIndex].str();
        }
    } else {
        if (!isAllowNewObj) {
            return false;
        }
        command = match[cmdNameIndex].str();
    }
    CommandCompletionObject obj;
    obj.command = command;
    cmdObjects.emplace_back(obj);
    return true;
}

void CommandCompletionApiHandler::GetCommandResults(std::string &result,
                                                    const std::vector<std::string> &params,
                                                    bool isAllowNewObj)
{
    cmdObjects.clear();
    const std::string &cmdTail = params.at(1);
    const std::string &regexStr = params.at(2);  // only match 2 results
    const std::string &spaceStr = params.at(3);
    const std::string &flagStr = params.at(4);
    result = CommonUtils::ReplaceStringEx(result, "\r", "\n");
    result = CommonUtils::ReplaceStringEx(result, "\n\n", "\n");
    std::vector<std::string> lines = CommonUtils::SplitString(result, "\n");
    std::optional<std::smatch> optMatchRet;
    int cmdIndex = -1;
    bool cmdMatchStart = false;
    bool isNotInSameCmd = false;
    bool isInsert;
    for (const auto &line : lines) {
        if (!cmdMatchStart) {
            cmdMatchStart = CommonUtils::StartWithString(line, flagStr);
            continue;
        }
        optMatchRet = CommonUtils::RegexMatch(line, regexStr);
        if (!optMatchRet.has_value()) {
            if (!isNotInSameCmd && ((int64_t)cmdObjects.size() > cmdIndex) &&
                CommonUtils::StartWithString(line, spaceStr)) {
                cmdObjects.at(cmdIndex).spec.append(CommonUtils::TrimString(line, " "));
            }
            continue;
        }
        isNotInSameCmd = true;
        if (cmdType == CmdType::OPTION_CMD) {
            isInsert = GetOptionalCmdResults(optMatchRet.value(), cmdTail, isAllowNewObj);
        } else {
            isInsert = GetCommonCmdResults(optMatchRet.value(), cmdTail, isAllowNewObj);
        }
        if (isInsert) {
            cmdIndex++;
            isNotInSameCmd = false;
        }
    }
}

bool CommandCompletionApiHandler::GetStrParams(const std::string &result, std::string &regexStr, std::string &spaceStr,
                                               std::string &flagStr)
{
    if (result.find(debuggerCmdFlag) != std::string::npos) {
        regexStr = regexCmdStr;
        spaceStr = space15Str;
        flagStr = debuggerCmdFlag;
        cmdType = CmdType::DEBUGGER_CMD;
    } else if (result.find(subCmdFlag) != std::string::npos) {
        regexStr = regexCmdStr;
        spaceStr = space15Str;
        flagStr = subCmdFlag;
        cmdType = CmdType::SUB_CMD;
    } else if (result.find(optCmdFlag) != std::string::npos) {
        regexStr = regexOptCmdStr;
        spaceStr = space10Str;
        flagStr = optCmdFlag;
        cmdType = CmdType::OPTION_CMD;
    } else {
        cmdType = CmdType::NONE;
        return false;
    }
    return true;
}

bool IsReverseHelp(const std::string &cmdKey, const std::string &cmdTail, bool isSingle)
{
    const static std::vector<std::string> SPECIAL_CMD_HEADS = { "image" };
    // commands like this: image help; help image list...
    bool result = false;
    for (auto &s : SPECIAL_CMD_HEADS) {
        if ((isSingle && CommonUtils::StartWithString(cmdTail, s))
            || (!isSingle && CommonUtils::StartWithString(cmdKey, s))) {
            result = true;
            break;
        }
    }
    return result;
}

std::string GetCmdKey(const std::string &cmdKey, const std::string &cmdTail, bool &isSuspectOptCmd)
{
    // cmdKey is not empty
    std::string key = cmdKey;
    if (CommonUtils::StartWithString(cmdTail, "-") || CommonUtils::StartWithString(cmdTail, "--")) {
        size_t pos = key.find(" -");
        if (pos != std::string::npos) {
            key = key.substr(0, pos);
        }
        isSuspectOptCmd = true;
    }
    LOG_INFO("cmdKey = ", cmdKey, "; cmdTail = ", cmdTail, "; key = ", key);
    return key;
}

std::string CommandCompletionApiHandler::GetTargetCmd(const std::string &cmdKey, const std::string &cmdTail,
                                                      bool isSingle) const
{
    bool isReverseHelp = IsReverseHelp(cmdKey, cmdTail, isSingle);
    std::string targetCmd;
    if (cmdKey.empty()) {
        if (isReverseHelp) {
            targetCmd = isSpaceTail ? cmdTail + " help" : "help";
        } else {
            targetCmd = isSpaceTail ? "help " + cmdTail : "help";
        }
        return targetCmd;
    }
    bool isSuspectOptCmd = false;
    std::string cmdKeyTmp = GetCmdKey(cmdKey, cmdTail, isSuspectOptCmd);
    if (isReverseHelp) {
        targetCmd = (isSpaceTail && !isSuspectOptCmd)
                ? "help " + cmdKeyTmp + " " + cmdTail + " help" : "help " + cmdKeyTmp + " help";
    } else {
        targetCmd = (isSpaceTail && !isSuspectOptCmd) ? "help " + cmdKeyTmp + " " + cmdTail : "help " + cmdKeyTmp;
    }
    return targetCmd;
}

bool CommandCompletionApiHandler::Get0Commands()
{
    const std::string targetCmd = "help";
    bool isFullCmd = true;
    std::string result;
    bool success = false;
    debugger->ExecCommand(targetCmd, result, success, false);
    if (!success) {
        error = "ERROR: Failed to execute command " + targetCmd;
        return false;
    }
    isCmdPatch = false;
    std::vector<std::string> params = { "", "", regexCmdStr, space15Str, debuggerCmdFlag};
    GetCommandResults(result, params, isFullCmd);
    return true;
}

bool CommandCompletionApiHandler::GetXCommands(const std::string &cmdKey, const std::string &cmdTail)
{
    std::string targetCmd = GetTargetCmd(cmdKey, cmdTail, cmdKey.empty());
    LOG_INFO("targetCmd = ", targetCmd);
    std::string result;
    bool success = false;
    debugger->ExecCommand(targetCmd, result, success, false);
    if (!success) {
        error = "ERROR: Failed to execute command " + targetCmd;
        return false;
    }
    std::string regexStr;
    std::string spaceStr;
    std::string flagStr;
    if (!GetStrParams(result, regexStr, spaceStr, flagStr)) {
        error = "ERROR: Failed to get string params.";
        return false;
    }
    std::vector<std::string> params = { cmdKey, cmdTail, regexStr, spaceStr, flagStr};
    bool isFullCmd = isSpaceTail;
    GetCommandResults(result, params, isFullCmd);
    if (!isSpaceTail) {
        isFullCmd = (cmdObjects.size() == 1) && (cmdObjects.at(0).command == cmdTail);
    }
    isCmdPatch = !isFullCmd;
    return true;
}