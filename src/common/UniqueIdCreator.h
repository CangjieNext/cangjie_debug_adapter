// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_UNIQUE_ID_CREATOR_H
#define DAP_UNIQUE_ID_CREATOR_H

#include <mutex>
#include <string>

namespace Dap {
    class UniqueIdCreator {
    public:
        static UniqueIdCreator &IdCreator();
        uint64_t Build();
        std::string ToString(const std::string &prefix, bool isNew);

    private:
        UniqueIdCreator() = default;
        ~UniqueIdCreator() = default;

    private:
        const static uint64_t maxId = 0x7dffffffffffffff;
        uint64_t id = 0;
        std::mutex mtx;
    };
}

#endif // DAP_UNIQUE_ID_CREATOR_H
