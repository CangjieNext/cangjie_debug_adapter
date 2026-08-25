// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "DapParamParser.h"
#include "CommonSuite.h"
#include "Option.h"

using namespace Dap;
using namespace std;

class DapParamParserTest : public CommonSuite {
protected:
    static constexpr int testPort = 9995;
    static constexpr size_t overlongPathLength = 261;
    static constexpr int defaultLogSize = 32;
    static constexpr int testLogSize = 1024;
    static constexpr int invalidPort = -1;
    static constexpr const char *invalidLogPath = "1234";

    void SetUp() override
    {
        DapParamParser::Instance().Clear();
    }
    void TearDown() override {}
};

TEST_F(DapParamParserTest, ParseDebugOption_params_one)
{
    std::vector<std::string> args = { "dap_server" };
    Option option;
    bool res = DapParamParser::Instance().ParseDebugOption(args, option);
    EXPECT_FALSE(res);
}

TEST_F(DapParamParserTest, ParseDebugOption_link_type_error)
{
    std::vector<std::string> args = {
        DapParamParser::SYMBOL_PORT + std::to_string(invalidPort), "--link=socket"
    };
    Option option;
    bool result = DapParamParser::Instance().ParseDebugOption(args, option);
    string error = DapParamParser::Instance().Error();
    EXPECT_EQ(result, false);
    EXPECT_FALSE(error.empty());
}

TEST_F(DapParamParserTest, ParseDebugOption_log_path_error)
{
    std::vector<std::string> args = {
        DapParamParser::SYMBOL_PORT + std::to_string(testPort), "--logpath=*:\\?"
    };
    Option option;
    bool result = DapParamParser::Instance().ParseDebugOption(args, option);
    string error = DapParamParser::Instance().Error();
    EXPECT_EQ(result, false);
    EXPECT_FALSE(error.empty());
}

TEST_F(DapParamParserTest, GetPort)
{
    string data = DapParamParser::SYMBOL_PORT + std::to_string(invalidPort);
    DapParamParser::Instance().ParseField(data);
    int result = DapParamParser::Instance().GetPort();
    string error = DapParamParser::Instance().Error();
    EXPECT_EQ(result, invalidPort);
    EXPECT_FALSE(error.empty());
}

TEST_F(DapParamParserTest, GetPort_1)
{
    string data = DapParamParser::SYMBOL_PORT + std::to_string(testPort);
    DapParamParser::Instance().ParseField(data);
    int result = DapParamParser::Instance().GetPort();
    EXPECT_EQ(result, testPort);
}

TEST_F(DapParamParserTest, GetLogPath)
{
    string data = std::string(DapParamParser::SYMBOL_LOG_PATH) + invalidLogPath;
    DapParamParser::Instance().ParseField(data);
    string result = DapParamParser::Instance().GetLogPath();
    string error = DapParamParser::Instance().Error();
    EXPECT_EQ(result, "");
#ifdef _WIN32
    EXPECT_EQ(error, "ERROR: log path has illegal characters or is created failed.");
#else
    EXPECT_EQ(error, "ERROR: log path does not exist or is illegal!");
#endif
}

TEST_F(DapParamParserTest, GetLogPath_1)
{
    string data = "--logpath=D:/";
    DapParamParser::Instance().ParseField(data);
    string result = DapParamParser::Instance().GetLogPath();
#ifdef _WIN32
    EXPECT_EQ(result, "D:\\/");
#else
    EXPECT_EQ(result, "");
#endif
}

TEST_F(DapParamParserTest, ErrorPath)
{
    string data = "abc";
    DapParamParser::Instance().ParseField(data);
    string result = DapParamParser::Instance().Error();
}

TEST_F(DapParamParserTest, ErrorPath_2)
{
    string overlongPath(overlongPathLength, 'a');
    string data = "--logpath=D:/" + overlongPath;
    bool result = DapParamParser::Instance().ParseField(data);
    string error = DapParamParser::Instance().Error();
    EXPECT_EQ(result, false);
#ifdef _WIN32
    EXPECT_EQ(error, "ERROR: log path is too long!");
#else
    EXPECT_EQ(error, "ERROR: log path does not exist or is illegal!");
#endif
}

TEST_F(DapParamParserTest, LinkType)
{
    string result = DapParamParser::Instance().GetLinkType();
    EXPECT_EQ(result, DapParamParser::Instance().LINK_TYPE_STD);

    string data = "--link=socket";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLinkType();
    EXPECT_EQ(result, DapParamParser::Instance().LINK_TYPE_SOCKET);

    data = "--link=std";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLinkType();
    EXPECT_EQ(result, DapParamParser::Instance().LINK_TYPE_STD);
}

TEST_F(DapParamParserTest, LogLevel)
{
    string result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "INFO");

    string data = "--loglevel=DEBUG";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "DEBUG");

    data = "--loglevel=INFO";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "INFO");

    data = "--loglevel=WARN";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "WARN");

    data = "--loglevel=ERROR";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "ERROR");

    data = "--loglevel=FATAL";
    DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogLevel();
    EXPECT_EQ(result, "FATAL");
}

TEST_F(DapParamParserTest, LogSize)
{
    int result = DapParamParser::Instance().GetLogSize();
    EXPECT_EQ(result, defaultLogSize);

    string data = DapParamParser::SYMBOL_LOG_SIZE + std::to_string(invalidPort);
    bool flag = DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogSize();
    EXPECT_FALSE(flag);
    EXPECT_EQ(result, defaultLogSize);

    data = DapParamParser::SYMBOL_LOG_SIZE + std::to_string(testLogSize);
    flag = DapParamParser::Instance().ParseField(data);
    result = DapParamParser::Instance().GetLogSize();
    EXPECT_TRUE(flag);
    EXPECT_EQ(result, testLogSize);
}
