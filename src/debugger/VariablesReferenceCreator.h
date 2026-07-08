// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_VARIABLES_REFERENCECREATOR_H
#define DAP_SERVER_VARIABLES_REFERENCECREATOR_H

#include <mutex>

namespace Dap {
class VariablesReferenceCreator {
public:
    static VariablesReferenceCreator &Instance();
    int Create();

private:
    VariablesReferenceCreator() = default;
    ~VariablesReferenceCreator() = default;

private:
    // The value should be more than or equal to 1000 [defined by Huawei]
    static constexpr int START_ID = 1000;
    // The value should be less than or equal to 2147483647 (2^31-1). [defined by DAP]
    static constexpr int MAX_ID = std::numeric_limits<int>::max();
    std::mutex idMutex;
    int variablesReferenceId = START_ID;
};
};

#endif // DAP_SERVER_VARIABLES_REFERENCECREATOR_H