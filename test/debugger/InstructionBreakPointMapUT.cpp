// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "InstructionBreakPointMap.h"

using namespace std;
using namespace Dap;

namespace {
constexpr int FIRST_BREAKPOINT_ID = 1;
constexpr int SECOND_BREAKPOINT_ID = 2;
}

class InstructionBreakPointMapUtilsTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

TEST_F(InstructionBreakPointMapUtilsTest, Functest)
{
    InstructionBreakPointMap instructionBreakPointMap;
    instructionBreakPointMap.AddBreakPoint(FIRST_BREAKPOINT_ID, "abc");
    instructionBreakPointMap.AddBreakPoint(SECOND_BREAKPOINT_ID, "def");

    int breakpointId = FIRST_BREAKPOINT_ID;
    string result = instructionBreakPointMap.GetInstructionReference(breakpointId);
    EXPECT_EQ(result, "abc");

    std::vector<int> breakpointIds = instructionBreakPointMap.GetAllBreakpointId();
    EXPECT_FALSE(breakpointIds.empty());

    bool ret = instructionBreakPointMap.IsInstructionBreakPoint(breakpointId);
    EXPECT_TRUE(ret);

    instructionBreakPointMap.RemoveAllBreakPoint();
}
