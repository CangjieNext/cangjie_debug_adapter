// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "UniqueIdCreator.h"

using namespace Dap;

UniqueIdCreator &UniqueIdCreator::IdCreator()
{
    static UniqueIdCreator instance;
    return instance;
}

uint64_t UniqueIdCreator::Build()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (id >= maxId) {
        id = 0;
    }
    return id++;
}

std::string UniqueIdCreator::ToString(const std::string &prefix, bool isNew)
{
    if (isNew) {
        Build();
    }
    return prefix + std::to_string(id);
}