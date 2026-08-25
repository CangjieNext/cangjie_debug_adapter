// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "VariablesApiHandler.h"
#include "ValueParser.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr VariablesApiHandler::HandleRequest(UniqueRequestPtr request)
{
    this->currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<VariablesRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        string error = "ERROR: VariablesRequest Pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<VariablesResponse>(), error, currentRequest->seq);
    }
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE) {
        debugger->GetSbDebugger().GetCategory(debugger->GetLanguageType()).SetEnabled(true);
    }
    std::vector<Variable> vars;
    curVariableRef = reqPtr->arguments.variablesReference;
    ContainerInfo containerInfo = debugger->GetContainerManager().GetContainerInfo(curVariableRef);
    SBFrame liveFrame = debugger->GetLiveFrame(curVariableRef);
    switch (containerInfo.containerType) {
        case DapContainer::LOCALS :
            vars = GetLocalVariables(liveFrame);
            break;
        case DapContainer::STATICS :
            vars = GetStaticVariables(liveFrame);
            break;
        case DapContainer::GLOBALS :
            vars = GetGlobalVariables(liveFrame);
            break;
        case DapContainer::REGISTERS :
            vars = GetRegisterVariables(liveFrame);
            break;
        case DapContainer::VALUE : {
            int64_t count = reqPtr->arguments.count.has_value() ? reqPtr->arguments.count.value() : 0;
            int64_t start = reqPtr->arguments.start.has_value() ? reqPtr->arguments.start.value() : 0;
            vars = GetChildVariables(std::get<SBValue>(containerInfo.container), start, count);
            break;
        }
        case DapContainer::RETURN_VALUE: {
            vars = GetReturnVariables(liveFrame);
            break;
        }
        default :
            string error = "ERROR: container type is not supported.";
            return debugger->CompleteFailureResponse(std::make_unique<VariablesResponse>(), error, currentRequest->seq);
    }

    return GenerateSuccessResponse(vars);
}

std::vector<Variable> VariablesApiHandler::GetLocalVariables(SBFrame &frame)
{
    SBValueList valueList = frame.GetVariables(true, true, false, true);
    if (!valueList.IsValid()) {
        std::vector<Variable> empty;
        return empty;
    } else {
        return GenerateAllVariables(frame, valueList, eValueTypeVariableLocal);
    }
}

std::vector<Variable> VariablesApiHandler::GetStaticVariables(SBFrame &frame)
{
    SBValueList valueList = frame.GetVariables(false, false, true, true);
    if (!valueList.IsValid()) {
        std::vector<Variable> empty;
        return empty;
    } else {
        return GenerateAllVariables(frame, valueList, eValueTypeVariableStatic);
    }
}

std::vector<Variable> VariablesApiHandler::GetGlobalVariables(SBFrame &frame)
{
    SBValueList valueList = frame.GetVariables(false, false, true, true);
    if (!valueList.IsValid()) {
        std::vector<Variable> empty;
        return empty;
    } else {
        return GenerateAllVariables(frame, valueList, eValueTypeVariableGlobal);
    }
}

std::vector<Variable> VariablesApiHandler::GetRegisterVariables(SBFrame &frame)
{
    SBValueList valueList = frame.GetRegisters();
    if (!valueList.IsValid()) {
        std::vector<Variable> empty;
        return empty;
    } else {
        return GenerateAllVariables(frame, valueList, eValueTypeRegisterSet);
    }
}

std::vector<Variable> VariablesApiHandler::GetReturnVariables(lldb::SBFrame &frame)
{
    SBThread thread = frame.GetThread();
    SBValue value = thread.GetStopReturnValue();
    if (!value.IsValid()) {
        return {};
    }
    Variable variable = GenerateVariable(value);
    if ("{...}" == variable.value && value.GetNumChildren() == 0) {
        return {};
    }
    variable.name = "Return Value";
    return std::vector<Variable>{variable};
}

std::vector<Variable> VariablesApiHandler::GenerateAllVariables(SBFrame &frame,
                                                                SBValueList &valueList, ValueType valueTypeFilter)
{
    std::vector<Variable> varVec;
    uint32_t currentLine = frame.GetLineEntry().GetLine();
    for (uint32_t i = 0; i < valueList.GetSize(); i++) {
        SBValue sbValue = valueList.GetValueAtIndex(i);
        if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE &&
            strstr(sbValue.GetName(), "$CapturedVars") != nullptr) {
            continue;
        }
        Variable variable;
        if (debugger->GetOption().filterInvalidLocals && valueTypeFilter == eValueTypeVariableLocal) {
            // interface of sbValue will crush debug on macOS when typeCategory enable
            debugger->UpdateCategory(true);
            if (sbValue.IsValid() && sbValue.GetDeclaration().GetLine() >= currentLine) {
                continue;
            }
        }
        debugger->UpdateCategory();
        if (!sbValue.IsValid()) {
            continue;
        }
        // If one stops inside try block with, which catch clause type is unnamed (e.g std::exception&),
        // then value name will be nullptr as well as value pointer
        if (sbValue.GetName() == nullptr) {
            continue;
        }
        // filter is not necessary for locals (eValueTypeVariableLocal + eValueTypeVariableArgument)
        if (valueTypeFilter != eValueTypeVariableLocal && sbValue.GetValueType() != valueTypeFilter) {
            continue;
        }
        variable = GenerateVariable(sbValue);
        varVec.emplace_back(variable);
    }
    return varVec;
}

