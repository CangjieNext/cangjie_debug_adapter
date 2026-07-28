// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "VariableHexViewHandler.h"
#include <cctype>

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

const static std::vector<std::string> INTEGER_TYPES = { "char", "unsigned char", "int", "unsigned int", "long",
                                                        "unsigned long", "long long", "unsigned long long", "int8_t",
                                                        "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
                                                        "int64_t", "uint64_t" };
const static std::string CMD_TYPE_FORMAT_ADD = "type format add -f ";
const static std::string CMD_TYPE_FORMAT_CLEAR = "type format clear";
const static std::string TYPE_FORMAT_HEX_VALUE = "hex";
const static std::string TYPE_FORMAT_HEX_ARRAY = "uint8_t[]";

UniqueResponsePtr VariableHexViewHandler::HandleRequest(Dap::UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<VariableHexViewRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        error = "ERROR: VariableHexViewRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<VariablesResponse>(), error, SEQ_INVALID);
    }
    std::unique_ptr<VariableHexViewResponse> responsePtr = std::make_unique<VariableHexViewResponse>();
    debugger->InitResponse(*responsePtr, currentRequest->seq, true);
    std::string dstFormatType = TYPE_FORMAT_HEX_VALUE;
    if (reqPtr->arguments.isArrayView.has_value() && reqPtr->arguments.isArrayView.value()) {
        dstFormatType = TYPE_FORMAT_HEX_ARRAY;
    }
    if (reqPtr->arguments.isEnable) {
        if (reqPtr->arguments.formats.empty()) {
            responsePtr->success = TypeFormatAdd(dstFormatType, INTEGER_TYPES);
        } else {
            responsePtr->success = TypeFormatAdd(dstFormatType, reqPtr->arguments.formats);
        }
    } else {
        responsePtr->success = TypeFormatClear();
    }
    responsePtr->message = error;
    return responsePtr;
}

bool VariableHexViewHandler::TypeFormatAdd(const std::string &dstType, const std::vector<std::string> &srcTypes)
{
    std::string cmdPrefix = CMD_TYPE_FORMAT_ADD + dstType;
    std::string cmd;
    std::string result;
    bool success = false;
    for (const auto &srcType : srcTypes) {
        if (!IsValidTypeName(srcType)) {
            error = "ERROR: Invalid type name: " + srcType;
            return false;
        }
        cmd = cmdPrefix;
        cmd += " \"";
        cmd += srcType;
        cmd += "\"";
        debugger->ExecCommand(cmd, result, success);
        if (!success) {
            error = "ERROR: Failed to execute command: " + cmd;
            return false;
        }
    }
    return true;
}

bool VariableHexViewHandler::IsValidTypeName(const std::string &typeName) const
{
    if (typeName.empty()) {
        return false;
    }
    for (char c : typeName) {
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && c != ' ' && c != '-' && c != '[' && c != ']' &&
            c != '*' && c != '&') {
            return false;
        }
    }
    return true;
}

bool VariableHexViewHandler::TypeFormatClear()
{
    std::string cmd = CMD_TYPE_FORMAT_CLEAR;
    std::string result;
    bool success = false;
    debugger->ExecCommand(cmd, result, success);
    if (!success) {
        error = "ERROR: Failed to execute command: " + cmd;
        return false;
    }
    return true;
}