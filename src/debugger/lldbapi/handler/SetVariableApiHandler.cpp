// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SetVariableApiHandler.h"
#include "DapLog.h"
#include "CommonUtils.h"
#include <cctype>

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr SetVariableApiHandler::HandleRequest(UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<SetVariableRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: SetVariableRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<SetVariableResponse>(), error, currentRequest->seq);
    }

    int64_t containerRef = reqPtr->arguments.variablesReference;
    ContainerInfo containerInfo = debugger->GetContainerManager().GetContainerInfo(containerRef);
    SBValue value = debugger->GetContainerManager().FindVarSBValue(reqPtr->arguments.name, containerRef,
        debugger->GetSbDebugger());
    if (!value.IsValid()) {
        string error = "Could not set variable value, variable name: " + reqPtr->arguments.name;
        return debugger->CompleteFailureResponse(std::make_unique<SetVariableResponse>(), error, currentRequest->seq);
    }

    string expression = reqPtr->arguments.value;
    string exprValue = expression;
    BasicType basicType = value.GetType().GetBasicType();
    if (IsSimpleLiteral(expression, basicType)) {
        ValueParser valueParser = GetExprValueParser(value, expression);
        if (valueParser.GetValue(exprValue) && exprValue != "unknown") {
            SBError sbError;
            bool ret = value.SetValueFromCString(exprValue.c_str(), sbError);
            if (ret) {
                return GenerateSuccessResponse(containerRef, value);
            }
        }
    }
    SBError sbError;
    bool ret = value.SetValueFromCString(exprValue.c_str(), sbError);
    if (ret) {
        return GenerateSuccessResponse(containerRef, value);
    } else {
        string error = sbError.GetCString() == nullptr ? "Failed to set variable." : sbError.GetCString();
        return debugger->CompleteFailureResponse(std::make_unique<SetVariableResponse>(), error, currentRequest->seq);
    }
}

ValueParser SetVariableApiHandler::GetExprValueParser(SBValue &value, string &expression) const
{
    expression = CommonUtils::TrimString(expression, " ", TrimType::BOTH);
    string varType = value.GetType().GetName();
    if (varType == "bool") {
        if (expression == "true") {
            expression = "1";
        }
        if (expression == "false") {
            expression = "0";
        }
    }
    BasicType basicType = value.GetType().GetBasicType();
    if (basicType == lldb::eBasicTypeUnsignedChar || basicType == lldb::eBasicTypeSignedChar) {
        expression = FormatCharValueExpr(expression, basicType == lldb::eBasicTypeUnsignedChar);
    }
    SBFrame frame = value.GetFrame();
    debugger->UpdateCategory(true);
    SBValue sbValue = debugger->FindExprValue(frame, expression);
    debugger->UpdateCategory();
    ValueParser valueParser(sbValue, debugger);
    return valueParser;
}

string SetVariableApiHandler::FormatCharValueExpr(const string &expression, bool isUnsigned) const
{
    string result = expression;
    constexpr int quotesPairLen = 2; // indicate "''"
    if (result.length() > quotesPairLen && result.at(0) == '\'' && result.back() == '\'') {
        result = result.substr(1, result.length() - quotesPairLen);
        signed char res;
        // single character, such as 'a'.
        if (result.length() == 1 && result.at(0) != '\\') {
            return to_string(static_cast<int>(result.at(0)));
        // escape character, such as '\n'.
        } else if (CommonUtils::ReverseEscapeChar(result, res)) {
            return to_string(static_cast<int>(res));
        // escape hexadecimal('\xhh') character or escape octal('\ddd') characters, such as '\xA2' or '\141'.
        } else if (CommonUtils::HexToChar(result, res) || CommonUtils::OctToChar(result, res)) {
            int charValue = static_cast<int>(res);
            if (charValue < 0 && isUnsigned) {
                charValue += CHAR_RANGE;
            }
            return std::to_string(charValue);
        }
    }
    return result;
}

bool SetVariableApiHandler::IsSimpleLiteral(const string &expression, BasicType basicType) const
{
    if (expression.empty()) {
        return false;
    }
    if (expression == "true" || expression == "false") {
        return true;
    }
    if (basicType == lldb::eBasicTypeUnsignedChar || basicType == lldb::eBasicTypeSignedChar) {
        constexpr int quotesPairLen = 2;
        if (expression.length() > quotesPairLen && expression.front() == '\'' && expression.back() == '\'') {
            return true;
        }
    }
    constexpr int minusSignLen = 1;
    string trimmed = CommonUtils::TrimString(expression, " ", TrimType::BOTH);
    if (trimmed.empty()) {
        return false;
    }
    size_t start = 0;
    if (trimmed[0] == '-') {
        start = minusSignLen;
    }
    if (start >= trimmed.length()) {
        return false;
    }
    for (size_t i = start; i < trimmed.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(trimmed[i]))) {
            return false;
        }
    }
    return true;
}

UniqueResponsePtr SetVariableApiHandler::GenerateSuccessResponse(int64_t parentIndex, SBValue &sbValue)
{
    std::unique_ptr<SetVariableResponse> responsePtr = std::make_unique<SetVariableResponse>();
    debugger->InitResponse(*responsePtr,  currentRequest->seq, true);

    ValueParser valueParser(sbValue, debugger);
    valueParser.GetValue(responsePtr->value);
    responsePtr->variableType = sbValue.GetType().GetName();
    unsigned numChild = sbValue.GetNumChildren();
    if (numChild > 0) {
        string name = sbValue.GetName() == nullptr ? "" : sbValue.GetName();
        int64_t containerIdx = debugger->GetContainerManager().GetContainerIndex(parentIndex, name);
        responsePtr->variablesReference = containerIdx;
        if (ValueParser::IsArrayType(sbValue, debugger->GetDebuggeeLanguageType())) {
            responsePtr->indexedVariables = numChild;
        } else {
            responsePtr->namedVariables = numChild;
        }
        // need to update the new value into ContainerManager
        ContainerInfo containerInfo;
        containerInfo.containerType = DapContainer::VALUE;
        containerInfo.container = sbValue;
        debugger->GetContainerManager().SetContainerInfo(containerIdx, containerInfo);
    }
    return responsePtr;
}