std::vector<Variable> VariablesApiHandler::GetChildVariables(SBValue& sbValue, int64_t start, int64_t count)
{
    std::vector<Variable> varVec;
    if (start < 0) {
        return varVec;
    }
    auto numChildren = sbValue.GetNumChildren();
    int64_t actualCount = count == 0 ? numChildren : count;
    int64_t actualEnd = start + actualCount > numChildren ? numChildren : start + actualCount;
    for (int64_t i = start; i < actualEnd; i++) {
        SBValue childValue = sbValue.GetChildAtIndex(i);
        Variable variable;
        if (childValue.IsValid()) {
            variable = GenerateVariable(childValue);
        } else {
            if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE) {
                // If the sbValue is invalid, no display
                continue;
            }
            variable.name = "";
            variable.value = "invalid value";
            variable.isVarCreated = std::nullopt;
            variable.declaredLine = std::nullopt;
            variable.evaluateName = debugger->GetContainerManager().GetExprOfVarRef(curVariableRef);
        }
        varVec.emplace_back(variable);
    }
    return varVec;
}

Variable VariablesApiHandler::GenerateVariable(SBValue &sbValue)
{
    Variable variable;
    if (!sbValue.IsValid()) {
        return variable;
    }
    variable.name = sbValue.GetName() != nullptr ? sbValue.GetName() : "";
    variable.declaredLine = sbValue.GetDeclaration().GetLine();
    SBFileSpec fileSpec = sbValue.GetDeclaration().GetFileSpec();
    if (fileSpec.GetFilename() != nullptr && fileSpec.GetDirectory() != nullptr) {
        string fileName = fileSpec.GetFilename();
        string directory = CommonUtils::ReplaceString(fileSpec.GetDirectory(), "\\", "/");
        Source source;
        source.path = directory + "/" + fileName;
        variable.declaredSource = source;
    }
    if (sbValue.GetValueType() != eValueTypeRegisterSet) {
        // sbValue.GetType() is not valid in eValueTypeRegisterSet situation,
        // call GetDisplayTypeName() will generate a crash.
        variable.type = sbValue.GetDisplayTypeName() != nullptr ? sbValue.GetDisplayTypeName() : "";
    }
    ValueParser valueParser(sbValue, debugger);
    valueParser.GetValue(variable.value);
    string evaluateName = debugger->GetContainerManager().GetExprOfVarRef(curVariableRef);
    lldb::addr_t varAddress = sbValue.GetLoadAddress();
    variable.memoryAddress = CommonUtils::Dec2HexAddress(varAddress, CommonUtils::hexAddressWidth64BitsCpu);
    if (sbValue.GetTypeName() == nullptr || variable.name != sbValue.GetTypeName()) {
        if (!evaluateName.empty() && !variable.name.empty() && variable.name.at(0) != '[') {
            evaluateName = evaluateName + ".";
        }
        evaluateName = evaluateName + variable.name;
        variable.evaluateName = evaluateName;
    }
    if (sbValue.GetNumChildren() > 0) {
        SetVariableChildInfo(sbValue, variable, evaluateName);
    }
    return variable;
}

void VariablesApiHandler::SetVariableChildInfo(SBValue &sbValue, Variable &variable, string &evaluateName)
{
    if (debugger->GetDebuggeeLanguageType() == DebuggeeLanguageType::CANGJIE && sbValue.GetValue() == NULL) {
        // Adapt cangjie option variable internal variables, do not show specific implementation
        // Set child member information only when value and summary are both null
        SBStream sbStream;
        SBTypeSummaryOptions summaryOptions;
        summaryOptions.SetLanguage(debugger->GetLanguageType());
        const char *charSummary = sbValue.GetSummary(sbStream, summaryOptions);
        if (charSummary != nullptr && charSummary[0] != '\0') {
            return;
        }
    }
    variable.variablesReference = debugger->GetContainerManager().GetContainerIndex(curVariableRef, variable.name);
    if (ValueParser::IsArrayType(sbValue, debugger->GetDebuggeeLanguageType())) {
        variable.indexedVariables = sbValue.GetNumChildren();
    } else {
        variable.namedVariables = sbValue.GetNumChildren();
    }
    ContainerInfo containerInfo;
    containerInfo.containerType = DapContainer::VALUE;
    containerInfo.container = sbValue;
    ContainerInfo parentInfo = debugger->GetContainerManager().GetContainerInfo(curVariableRef);
    containerInfo.threadId = parentInfo.threadId;
    containerInfo.frameIndex = parentInfo.frameIndex;
    debugger->GetContainerManager().SetContainerInfo(variable.variablesReference, containerInfo);
    debugger->GetContainerManager().SetExprOfVarRef(variable.variablesReference, evaluateName);
}

UniqueResponsePtr VariablesApiHandler::GenerateSuccessResponse(std::vector<Variable> &variables)
{
    std::unique_ptr<VariablesResponse> responsePtr = std::make_unique<VariablesResponse>();
    debugger->InitResponse(*responsePtr,  currentRequest->seq, true);
    responsePtr->variables = std::move(variables);
    return responsePtr;
}

