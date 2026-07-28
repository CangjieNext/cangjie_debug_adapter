// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelBaseSuite.h"
#include "UvLoopMgr.h"

using namespace ::testing;
using namespace Dap;

class UvLoopMgrSuite : public ChannelBaseSuite {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(UvLoopMgrSuite, loop_start)
{
    int loopId = -2;
    EXPECT_FALSE(UvLoopMgr::Instance().IsLoopStart(loopId));
    loopId = -1;
    EXPECT_TRUE(UvLoopMgr::Instance().IsLoopStart(loopId));
}

TEST_F(UvLoopMgrSuite, loop_exit)
{
    int loopId = -2;
    EXPECT_FALSE(UvLoopMgr::Instance().IsLoopExist(loopId));
    loopId = -1;
    EXPECT_TRUE(UvLoopMgr::Instance().IsLoopExist(loopId));
}

TEST_F(UvLoopMgrSuite, loop_remove)
{
    int loopId = -1;
    UvLoopMgr::Instance().LoopRemove(loopId);
    loopId = -2;
    UvLoopMgr::Instance().LoopRemove(loopId);
    loopId = 0;
    (void)UvLoopMgr::Instance().Loop(loopId);
    UvLoopMgr::Instance().LoopRemove(loopId);
}