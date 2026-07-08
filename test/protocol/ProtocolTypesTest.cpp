// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "ProtocolSuite.h"
#include "Protocol.h"

using namespace Dap::Protocol;

class ProtocolTypesTest : public ProtocolSuite {
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ProtocolTypesTest, ToLocalPath)
{
    std::string remotePath = "C:\\main.cpp";
    std::string localPath = "C:\\main.cpp";
    SourceFileMap fileMap(remotePath, localPath);
    std::string path = fileMap.ToLocalPath(localPath);
    EXPECT_EQ(path, "C:/main.cpp");

    fileMap.remotePath = "D:\\main.cpp";
    std::string newPath = fileMap.ToLocalPath(localPath);
    EXPECT_EQ(newPath, "C:/main.cpp");
}

TEST_F(ProtocolTypesTest, ToRemotePath)
{
    std::string remotePath = "C:\\main.cpp";
    std::string localPath = "C:\\main.cpp";
    SourceFileMap fileMap(remotePath, localPath);
    std::string path = fileMap.ToRemotePath(remotePath);
    EXPECT_EQ(path, "C:/main.cpp");

    fileMap.localPath = "D:\\main.cpp";
    std::string newPath = fileMap.ToRemotePath(remotePath);
    EXPECT_EQ(newPath, "C:/main.cpp");
}