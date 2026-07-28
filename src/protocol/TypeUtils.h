// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_TYPE_UTILS_H
#define DAP_TYPE_UTILS_H

#include <optional>

#include "defs.h"
#include "Protocol.h"
#include "EnumUtils.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

#define SET_JSON_OPTIONAL(JSON, OPTIONAL) \
    if ((OPTIONAL).has_value()) {           \
        JSON = (OPTIONAL).value();          \
    }

#define SET_JSON_OPTIONAL_MAP(JSON, OPTIONAL, K, V)    \
    if ((OPTIONAL).has_value()) {                        \
        (JSON)[(OPTIONAL).value().K] = (OPTIONAL).value().V; \
    }

#define SET_JSON_OPTIONAL_VECTOR_OPTIONAL(JSON, OPTIONAL) \
    if ((OPTIONAL).has_value()) {                           \
        for (auto e : (OPTIONAL).value()) {                 \
            (JSON).push_back(e.value());                    \
        }                                                 \
    }

#define SET_JSON_OPTIONAL_VECTOR_FUNC_OPTIONAL(JSON, OPTIONAL, FUNC) \
    if ((OPTIONAL).has_value()) {                                      \
        for (auto e : (OPTIONAL).value()) {                            \
            (JSON).push_back(FUNC(e).value());                         \
        }                                                            \
    }

template <typename T> std::optional<json_t> GetTypeJson(const T &)
{
    return std::nullopt;
}

template <> inline std::optional<json_t> GetTypeJson<Message>(const Message &msg)
{
    json_t json;
    json["id"] = msg.id;
    json["format"] = msg.format;
    SET_JSON_OPTIONAL_MAP(json["variables"], msg.variables, key, value)
    SET_JSON_OPTIONAL(json["sendTelemetry"], msg.sendTelemetry)
    SET_JSON_OPTIONAL(json["showUser"], msg.showUser)
    SET_JSON_OPTIONAL(json["urlLabel"], msg.urlLabel)
    return json;
}

template <>
inline std::optional<json_t> GetTypeJson<ExceptionBreakpointsFilter>(const ExceptionBreakpointsFilter &filter)
{
    json_t json;
    json["filter"] = filter.filter;
    json["label"] = filter.label;
    SET_JSON_OPTIONAL(json["default"], filter.default_)
    SET_JSON_OPTIONAL(json["supportsCondition"], filter.supportsCondition)
    return json;
}

template <> inline std::optional<json_t> GetTypeJson<ColumnDescriptor>(const ColumnDescriptor &columnDescriptor)
{
    json_t json;
    json["attributeName"] = columnDescriptor.attributeName;
    json["label"] = columnDescriptor.label;
    SET_JSON_OPTIONAL(json["format"], columnDescriptor.format)
    SET_JSON_OPTIONAL(json["type"], columnDescriptor.type)
    SET_JSON_OPTIONAL(json["width"], columnDescriptor.width)
    return json;
}

