// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#include <string>
#include <gtest/gtest.h>
#include "CommonSuite.h"
#include "TimeUtils.h"

using namespace ::testing;
using namespace Dap;

class TimeUtilsTest : public CommonSuite {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TimeUtilsTest, now)
{
    std::unique_ptr<Time> time = TimeUtils::Instance().Now();
    EXPECT_TRUE(time->year > 0);
    EXPECT_TRUE(time->month > 0);
    EXPECT_TRUE(time->day > 0);
    EXPECT_TRUE(time->hour >= 0);
    EXPECT_TRUE(time->minute >= 0);
    EXPECT_TRUE(time->second >= 0);
    EXPECT_TRUE(time->millisecond >= 0);
}

TEST_F(TimeUtilsTest, now_str)
{
    std::string timeStr = TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_1);
    EXPECT_TRUE(!timeStr.empty());
    timeStr = TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_2);
    EXPECT_TRUE(!timeStr.empty());
    timeStr = TimeUtils::Instance().NowStr(TimeStyle::DATE_TIME_3);
    EXPECT_TRUE(!timeStr.empty());
    timeStr = TimeUtils::Instance().NowStr(TimeStyle::DATE_1);
    EXPECT_TRUE(!timeStr.empty());
    timeStr = TimeUtils::Instance().NowStr(TimeStyle::TIME_1);
    EXPECT_TRUE(!timeStr.empty());
}