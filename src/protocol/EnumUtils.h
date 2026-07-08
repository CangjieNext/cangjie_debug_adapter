// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_ENUMUTILS_H
#define DAP_SERVER_ENUMUTILS_H

#include <map>
#include <string>
#include <optional>
#include "defs.h"
#include "Protocol.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

template <typename ENUM> using EnumStrMap = std::map<ENUM, std::string>;

const EnumStrMap<ProtocolMessage::Type> PROTOCOL_MESSAGE_TYPE_ES = {
    { ProtocolMessage::Type::REQUEST, "request" },
    { ProtocolMessage::Type::RESPONSE, "response" },
    { ProtocolMessage::Type::EVENT, "event" },
    { ProtocolMessage::Type::NONE, "none" }
};

const EnumStrMap<StoppedReason> STOPPED_REASON_ES = {
    { StoppedReason::STEP, "step" },
    { StoppedReason::BREAKPOINT, "breakpoint" },
    { StoppedReason::EXCEPTION, "exception" },
    { StoppedReason::PAUSE, "pause" },
    { StoppedReason::ENTRY, "entry" },
    { StoppedReason::GOTO, "goto" },
    { StoppedReason::FUNCTION_BREAKPOINT, "function breakpoint" },
    { StoppedReason::DATA_BREAKPOINT, "data breakpoint" },
    { StoppedReason::INSTRUCTION_BREAKPOINT, "instruction breakpoint" },
    { StoppedReason::SET_EXECUTIONPOINT, "set executionpoint" },
    { StoppedReason::UNKNOWN, "unknown"}
};

const EnumStrMap<ColumnDescriptorType> COLUMN_DESCRIPTOR_TYPE_ES = { { ColumnDescriptorType::STRING, "string" },
    { ColumnDescriptorType::NUMBER, "number" },
    { ColumnDescriptorType::BOOLEAN, "boolean" },
    { ColumnDescriptorType::UNIX_TIMESTAMP_UTC, "unixTimestampUTC" } };

const EnumStrMap<ChecksumAlgorithmType> CHECKSUM_ALGORITHM_TYPE_ES = { { ChecksumAlgorithmType::MD5, "MD5" },
    { ChecksumAlgorithmType::SHA1, "SHA1" },
    { ChecksumAlgorithmType::SHA256, "SHA256" },
    { ChecksumAlgorithmType::TIMESTAMP, "timestamp" } };

const EnumStrMap<PathFormat> PATH_FORMAT_ES = { { PathFormat::PATH, "path" }, { PathFormat::URL, "url" } };

const EnumStrMap<AttachStep> ATTACH_STEP_ES = { {AttachStep::CONNECT_DBG_SERVER, "connectDbgServer" },
    { AttachStep::ATTACH_TARGET, "attachTarget" },
    { AttachStep::ALL_STEPS, "allSteps" } };

const EnumStrMap<PresentationHint> PRESENTATION_HINT_ES = { { PresentationHint::NORMAL, "normal" },
    { PresentationHint::EMPHASIZE, "emphasize" },
    { PresentationHint::DEEMPHASIZE, "deemphasize" } };

const EnumStrMap<Reason> BREAKPOINT_REASON_ES = { { Reason::CHANGED, "changed" },
    { Reason::NEW, "new" },
    { Reason::REMOVED, "removed" } };

const EnumStrMap<SteppingGranularity> STEPPING_GRANULARITY_ES = {
    { SteppingGranularity::STATEMENT, "statement" },
    { SteppingGranularity::LINE, "line" },
    { SteppingGranularity::INSTRUCTION, "instruction" } };

const EnumStrMap<Filter> FILTER_ES = { { Filter::INDEXED, "indexed" }, { Filter::NAMED, "named" } };

const EnumStrMap<Kind> KIND_ES = { { Kind::PROPERTY, "property" },
    { Kind::METHOD, "method" },
    { Kind::CLASS, "class" },
    { Kind::DATA, "data" },
    { Kind::EVENT, "event" },
    { Kind::BASE_CLASS, "baseClass" },
    { Kind::INNER_CLASS, "innerClass" },
    { Kind::INTERFACE, "interface" },
    { Kind::MOST_DERIVED_CLASS, "mostDerivedClass" },
    { Kind::VIRTUAL, "virtual" },
    { Kind::DATA_BREAKPOINT, "dataBreakpoint" } };

const EnumStrMap<Visibility> VISIBILITY_ES = { { Visibility::PUBLIC, "public" },
    { Visibility::PRIVATE, "private" },
    { Visibility::PROTECTED, "protected" },
    { Visibility::INTERNAL, "internal" },
    { Visibility::FINAL, "final" } };