template <> inline std::optional<json_t> GetTypeJson<Capabilities>(const Capabilities &capabilities)
{
    json_t json;
    json["exceptionBreakpointFilters"] = json_t::array();
    SET_JSON_OPTIONAL_VECTOR_FUNC_OPTIONAL(json["exceptionBreakpointFilters"], capabilities.exceptionBreakpointFilters,
        GetTypeJson<ExceptionBreakpointsFilter>)
    SET_JSON_OPTIONAL(json["completionTriggerCharacters"], capabilities.completionTriggerCharacters)
    json["additionalModuleColumns"] = json_t::array();
    SET_JSON_OPTIONAL_VECTOR_FUNC_OPTIONAL(json["additionalModuleColumns"], capabilities.additionalModuleColumns,
        GetTypeJson<ColumnDescriptor>)
    json["supportedChecksumAlgorithms"] = json_t::array();
    SET_JSON_OPTIONAL_VECTOR_FUNC_OPTIONAL(json["supportedChecksumAlgorithms"],
        capabilities.supportedChecksumAlgorithms, ENUM_TO_STR<ChecksumAlgorithmType>)
    SET_JSON_OPTIONAL(json["supportsConfigurationDoneRequest"], capabilities.supportsConfigurationDoneRequest)
    SET_JSON_OPTIONAL(json["supportsFunctionBreakpoints"], capabilities.supportsFunctionBreakpoints)
    SET_JSON_OPTIONAL(json["supportsConditionalBreakpoints"], capabilities.supportsConditionalBreakpoints)
    SET_JSON_OPTIONAL(json["supportsHitConditionalBreakpoints"], capabilities.supportsHitConditionalBreakpoints)
    SET_JSON_OPTIONAL(json["supportsEvaluateForHovers"], capabilities.supportsEvaluateForHovers)
    SET_JSON_OPTIONAL(json["supportsStepBack"], capabilities.supportsStepBack)
    SET_JSON_OPTIONAL(json["supportsSetVariable"], capabilities.supportsSetVariable)
    SET_JSON_OPTIONAL(json["supportsRestartFrame"], capabilities.supportsRestartFrame)
    SET_JSON_OPTIONAL(json["supportsGotoTargetsRequest"], capabilities.supportsGotoTargetsRequest)
    SET_JSON_OPTIONAL(json["supportsStepInTargetsRequest"], capabilities.supportsStepInTargetsRequest)
    SET_JSON_OPTIONAL(json["supportsCompletionsRequest"], capabilities.supportsCompletionsRequest)
    SET_JSON_OPTIONAL(json["supportsModulesRequest"], capabilities.supportsModulesRequest)
    SET_JSON_OPTIONAL(json["supportsRestartRequest"], capabilities.supportsRestartRequest)
    SET_JSON_OPTIONAL(json["supportsExceptionOptions"], capabilities.supportsExceptionOptions)
    SET_JSON_OPTIONAL(json["supportsValueFormattingOptions"], capabilities.supportsValueFormattingOptions)
    SET_JSON_OPTIONAL(json["supportsExceptionInfoRequest"], capabilities.supportsExceptionInfoRequest)
    SET_JSON_OPTIONAL(json["supportTerminateDebuggee"], capabilities.supportTerminateDebuggee)
    SET_JSON_OPTIONAL(json["supportsDelayedStackTraceLoading"], capabilities.supportsDelayedStackTraceLoading)
    SET_JSON_OPTIONAL(json["supportsLoadedSourcesRequest"], capabilities.supportsLoadedSourcesRequest)
    SET_JSON_OPTIONAL(json["supportsLogPoints"], capabilities.supportsLogPoints)
    SET_JSON_OPTIONAL(json["supportsTerminateThreadsRequest"], capabilities.supportsTerminateThreadsRequest)
    SET_JSON_OPTIONAL(json["supportsSetExpression"], capabilities.supportsSetExpression)
    SET_JSON_OPTIONAL(json["supportsTerminateRequest"], capabilities.supportsTerminateRequest)
    SET_JSON_OPTIONAL(json["supportsDataBreakpoints"], capabilities.supportsDataBreakpoints)
    SET_JSON_OPTIONAL(json["supportsReadMemoryRequest"], capabilities.supportsReadMemoryRequest)
    SET_JSON_OPTIONAL(json["supportsWriteMemoryRequest"], capabilities.supportsWriteMemoryRequest)
    SET_JSON_OPTIONAL(json["supportsDisassembleRequest"], capabilities.supportsDisassembleRequest)
    SET_JSON_OPTIONAL(json["supportsCancelRequest"], capabilities.supportsCancelRequest)
    SET_JSON_OPTIONAL(json["supportsBreakpointLocationsRequest"], capabilities.supportsBreakpointLocationsRequest)
    SET_JSON_OPTIONAL(json["supportsClipboardContext"], capabilities.supportsClipboardContext)
    SET_JSON_OPTIONAL(json["supportsSteppingGranularity"], capabilities.supportsSteppingGranularity)
    SET_JSON_OPTIONAL(json["supportsInstructionBreakpoints"], capabilities.supportsInstructionBreakpoints)
    SET_JSON_OPTIONAL(json["supportsExceptionFilterOptions"], capabilities.supportsExceptionFilterOptions)
    SET_JSON_OPTIONAL(json["supportsTimeTravelDebugging"], capabilities.supportsTimeTravelDebugging)
    SET_JSON_OPTIONAL(json["supportsSetExecutionPoint"], capabilities.supportsSetExecutionPoint)
    json["supportedTimeTravelRecordPointTypes"] = json_t::array();
    if (capabilities.supportedTimeTravelRecordPointTypes.has_value()) {
        for (auto str : capabilities.supportedTimeTravelRecordPointTypes.value()) {
            json["supportedTimeTravelRecordPointTypes"].push_back(str);
        }
    }
    return json;
}

