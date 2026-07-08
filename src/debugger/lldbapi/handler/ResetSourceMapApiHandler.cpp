// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "AddSourceMapApiHandler.h"
#include "ResponseManager.h"
#include "ResetSourceMapApiHandler.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr ResetSourceMapApiHandler::HandleRequest(UniqueRequestPtr request)
{
    currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<ResetSourceMappingRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        return debugger->CompleteFailureResponse(std::make_unique<ResetSourceMappingResponse>(),
            "ERROR: ResetSourceMapping Pointer conversion failed.", Protocol::SEQ_INVALID);
    }
    if (reqPtr->arguments.moduleIdList.empty() || reqPtr->arguments.sourceRootDirList.empty()
        || reqPtr->arguments.moduleIdList.size() != reqPtr->arguments.sourceRootDirList.size()) {
        return debugger->CompleteFailureResponse(std::make_unique<ResetSourceMappingResponse>(),
            "ERROR: Size of moduleIdList and sourceRootDirList should be same (not 0)", currentRequest->seq);
    }
    std::unique_ptr<ResetSourceMappingResponse> responsePointer = std::make_unique<ResetSourceMappingResponse>();
    debugger->InitResponse(*responsePointer,  currentRequest->seq, true);

    std::string sourceMapStr;
    if (!debugger->GetSourceMap(sourceMapStr, error)) {
        return debugger->CompleteFailureResponse(
            std::make_unique<ResetSourceMappingResponse>(), error, currentRequest->seq);
    }

    auto sourceMap = debugger->ParseSourceMap(sourceMapStr);

    // Remove source-map settings of request
    for (unsigned i = 0; i < reqPtr->arguments.moduleIdList.size(); i++) {
        std::string srcPath = reqPtr->arguments.sourceRootDirList[i];
        // srcPath (settings set target.source-map srcPath destPath) stored in sourceMap is in current OS path format
#ifdef _WIN32
        // on Windows, srcPath shown in "settings show target.source-map" was transformed into Windows format("\"->"/")
        if (!sourceMap.count(CommonUtils::GetFilePathByPlatform(srcPath))) {
            continue;
        }
        // remove srcPath -> xxx in target.source-map
        sourceMap.erase(CommonUtils::GetFilePathByPlatform(srcPath));
#else
        if (!sourceMap.count(srcPath)) {
            continue;
        }
        sourceMap.erase(srcPath);
#endif
        responsePointer->moduleIdList.emplace_back(reqPtr->arguments.moduleIdList[i]);
    }

    if (!error.empty()) {
        responsePointer->error = error;
    }

    if (!ResetSourceMap(sourceMap)) {
        return debugger->CompleteFailureResponse(
            std::make_unique<ResetSourceMappingResponse>(), error, currentRequest->seq);
    }

    return responsePointer;
}

bool ResetSourceMapApiHandler::ResetSourceMap(const std::unordered_map<std::string, std::string>& sourceMap)
{
    // Execute "settings set target.source-map src1 dest1 src2 dest2" to redo source map settings
    std::string command;
    if (sourceMap.empty()) {
        command = CLEAR_SOURCE_MAP_COMMAND;
    } else {
        command = SET_SOURCE_MAP_COMMAND;
        for (auto& iter : sourceMap) {
            command.append("\"").append(iter.first).append("\" \"").append(iter.second).append("\" ");
        }
    }

    return debugger->ExecuteSourceMapCommand(command, error);
}