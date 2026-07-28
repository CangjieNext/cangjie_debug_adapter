// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_CHANNEL_BASE_SUITE_H
#define DAP_CHANNEL_BASE_SUITE_H

#include <gtest/gtest.h>

class ChannelBaseSuite : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

#endif // DAP_CHANNEL_BASE_SUITE_H
