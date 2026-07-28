// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLREQUESTS_EXT_H
#define DAP_PROTOCOLREQUESTS_EXT_H

#include <optional>
#include "defs.h"
#include "ProtocolBase.h"
#include "ProtocolDefs.h"
#include "ProtocolTypes.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

// GetBitmapRequest
struct GetBitmapArguments {
    std::string varName;
    std::optional<int64_t> frameId;
    std::optional<std::string> varAddress;
    std::optional<std::string> varType;
    std::optional<bool> hasData;
};

struct GetBitmapRequest : public Request {
    GetBitmapRequest() : Request(REQ_RES_GET_BITMAP) {}

    GetBitmapArguments arguments;
};

struct LoadModulesArguments {
    std::vector<std::string> uuidList;
    std::vector<std::string> pathList;
};

struct LoadModuleRequest : public Request {
    LoadModuleRequest() : Request(REQ_RES_LOAD_MODULES) {}

    LoadModulesArguments arguments;
};

struct AddSourceMappingArguments {
    std::vector<int32_t> moduleIdList;
    std::vector<std::string> targetSourcePathList;
};

struct AddSourceMappingRequest : public Request {
    AddSourceMappingRequest() : Request(REQ_RES_ADD_SOURCE_MAPPING) {}

    AddSourceMappingArguments arguments;
};

struct ResetSourceMappingArguments {
    std::vector<int32_t> moduleIdList;
    std::vector<std::string> sourceRootDirList;
};

struct ResetSourceMappingRequest : public Request {
    ResetSourceMappingRequest() : Request(REQ_RES_RESET_SOURCE_MAPPING) {}

    ResetSourceMappingArguments arguments;
};

struct VariableHexViewArguments {
    bool isEnable;  // if enable is true, type format add all the integer types; otherwise type format clear
    std::optional<bool> isArrayView;
    std::vector<std::string> formats; // integer type strings
};

struct VariableHexViewRequest : public Request {
    VariableHexViewRequest() : Request(REQ_RES_VARIABLE_HEX_VIEW) {}

    VariableHexViewArguments arguments;
};

// AddSourceBreakpointArguments
struct AddSourceBreakpointArguments {
    Source source;
    int64_t line;
    std::optional<int64_t> column;
};
struct AddSourceBreakpointRequest : public Request {
    AddSourceBreakpointRequest() : Request(REQ_RES_ADD_SOURCE_BREAKPOINT) {}
    AddSourceBreakpointArguments arguments;
};

// DeleteBreakpoint
struct DeleteBreakpointArguments {
    int64_t id;
};

struct DeleteBreakpointRequest : public Request {
    DeleteBreakpointRequest() : Request(REQ_RES_DELETE_BREAKPOINT) {}
    DeleteBreakpointArguments arguments;
};

// CommandCompletion
struct CommandCompletionArguments {
    std::string command;
};

struct CommandCompletionRequest : public Request {
    CommandCompletionRequest() : Request(REQ_RES_COMMAND_COMPLETION) {}
    CommandCompletionArguments arguments;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLREQUESTS_EXT_H