const EnumStrMap<Attributes> ATTRIBUTES_ES = { { Attributes::STATIC, "static" },
    { Attributes::CONSTANT, "constant" },
    { Attributes::READ_ONLY, "readOnly" },
    { Attributes::RAW_STRING, "rawString" },
    { Attributes::HAS_OBJECT_ID, "hasObjectId" },
    { Attributes::CAN_HAVE_OBJECT_ID, "canHaveObjectId" },
    { Attributes::HAS_SIDE_EFFECTS, "hasSideEffects" } };

const EnumStrMap<OutputEventBodyCategory> OUTPUT_EVENT_BODY_CATEGORY_ES = {
    { OutputEventBodyCategory::CONSOLE, "console" },
    { OutputEventBodyCategory::STDOUT, "stdout" },
    { OutputEventBodyCategory::STDERR, "stderr" },
    { OutputEventBodyCategory::TELEMETRY, "telemetry" }
};

const EnumStrMap<OutputEventBodyGroup> OUTPUT_EVENT_BODY_GROUP_ES = {
    { OutputEventBodyGroup::START, "start" },
    { OutputEventBodyGroup::START_COLLAPSED, "startCollapsed" },
    { OutputEventBodyGroup::END, "end" }
};

const EnumStrMap<StartMethod> START_METHOD_ES = {
    { StartMethod::LAUNCH, "launch" },
    { StartMethod::ATTACH, "attach" },
    { StartMethod::ATTACH_FOR_SUSPENDED_LAUNCH, "attachForSuspendedLaunch" }
};

const EnumStrMap<InvalidatedAreas> INVALIDATED_AREAS_ES = {
    { InvalidatedAreas::ALL, "all" },
    { InvalidatedAreas::STACKS, "stacks" },
    { InvalidatedAreas::THREADS, "threads" },
    { InvalidatedAreas::VARIABLES, "variables" }
};

const EnumStrMap<Context> CONTEXT_ES = {
    { Context::WATCH, "watch" },
    { Context::REPL, "repl" },
    { Context::HOVER, "hover" },
    { Context::CLIPBOARD, "clipboard" }
};

const EnumStrMap<StackFramePresentationHint> STACK_FRAME_PRESENTATION_HINT_ES = {
    { StackFramePresentationHint::NORMAL, "normal" },
    { StackFramePresentationHint::LABEL, "label" },
    { StackFramePresentationHint::SUBTLE, "subtle" }
};

const EnumStrMap<RunInTerminalKind> RUN_IN_TERMINAL_KIND_ES = {
    {RunInTerminalKind::INTEGRATED, "integrated"},
    {RunInTerminalKind::EXTERNAL,  "external"}
};

const EnumStrMap<DataBreakpointAccessType> DATA_BREAKPOINT_ACCESS_TYPE_ES = {
    { DataBreakpointAccessType::READ, "read" },
    { DataBreakpointAccessType::WRITE, "write" },
    { DataBreakpointAccessType::READ_WRITE, "readWrite" }
};

const EnumStrMap<ExceptionBreakMode> EXCEPTION_BREAKPOINT_ACCESS_TYPE_ES = {
    { ExceptionBreakMode::NEVER, "never" },
    { ExceptionBreakMode::ALWAYS, "always" },
    { ExceptionBreakMode::UNHANDLED, "unhandled" },
    { ExceptionBreakMode::USERUNHANDLED, "userUnhandled" }
};

const EnumStrMap<UndoReason> UNDO_REASON_ES = {
    { UndoReason::REVERSE_STEP, "reverseStep" },
    { UndoReason::REVERSE_CONTINUE, "reverseContinue" },
    { UndoReason::CONTINUE_IN_REVERSE, "continueInReverse" },
    { UndoReason::STEP_IN_REVERSE, "stepInReverse" },
    { UndoReason::GOTO_RECORD_POINT, "gotoRecordPoint" },
    { UndoReason::UNKNOWN, "unknown"}
};

template <typename E> const std::optional<std::string> ENUM_TO_STR(const E &e)
{
    (void)e;
    return std::nullopt;
}

template <typename E> const std::optional<E> STR_TO_ENUM(const std::string &s)
{
    (void)s;
    return std::nullopt;
}

template <typename E, typename M> const std::optional<E> TryGetEnum(const M &map, const std::string &s)
{
    for (auto iter : map) {
        if (iter.second == s) {
            return iter.first;
        }
    }
    return std::nullopt;
}

