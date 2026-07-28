// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "AddSourceMapApiHandler.h"
#include "ResponseManager.h"
#include <cctype>

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr AddSourceMapApiHandler::HandleRequest(UniqueRequestPtr request)
{
    currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<AddSourceMappingRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        return debugger->CompleteFailureResponse(std::make_unique<AddSourceMappingResponse>(),
            "ERROR: AddSourceMapping Pointer conversion failed.", Protocol::SEQ_INVALID);
    }
    if (reqPtr->arguments.targetSourcePathList.empty() || reqPtr->arguments.moduleIdList.empty()
        || reqPtr->arguments.targetSourcePathList.size() != reqPtr->arguments.moduleIdList.size()) {
        return debugger->CompleteFailureResponse(std::make_unique<AddSourceMappingResponse>(),
            "ERROR: Size of moduleIdList and targetSourcePathList should be same (not 0)", currentRequest->seq);
    }
    std::unique_ptr<AddSourceMappingResponse> responsePointer = std::make_unique<AddSourceMappingResponse>();
    debugger->InitResponse(*responsePointer,  currentRequest->seq, true);

    std::string sourceMapStr;
    if (!debugger->GetSourceMap(sourceMapStr, error)) {
        return debugger->CompleteFailureResponse(
            std::make_unique<ResetSourceMappingResponse>(), error, currentRequest->seq);
    }

    auto sourceMap = debugger->ParseSourceMap(sourceMapStr);

    for (unsigned i = 0; i < reqPtr->arguments.moduleIdList.size(); i++) {
        int moduleId = reqPtr->arguments.moduleIdList[i];
        SBModule sbModule = debugger->GetSbTarget().GetModuleAtIndex(moduleId);
        if (!sbModule.IsValid() || !sbModule.GetNumCompileUnits()) {
            error.append("ERROR: Module with id: " + to_string(moduleId) + " is invalid!\n");
            continue;
        }

        std::string srcPath = ExtractCompiledRootDirPath(sbModule);
        std::string moduleName = sbModule.GetFileSpec().GetFilename();
        if (srcPath.empty()) {
            error.append("ERROR: Source map on " + moduleName + " err, source files distributed on multi platforms!\n");
            continue;
        }
        sourceMap[srcPath] = reqPtr->arguments.targetSourcePathList[i];
        responsePointer->moduleIdList.emplace_back(reqPtr->arguments.moduleIdList[i]);
        responsePointer->sourceRootDirList.emplace_back(srcPath);
    }

    if (!error.empty()) {
        responsePointer->error = error;
    }

    if (sourceMap.empty()) {
        return responsePointer;
    }

    if (!SetSourceMap(sourceMap)) {
        return debugger->CompleteFailureResponse(
            std::make_unique<ResetSourceMappingResponse>(), error, currentRequest->seq);
    }

    return responsePointer;
}

std::string AddSourceMapApiHandler::ExtractCompiledRootDirPath(lldb::SBModule &sbModule)
{
    std::string commonPath;
    for (unsigned i = 0; i < sbModule.GetNumCompileUnits(); i++) {
        // only c++ source mapping supported
        if (!sourceMappingSupportLanguageType.count(sbModule.GetCompileUnitAtIndex(i).GetLanguage())) {
            continue;
        }
        std::string curDirectory = sbModule.GetCompileUnitAtIndex(i).GetFileSpec().GetDirectory();
        // ignore source xxx.cpp file from toolchain
        if (CommonUtils::RegexSearch(curDirectory, OH_TOOLCHAIN_PATH_PATTERN)) {
            continue;
        }
        if (commonPath.empty()) {
            commonPath = curDirectory;
        } else {
            commonPath = CommonUtils::FindLongestCommonPathPrefix(commonPath, curDirectory);
        }
    }
    return commonPath;
}

bool AddSourceMapApiHandler::SetSourceMap(const std::unordered_map<std::string, std::string>& sourceMap)
{
    // Execute "settings set target.source-map src1 dest1 src2 dest2" to redo source map settings
    std::string command = SET_SOURCE_MAP_COMMAND;
    for (auto& iter : sourceMap) {
        if (!IsValidPath(iter.first) || !IsValidPath(iter.second)) {
            error = "ERROR: Invalid path contains dangerous characters.";
            return false;
        }
        command.append("\"").append(iter.first).append("\" \"").append(iter.second).append("\" ");
    }

    return debugger->ExecuteSourceMapCommand(command, error);
}

bool AddSourceMapApiHandler::IsValidPath(const std::string &path) const
{
    if (path.empty()) {
        return false;
    }
    for (char c : path) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!isalnum(uc) && c != '_' && c != '-' && c != '.' && c != '/' && c != '\\' && c != ':' && c != ' ' &&
            c != '[' && c != ']' && c != '(' && c != ')') {
            return false;
        }
    }
    return true;
}