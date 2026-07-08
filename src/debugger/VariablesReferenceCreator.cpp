// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "VariablesReferenceCreator.h"

using namespace Dap;

VariablesReferenceCreator &VariablesReferenceCreator::Instance()
{
    static VariablesReferenceCreator creator;
    return creator;
}

int VariablesReferenceCreator::Create()
{
    std::lock_guard<std::mutex> lock(idMutex);
    if (variablesReferenceId == VariablesReferenceCreator::MAX_ID) {
        variablesReferenceId = VariablesReferenceCreator::START_ID;
    }
    return variablesReferenceId++;
}