template <> inline std::optional<json_t> GetTypeJson<Source>(const Source &source)
{
    json_t json;
    SET_JSON_OPTIONAL(json["name"], source.name)
    SET_JSON_OPTIONAL(json["path"], source.path)
    SET_JSON_OPTIONAL(json["sourceReference"], source.sourceReference)
    if (source.presentationHint.has_value()) {
        json["presentationHint"] = ENUM_TO_STR(source.presentationHint.value()).value();
    }
    SET_JSON_OPTIONAL(json["origin"], source.origin)
    if (source.sources.has_value()) {
        json_t jsonArray = json_t::array();
        for (Source source : source.sources.value()) {
            jsonArray.push_back(GetTypeJson(source).value());
        }
        json["sources"] = jsonArray;
    }
    if (source.adapterData.has_value()) {
        /* *
         * Optional data that a debug adapter might want to loop through the client.
         * The client should leave the data intact and persist it across sessions. The
         * client should not interpret the data.
         */
        json["adapterData"] = std::any_cast<json_t>(source.adapterData.value());
    }
    if (source.checksums.has_value()) {
        json_t checksumsJson = json_t::array();
        for (Checksum checksum : source.checksums.value()) {
            json_t checksumJson = json_t::object();
            checksumJson["algorithm"] = ENUM_TO_STR(checksum.algorithm).value();
            checksumJson["checksum"] = checksum.checksum;
            checksumsJson.push_back(checksumJson);
        }
        json["checksums"] = checksumsJson;
    }
    return json;
}

template <> inline std::optional<json_t> GetTypeJson<Module>(const Module &module)
{
    json_t json;
    json["id"] = module.id;
    json["name"] = module.name;
    json["uuid"] = module.uuid;
    SET_JSON_OPTIONAL(json["path"], module.path)
    SET_JSON_OPTIONAL(json["isOptimized"], module.isOptimized)
    SET_JSON_OPTIONAL(json["isUserCode"], module.isUserCode)
    SET_JSON_OPTIONAL(json["version"], module.version)
    SET_JSON_OPTIONAL(json["symbolStatus"], module.symbolStatus)
    SET_JSON_OPTIONAL(json["symbolFilePath"], module.symbolFilePath)
    SET_JSON_OPTIONAL(json["dateTimeStamp"], module.dateTimeStamp)
    SET_JSON_OPTIONAL(json["addressRange"], module.addressRange)
    SET_JSON_OPTIONAL(json["codeRange"], module.codeRange)
    SET_JSON_OPTIONAL(json["loadedAddress"], module.loadedAddress)
    SET_JSON_OPTIONAL(json["platformFilePath"], module.platformFilePath)
    return json;
}

template <>
inline std::optional<json_t> GetTypeJson<DisassembledInstruction>(const DisassembledInstruction &instruction)
{
    json_t instructionJson;
    instructionJson["address"] = instruction.address;
    SET_JSON_OPTIONAL(instructionJson["instructionBytes"], instruction.instructionBytes)
    instructionJson["instruction"] = instruction.instruction;
    SET_JSON_OPTIONAL(instructionJson["symbol"], instruction.symbol)
    SET_JSON_OPTIONAL(instructionJson["symbol"], instruction.symbol)
    if (instruction.location.has_value()) {
        instructionJson["location"] = GetTypeJson(instruction.location.value()).value();
    }
    SET_JSON_OPTIONAL(instructionJson["line"], instruction.line)
    SET_JSON_OPTIONAL(instructionJson["column"], instruction.column)
    SET_JSON_OPTIONAL(instructionJson["endLine"], instruction.endLine)
    SET_JSON_OPTIONAL(instructionJson["endColumn"], instruction.endColumn)
    return instructionJson;
}

