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

template <typename Optional, typename... Keys>
void SetJsonOptional(json_t &json, const Optional &optional, const Keys &...keys)
{
    if (optional.has_value()) {
        json_t *field = &json;
        ((field = &((*field)[keys])), ...);
        *field = optional.value();
    }
}

template <typename Optional, typename KeyMember, typename ValueMember, typename... Keys>
void SetJsonOptionalMap(json_t &json, const Optional &optional, KeyMember keyMember, ValueMember valueMember,
    const Keys &...keys)
{
    if (optional.has_value()) {
        json_t *field = &json;
        ((field = &((*field)[keys])), ...);
        (*field)[optional.value().*keyMember] = optional.value().*valueMember;
    }
}

template <typename Json, typename Optional, typename Function>
void SetJsonOptionalVectorFunction(Json &json, const Optional &optional, Function function)
{
    if (optional.has_value()) {
        for (const auto &element : optional.value()) {
            json.push_back(function(element).value());
        }
    }
}

template <typename T> std::optional<json_t> GetTypeJson(const T &)
{
    return std::nullopt;
}

template <> std::optional<json_t> GetTypeJson<Message>(const Message &msg);
template <> std::optional<json_t> GetTypeJson<ExceptionBreakpointsFilter>(const ExceptionBreakpointsFilter &filter);
template <> std::optional<json_t> GetTypeJson<ColumnDescriptor>(const ColumnDescriptor &columnDescriptor);
template <> std::optional<json_t> GetTypeJson<Capabilities>(const Capabilities &capabilities);
template <> std::optional<json_t> GetTypeJson<Source>(const Source &source);
template <> std::optional<json_t> GetTypeJson<Module>(const Module &module);
template <> std::optional<json_t> GetTypeJson<DisassembledInstruction>(const DisassembledInstruction &instruction);
template <> std::optional<json_t> GetTypeJson<VariablePresentationHint>(
    const VariablePresentationHint &presentationHint);
template <> std::optional<json_t> GetTypeJson<MonitorVariable>(const MonitorVariable &monitorVariable);
template <> std::optional<json_t> GetTypeJson<StackFrame>(const StackFrame &stackFrame);
template <> std::optional<json_t> GetTypeJson<Variable>(const Variable &variable);

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_TYPE_UTILS_H
