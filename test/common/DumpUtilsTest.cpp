// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "DumpUtils.h"
#include "CommonSuite.h"

using namespace Dap;

class DumpUtilsTest : public CommonSuite {};

TEST_F(DumpUtilsTest, Instance)
{
    (void)DumpUtils::Instance();
}

TEST_F(DumpUtilsTest, SetDumpFileDir)
{
#ifdef _WIN32
    DumpUtils::Instance().SetDumpFileDir("D:\\");
#else
    DumpUtils::Instance().SetDumpFileDir("/tmp");
#endif
}

TEST_F(DumpUtilsTest, GetDumpFilePath)
{
    std::string path = DumpUtils::Instance().GetDumpFilePath();
    EXPECT_TRUE(!path.empty());
}

TEST_F(DumpUtilsTest, StartRecordDumpInfo)
{
#ifdef _WIN32
    DumpUtils::Instance().StartRecordDumpInfo();
#endif
}