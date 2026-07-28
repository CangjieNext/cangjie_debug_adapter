// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_CONTAINER_MANAGER_H
#define DAP_LLDBAPI_CONTAINER_MANAGER_H

#include <variant>
#include <string>
#include <memory>
#include <mutex>
#include <map>
#include "LLDB.h"
#include "CommonUtils.h"
#include "VariablesReferenceCreator.h"

namespace Dap {
enum class DapContainer {
    STACKFRAME,
    LOCALS,
    STATICS,
    GLOBALS,
    VALUE,
    REGISTERS,
    RETURN_VALUE,
};

struct ContainerInfo {
    DapContainer containerType;
    std::variant<lldb::SBFrame, lldb::SBValue> container;
    int64_t threadId = -1;
    uint32_t frameIndex = 0;
};

class ContainerManager {
public:
    ContainerManager() {}
    ~ContainerManager() = default;

    // Get frameId or variablesReference
    int64_t GetContainerIndex(int64_t parentIndex, const std::string &childName)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (childIndexCache.count(std::make_tuple(parentIndex, childName))) {
            return childIndexCache[std::make_tuple(parentIndex, childName)];
        } else {
            int newIndex = NextContainerIndex();
            childIndexCache[std::make_tuple(parentIndex, childName)] = newIndex;
            return newIndex;
        }
    }

    void SetContainerInfo(int64_t containerIndex, const ContainerInfo &container)
    {
        std::lock_guard<std::mutex> lock(mutex);
        // if not exist, add it, otherwise overwrite it
        index2ContainerInfo[containerIndex] = container;
    }

    ContainerInfo GetContainerInfo(int64_t containerIndex)
    {
        std::lock_guard<std::mutex> lock(mutex);
        ContainerInfo info;
        if (index2ContainerInfo.count(containerIndex) != 0) {
            info = index2ContainerInfo[containerIndex];
        }
        return info;
    }

    void SetExprOfVarRef(int64_t variableReference, const std::string &expression)
    {
        std::lock_guard<std::mutex> lock(mutex);
        // if not exist, add it, otherwise overwrite it
        varRef2Expr[variableReference] = expression;
    }

    std::string GetExprOfVarRef(int64_t variableReference)
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::string expression;
        if (varRef2Expr.count(variableReference) != 0) {
            expression = varRef2Expr[variableReference];
        }
        return expression;
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lock(mutex);
        index2ContainerInfo.clear();
        childIndexCache.clear();
        varRef2Expr.clear();
    }

    lldb::SBValue FindVarSBValue(const std::string &varName, int64_t &containerRef, lldb::SBDebugger &sbDebugger);

private:
    int64_t NextContainerIndex() const
    {
        return Dap::VariablesReferenceCreator::Instance().Create();
    }

private:
    std::map<int64_t, ContainerInfo> index2ContainerInfo; // index may be frameId or variablesReference
    std::map<std::tuple<int64_t, std::string>, int64_t> childIndexCache;
    std::map<int64_t, std::string> varRef2Expr;
    std::mutex mutex;
};
};

#endif // DAP_LLDBAPI_CONTAINER_MANAGER_H