template <> inline const std::optional<std::string> ENUM_TO_STR<ProtocolMessage::Type>(const ProtocolMessage::Type &e)
{
    if (PROTOCOL_MESSAGE_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROTOCOL_MESSAGE_TYPE_ES.at(e);
}

template <> inline const std::optional<ProtocolMessage::Type> STR_TO_ENUM<ProtocolMessage::Type>(const std::string &s)
{
    return TryGetEnum<ProtocolMessage::Type>(PROTOCOL_MESSAGE_TYPE_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<StoppedReason>(const StoppedReason &e)
{
    if (STOPPED_REASON_ES.count(e) == 0) {
        return std::nullopt;
    }
    return STOPPED_REASON_ES.at(e);
}

template <> inline const std::optional<StoppedReason> STR_TO_ENUM<StoppedReason>(const std::string &s)
{
    return TryGetEnum<StoppedReason>(STOPPED_REASON_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<ColumnDescriptorType>(const ColumnDescriptorType &e)
{
    if (COLUMN_DESCRIPTOR_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return COLUMN_DESCRIPTOR_TYPE_ES.at(e);
}

template <> inline const std::optional<ColumnDescriptorType> STR_TO_ENUM<ColumnDescriptorType>(const std::string &s)
{
    return TryGetEnum<ColumnDescriptorType>(COLUMN_DESCRIPTOR_TYPE_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<ChecksumAlgorithmType>(const ChecksumAlgorithmType &e)
{
    if (CHECKSUM_ALGORITHM_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return CHECKSUM_ALGORITHM_TYPE_ES.at(e);
}

template <> inline const std::optional<ChecksumAlgorithmType> STR_TO_ENUM<ChecksumAlgorithmType>(const std::string &s)
{
    return TryGetEnum<ChecksumAlgorithmType>(CHECKSUM_ALGORITHM_TYPE_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<PathFormat>(const PathFormat &e)
{
    if (PATH_FORMAT_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PATH_FORMAT_ES.at(e);
}

template <> inline const std::optional<PathFormat> STR_TO_ENUM<PathFormat>(const std::string &s)
{
    return TryGetEnum<PathFormat>(PATH_FORMAT_ES, s);
}

template <> inline const std::optional<AttachStep> STR_TO_ENUM<AttachStep>(const std::string &s)
{
    return TryGetEnum<AttachStep>(ATTACH_STEP_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<PresentationHint>(const PresentationHint &e)
{
    if (PRESENTATION_HINT_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PRESENTATION_HINT_ES.at(e);
}

template <> inline const std::optional<PresentationHint> STR_TO_ENUM<PresentationHint>(const std::string &s)
{
    return TryGetEnum<PresentationHint>(PRESENTATION_HINT_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Reason>(const Reason &e)
{
    if (BREAKPOINT_REASON_ES.count(e) == 0) {
        return std::nullopt;
    }
    return BREAKPOINT_REASON_ES.at(e);
}

template <> inline const std::optional<Reason> STR_TO_ENUM<Reason>(const std::string &s)
{
    return TryGetEnum<Reason>(BREAKPOINT_REASON_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<SteppingGranularity>(const SteppingGranularity &e)
{
    if (STEPPING_GRANULARITY_ES.count(e) == 0) {
        return std::nullopt;
    }
    return STEPPING_GRANULARITY_ES.at(e);
}

template <> inline const std::optional<SteppingGranularity> STR_TO_ENUM<SteppingGranularity>(const std::string &s)
{
    return TryGetEnum<SteppingGranularity>(STEPPING_GRANULARITY_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Filter>(const Filter &e)
{
    if (FILTER_ES.count(e) == 0) {
        return std::nullopt;
    }
    return FILTER_ES.at(e);
}

template <> inline const std::optional<Filter> STR_TO_ENUM<Filter>(const std::string &s)
{
    return TryGetEnum<Filter>(FILTER_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Kind>(const Kind &e)
{
    if (KIND_ES.count(e) == 0) {
        return std::nullopt;
    }
    return KIND_ES.at(e);
}

template <> inline const std::optional<Kind> STR_TO_ENUM<Kind>(const std::string &s)
{
    return TryGetEnum<Kind>(KIND_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Visibility>(const Visibility &e)
{
    if (VISIBILITY_ES.count(e) == 0) {
        return std::nullopt;
    }
    return VISIBILITY_ES.at(e);
}

template <> inline const std::optional<Visibility> STR_TO_ENUM<Visibility>(const std::string &s)
{
    return TryGetEnum<Visibility>(VISIBILITY_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Attributes>(const Attributes &e)
{
    if (ATTRIBUTES_ES.count(e) == 0) {
        return std::nullopt;
    }
    return ATTRIBUTES_ES.at(e);
}

template <> inline const std::optional<Attributes> STR_TO_ENUM<Attributes>(const std::string &s)
{
    return TryGetEnum<Attributes>(ATTRIBUTES_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<OutputEventBodyCategory>(
    const OutputEventBodyCategory &e)
{
    if (OUTPUT_EVENT_BODY_CATEGORY_ES.count(e) == 0) {
        return std::nullopt;
    }
    return OUTPUT_EVENT_BODY_CATEGORY_ES.at(e);
}

template <> inline const std::optional<OutputEventBodyCategory> STR_TO_ENUM<OutputEventBodyCategory>(
    const std::string &s)
{
    return TryGetEnum<OutputEventBodyCategory>(OUTPUT_EVENT_BODY_CATEGORY_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<OutputEventBodyGroup>(const OutputEventBodyGroup &e)
{
    if (OUTPUT_EVENT_BODY_GROUP_ES.count(e) == 0) {
        return std::nullopt;
    }
    return OUTPUT_EVENT_BODY_GROUP_ES.at(e);
}

template <> inline const std::optional<OutputEventBodyGroup> STR_TO_ENUM<OutputEventBodyGroup>(const std::string &s)
{
    return TryGetEnum<OutputEventBodyGroup>(OUTPUT_EVENT_BODY_GROUP_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<StartMethod>(const StartMethod &e)
{
    if (START_METHOD_ES.count(e) == 0) {
        return std::nullopt;
    }
    return START_METHOD_ES.at(e);
}

template <> inline const std::optional<StartMethod> STR_TO_ENUM<StartMethod>(const std::string &s)
{
    return TryGetEnum<StartMethod>(START_METHOD_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<InvalidatedAreas>(const InvalidatedAreas &e)
{
    if (INVALIDATED_AREAS_ES.count(e) == 0) {
        return std::nullopt;
    }
    return INVALIDATED_AREAS_ES.at(e);
}

template <> inline const std::optional<InvalidatedAreas> STR_TO_ENUM<InvalidatedAreas>(const std::string &s)
{
    return TryGetEnum<InvalidatedAreas>(INVALIDATED_AREAS_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Context>(const Context &e)
{
    if (CONTEXT_ES.count(e) == 0) {
        return std::nullopt;
    }
    return CONTEXT_ES.at(e);
}

template <> inline const std::optional<Context> STR_TO_ENUM<Context>(const std::string &s)
{
    return TryGetEnum<Context>(CONTEXT_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<StackFramePresentationHint>(
    const StackFramePresentationHint &e)
{
    if (STACK_FRAME_PRESENTATION_HINT_ES.count(e) == 0) {
        return std::nullopt;
    }
    return STACK_FRAME_PRESENTATION_HINT_ES.at(e);
}

template <> inline const std::optional<StackFramePresentationHint> STR_TO_ENUM<StackFramePresentationHint>(
    const std::string &s)
{
    return TryGetEnum<StackFramePresentationHint>(STACK_FRAME_PRESENTATION_HINT_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<RunInTerminalKind>(
    const RunInTerminalKind &e)
{
    if (RUN_IN_TERMINAL_KIND_ES.count(e) == 0) {
        return std::nullopt;
    }
    return RUN_IN_TERMINAL_KIND_ES.at(e);
}

template <> inline const std::optional<RunInTerminalKind> STR_TO_ENUM<RunInTerminalKind>(
    const std::string &s)
{
    return TryGetEnum<RunInTerminalKind>(RUN_IN_TERMINAL_KIND_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<DataBreakpointAccessType>(
    const DataBreakpointAccessType &e)
{
    if (DATA_BREAKPOINT_ACCESS_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return DATA_BREAKPOINT_ACCESS_TYPE_ES.at(e);
}

template <> inline const std::optional<DataBreakpointAccessType> STR_TO_ENUM<DataBreakpointAccessType>(
    const std::string &s)
{
    return TryGetEnum<DataBreakpointAccessType>(DATA_BREAKPOINT_ACCESS_TYPE_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<ExceptionBreakMode>(const ExceptionBreakMode &e)
{
    if (EXCEPTION_BREAKPOINT_ACCESS_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return EXCEPTION_BREAKPOINT_ACCESS_TYPE_ES.at(e);
}

template <> inline const std::optional<ExceptionBreakMode> STR_TO_ENUM<ExceptionBreakMode>(const std::string &s)
{
    return TryGetEnum<ExceptionBreakMode>(EXCEPTION_BREAKPOINT_ACCESS_TYPE_ES, s);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<UndoReason>(const UndoReason &e)
{
    if (UNDO_REASON_ES.count(e) == 0) {
        return std::nullopt;
    }
    return UNDO_REASON_ES.at(e);
}

template <> inline const std::optional<UndoReason> STR_TO_ENUM<UndoReason>(const std::string &s)
{
    return TryGetEnum<UndoReason>(UNDO_REASON_ES, s);
}

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_ENUM_UTILS_H
