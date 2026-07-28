// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_COMMON_SUITE_H
#define DAP_SERVER_COMMON_SUITE_H

#include "gtest/gtest.h"

class CommonSuite : public testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

#endif // DAP_SERVER_COMMON_SUITE_H