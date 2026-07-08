// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLENUMS_H
#define DAP_PROTOCOLENUMS_H

#include "defs.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

enum class StoppedReason {
    STEP,
    BREAKPOINT,
    EXCEPTION,
    PAUSE,
    ENTRY,
    GOTO,
    FUNCTION_BREAKPOINT,
    DATA_BREAKPOINT,
    INSTRUCTION_BREAKPOINT,
    SET_EXECUTIONPOINT,
    UNKNOWN
};

enum class ColumnDescriptorType {
    STRING,
    NUMBER,
    BOOLEAN,
    UNIX_TIMESTAMP_UTC
};

enum class ChecksumAlgorithmType {
    MD5,
    SHA1,
    SHA256,
    TIMESTAMP
};

enum class PathFormat {
    PATH,
    URL
};

enum class PresentationHint {
    NORMAL,
    EMPHASIZE,
    DEEMPHASIZE
};

enum class Reason {
    CHANGED,
    NEW,
    REMOVED
};

enum class SteppingGranularity {
    STATEMENT,
    LINE,
    INSTRUCTION
};

enum class Filter {
    INDEXED,
    NAMED
};

enum class Kind {
    PROPERTY,
    METHOD,
    CLASS,
    DATA,
    EVENT,
    BASE_CLASS,
    INNER_CLASS,
    INTERFACE,
    MOST_DERIVED_CLASS,
    VIRTUAL,
    DATA_BREAKPOINT
};

enum class Visibility {
    PUBLIC,
    PRIVATE,
    PROTECTED,
    INTERNAL,
    FINAL
};

enum class Attributes {
    STATIC,
    CONSTANT,
    READ_ONLY,
    RAW_STRING,
    HAS_OBJECT_ID,
    CAN_HAVE_OBJECT_ID,
    HAS_SIDE_EFFECTS
};

enum class OutputEventBodyCategory {
    CONSOLE,
    STDOUT,
    STDERR,
    TELEMETRY
};

enum class OutputEventBodyGroup {
    START,
    START_COLLAPSED,
    END
};

enum class StartMethod {
    LAUNCH,
    ATTACH,
    ATTACH_FOR_SUSPENDED_LAUNCH
};

enum class InvalidatedAreas {
    ALL,
    STACKS,
    THREADS,
    VARIABLES
};

enum class Context {
    WATCH,
    REPL,
    HOVER,
    CLIPBOARD
};

enum class StackFramePresentationHint {
    NORMAL,
    LABEL,
    SUBTLE
};

enum class RunInTerminalKind {
    INTEGRATED,
    EXTERNAL
};

enum class DataBreakpointAccessType {
    READ,
    WRITE,
    READ_WRITE
};

enum class ExceptionBreakMode {
    NEVER,
    ALWAYS,
    UNHANDLED,
    USERUNHANDLED
};

// huawei
enum class AttachStep {
    CONNECT_DBG_SERVER,
    ATTACH_TARGET,
    ALL_STEPS
};

enum class UndoReason {
    REVERSE_STEP,
    REVERSE_CONTINUE,
    CONTINUE_IN_REVERSE,
    STEP_IN_REVERSE,
    GOTO_RECORD_POINT,
    UNKNOWN
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLENUMS_H
