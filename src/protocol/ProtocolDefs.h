// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOL_DEFS_H
#define DAP_PROTOCOL_DEFS_H

#include <string>
#include "defs.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

const unsigned int SEQ_MAX = 1000000;
const unsigned int SEQ_INVALID = 0xFFFFFFFF;

const std::string REQUEST_NAME = "request";
const std::string RESPONSE_NAME = "response";
const std::string EVENT_NAME = "event";

const std::string EVENT_INITIALIZED = "initialized";
const std::string EVENT_STOPPED = "stopped";
const std::string EVENT_CONTINUED = "continued";
const std::string EVENT_EXITED = "exited";
const std::string EVENT_TERMINATED = "terminated";
const std::string EVENT_CAPABILITIES = "capabilities";
const std::string EVENT_OUTPUT = "output";
const std::string EVENT_BREAKPOINT = "breakpoint";
const std::string EVENT_THREAD = "thread";
const std::string EVENT_MODULE = "module";
const std::string EVENT_MODULES = "modules";
const std::string EVENT_LOADED_SOURCE = "loadedSource";
const std::string EVENT_PROCESS = "process";
const std::string EVENT_PROGRESS_START = "progressStart";
const std::string EVENT_PROGRESS_UPDATE = "progressUpdate";
const std::string EVENT_PROGRESS_END = "progressEnd";
const std::string EVENT_INVALIDATED = "invalidated";
const std::string EVENT_UNDO = "undo";
const std::string EVENT_RECORD_UPDATE_POINTS = "recordPointsUpdate";

const std::string REQ_RES_INITIALIZE = "initialize";
const std::string REQ_RES_CANCEL = "cancel";
const std::string REQ_RES_LAUNCH = "launch";
const std::string REQ_RES_ATTACH = "attach";
const std::string REQ_RES_TERMINATE = "terminate";
const std::string REQ_RES_CONFIGURATION_DONE = "configurationDone";
const std::string REQ_RES_RESTART = "restart";
const std::string REQ_RES_DISCONNECT = "disconnect";
const std::string REQ_RES_BREAKPOINT_LOCATIONS = "breakpointLocations";
const std::string REQ_RES_BREAKPOINTS = "breakpoints";
const std::string REQ_RES_SET_BREAKPOINTS = "setBreakpoints";
const std::string REQ_RES_SET_FUNCTION_BREAKPOINTS = "setFunctionBreakpoints";
const std::string REQ_RES_SET_INSTRUCTION_BREAKPOINTS = "setInstructionBreakpoints";
const std::string REQ_RES_DATA_BREAKPOINT_INFO = "dataBreakpointInfo";
const std::string REQ_RES_SET_EXCEPTION_BREAKPOINTS = "setExceptionBreakpoints";
const std::string REQ_RES_SET_DATA_BREAKPOINTS = "setDataBreakpoints";
const std::string REQ_RES_CONTINUE = "continue";
const std::string REQ_RES_NEXT = "next";
const std::string REQ_RES_VARIABLES = "variables";
const std::string REQ_RES_VARIABLES_EXPAND = "variablesExpand";
const std::string REQ_RES_PRE_VARIABLES = "preVariables";
const std::string REQ_RES_STEP_IN = "stepIn";
const std::string REQ_RES_STEP_OUT = "stepOut";
const std::string REQ_RES_STEP_BACK = "stepBack";
const std::string REQ_RES_REVERSE_CONTINUE = "reverseContinue";
const std::string REQ_RES_RESTART_FRAME = "restartFrame";
const std::string REQ_RES_GOTO = "goto";
const std::string REQ_RES_PAUSE = "pause";
const std::string REQ_RES_STACK_TRACE = "stackTrace";
const std::string REQ_RES_SCOPES = "scopes";
const std::string REQ_RES_SET_VARIABLE = "setVariable";
const std::string REQ_RES_SOURCE = "source";
const std::string REQ_RES_THREADS = "threads";
const std::string REQ_RES_TERMINATE_THREADS = "terminateThreads";
const std::string REQ_RES_MODULES = "modules";
const std::string REQ_RES_LOAD_MODULES = "loadModules";
const std::string REQ_RES_LOADED_SOURCES = "loadedSources";
const std::string REQ_RES_ADD_SOURCE_MAPPING = "addSourceMapping";
const std::string REQ_RES_RESET_SOURCE_MAPPING = "resetSourceMapping";
const std::string REQ_RES_EVALUATE = "evaluate";
const std::string REQ_RES_SET_EXPRESSION = "setExpression";
const std::string REQ_RES_STEP_IN_TARGETS = "stepInTargets";
const std::string REQ_RES_GOTO_TARGETS = "gotoTargets";
const std::string REQ_RES_COMPLETIONS = "completions";
const std::string REQ_RES_EXCEPTION_INFO = "exceptionInfo";
const std::string REQ_RES_READ_MEMORY = "readMemory";
const std::string REQ_RES_WRITE_MEMORY = "writeMemory";
const std::string REQ_RES_DISASSEMBLE = "disassemble";
const std::string REQ_RES_RUN_IN_TERMINAL = "runInTerminal";
const std::string REQ_RES_DEBUG_IN_CONSOLE = "debugInConsole";
const std::string REQ_RES_REVERSE_STEP = "reverseStep";
const std::string REQ_RES_UNDO = "undo";
const std::string REQ_RES_SET_EXECUTION_POINT = "setExecutionPoint";
const std::string REQ_RES_GOTO_RECORD_POINT = "gotoRecordPoint";
const std::string REQ_RES_RECORD_POINTS = "recordPoints";
const std::string REQ_RES_CONTINUE_IN_REVERSE = "continueInReverse";
const std::string REQ_RES_STEP_IN_REVERSE = "stepInReverse";
const std::string REQ_RES_CACHE_DATA_CONFIG = "cacheDataConfig";
const std::string REQ_RES_RECORD_MESSAGE = "recordMessage";
const std::string REQ_RES_SET_MEMORY_BREAKPOINTS = "setMemoryBreakpoints";
const std::string REQ_RES_GET_BITMAP = "getBitmap";
const std::string REQ_RES_ADD_SOURCE_BREAKPOINT = "addSourceBreakpoint";
const std::string REQ_RES_DELETE_BREAKPOINT = "deleteBreakpoint";
const std::string REQ_RES_VARIABLE_HEX_VIEW = "variableHexView";
const std::string REQ_RES_COMMAND_COMPLETION = "commandCompletion";

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOL_DEFS_H