template <>
inline std::optional<json_t> GetTypeJson<VariablePresentationHint>(const VariablePresentationHint &presentationHint)
{
    json_t res;
    if (presentationHint.kind.has_value()) {
        SET_JSON_OPTIONAL(res["kind"], ENUM_TO_STR<Kind>(presentationHint.kind.value()))
    }
    if (presentationHint.attributes.has_value()) {
        for (unsigned i = 0; i < presentationHint.attributes.value().size(); i++) {
            SET_JSON_OPTIONAL(res["attributes"][i], ENUM_TO_STR<Attributes>(presentationHint.attributes.value().at(i)))
        }
    }
    if (presentationHint.visibility.has_value()) {
        SET_JSON_OPTIONAL(res["visibility"], ENUM_TO_STR<Visibility>(presentationHint.visibility.value()))
    }
    return res;
}

template <>
inline std::optional<json_t> GetTypeJson<MonitorVariable>(const MonitorVariable &monitorVariable)
{
    json_t res;
    res["id"] = monitorVariable.id;
    SET_JSON_OPTIONAL(res["name"], monitorVariable.name)
    SET_JSON_OPTIONAL(res["scopeName"], monitorVariable.scopeName)
    SET_JSON_OPTIONAL(res["type"], monitorVariable.type)
    SET_JSON_OPTIONAL(res["value"], monitorVariable.value)
    return res;
}

template <>
inline std::optional<json_t> GetTypeJson<StackFrame>(const StackFrame &stackFrame)
{
    json_t stackFrameJson = json_t::object();
    stackFrameJson["id"] = stackFrame.id;
    stackFrameJson["name"] = stackFrame.name;
    stackFrameJson["line"] = stackFrame.line;
    stackFrameJson["column"] = stackFrame.column;
    if (stackFrame.source.has_value()) {
        stackFrameJson["source"] = GetTypeJson(stackFrame.source.value()).value();
    }
    SET_JSON_OPTIONAL(stackFrameJson["endLine"], stackFrame.endLine)
    SET_JSON_OPTIONAL(stackFrameJson["endColumn"], stackFrame.endColumn)
    SET_JSON_OPTIONAL(stackFrameJson["instructionPointerReference"], stackFrame.instructionPointerReference)
    SET_JSON_OPTIONAL(stackFrameJson["symbolInstructionRange"], stackFrame.symbolInstructionRange)
    SET_JSON_OPTIONAL(stackFrameJson["moduleId"], stackFrame.moduleId)
    SET_JSON_OPTIONAL(stackFrameJson["moduleName"], stackFrame.moduleName)
    if (stackFrame.presentationHint.has_value()) {
        stackFrameJson["presentationHint"] = ENUM_TO_STR(stackFrame.presentationHint.value()).value();
    }
    return stackFrameJson;
}

template <>
inline std::optional<json_t> GetTypeJson<Variable>(const Variable &variable)
{
    json_t variableJson = json_t::object();
    variableJson["name"] = variable.name;
    variableJson["value"] = variable.value;
    if (CommonUtils::JsonDump(variableJson["value"]).empty()) {
        LOG_ERROR("can not get variables value,variable name is: ", variable.name);
        variableJson["value"] = "??";
    }
    variableJson["variablesReference"] = variable.variablesReference;
    if (variable.type.has_value()) {
        variableJson["type"] = variable.type.value();
    }
    if (variable.evaluateName.has_value()) {
        variableJson["evaluateName"] = variable.evaluateName.value();
    }
    if (variable.namedVariables.has_value()) {
        variableJson["namedVariables"] = variable.namedVariables.value();
    }
    if (variable.indexedVariables.has_value()) {
        variableJson["indexedVariables"] = variable.indexedVariables.value();
    }
    if (variable.declaredLine.has_value()) {
        variableJson["declaredLine"] = variable.declaredLine.value();
    }
    if (variable.declaredSource.has_value()) {
        variableJson["declaredSource"] = GetTypeJson(variable.declaredSource.value()).value();
    }
    if (variable.isVarCreated.has_value()) {
        variableJson["isVarCreated"] = variable.isVarCreated.value();
    }
    if (variable.memoryReference.has_value()) {
        variableJson["memoryReference"] = variable.memoryReference.value();
    }
    if (variable.memoryAddress.has_value()) {
        variableJson["memoryAddress"] = variable.memoryAddress.value();
    }
    return variableJson;
}

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_TYPE_UTILS_H