// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include <cstdint>
#include <limits>
#include "CommonUtils.h"
#include "CommonSuite.h"
#include "json.hpp"

using namespace Dap;
using namespace std;
using json = nlohmann::json;

class CommonUtilsTest : public CommonSuite {
protected:
    static constexpr int logFileLimit = 10;
    static constexpr size_t thirdSplitPartIndex = 2;
    static constexpr int largeRandomStringLength = 1024;
    static constexpr long long decimalAddress = 100;
    static constexpr int formatWidth = 10;
    static constexpr size_t overlongPathLength = 261;
    static constexpr std::int64_t secondFileTimestamp = 2;
    static constexpr std::int64_t thirdFileTimestamp = 3;
    static constexpr int invalidAddressLength = 15;
    static constexpr uint64_t zeroValue = 0;
    static constexpr size_t expectedFileCount = 1;
};

TEST_F(CommonUtilsTest, RegexSearch)
{
    string data = "123";
    string pattern = "\\d";
    bool result = CommonUtils::RegexSearch(data, pattern);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, RegexSearch_1)
{
    string data = "abc";
    string pattern = "\\d";
    bool result = CommonUtils::RegexSearch(data, pattern);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, RegexMatch)
{
    string data = "123";
    string pattern = "\\d+";
    std::optional<std::smatch> result = CommonUtils::RegexMatch(data, pattern);
    EXPECT_EQ(result.value()[0], "123");
}

TEST_F(CommonUtilsTest, RegexMatch_1)
{
    string data = "abc";
    string pattern = "\\d+";
    std::optional<std::smatch> result = CommonUtils::RegexMatch(data, pattern);
    EXPECT_EQ(result, std::nullopt);
}

TEST_F(CommonUtilsTest, RegexSearchIgnoreCase)
{
    string data = "123";
    string pattern = "\\d";
    bool result = CommonUtils::RegexSearchIgnoreCase(data, pattern);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, IsFileExist)
{
    string filePath = "123";
    bool result = CommonUtils::IsFileExist(filePath);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, GetFileName)
{
    string filePath = "D:/project/code/a.exe";
    string result = CommonUtils::GetFileName(filePath);
    EXPECT_EQ(result, "a.exe");
}

TEST_F(CommonUtilsTest, RemoveFile)
{
#ifdef _WIN32
    string filePath = "D:/CommonUtilsTest_RemoveFile.exe";
#else
    string filePath = "/tmp/CommonUtilsTest_RemoveFile";
#endif
    bool result = CommonUtils::RemoveFile(filePath);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, GetFilesToBeDeleted)
{
#ifdef _WIN32
    string folder = "D:";
#else
    string folder = "/tmp";
#endif
    string start = "dap_server";
    string ext = "...abc";
    float limitDays = 1;
    int limitNums = logFileLimit;
    std::vector<std::string> result = CommonUtils::GetFilesToBeDeleted(folder, start, ext, limitDays, limitNums);
    EXPECT_EQ(result.empty(), true);
}

TEST_F(CommonUtilsTest, DeleteObsoleteLogs_0)
{
    float days = 0.0f;
    int nums = logFileLimit;
    CommonUtils::DeleteObsoleteLogs("./", "dap_server", "log", days, nums);
}

TEST_F(CommonUtilsTest, DeleteObsoleteLogs_1)
{
    float days = CommonUtils::LIMIT_DAYS;
    int nums = logFileLimit;
#ifdef _WIN32
    std::string folder = "D:\\";
#else
    std::string folder = "/tmp/";
#endif
    CommonUtils::DeleteObsoleteLogs(folder, "dap_server", "log", days, nums);
}

TEST_F(CommonUtilsTest, IsFolderExist)
{
    string folderPath = "";
    bool result = CommonUtils::IsFolderExist(folderPath);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, IsFolderExist_1)
{
#ifdef _WIN32
    string folderPath = "D:\\";
#else
    string folderPath = "/tmp";
#endif
    bool result = CommonUtils::IsFolderExist(folderPath);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, CreateFolder)
{
#ifdef _WIN32
    string folderPath = "D:\\";
#else
    string folderPath = "/tmp";
#endif
    bool result = CommonUtils::CreateFolder(folderPath);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, CreateFolder_1)
{
    string folderPath = "";
    bool result = CommonUtils::CreateFolder(folderPath);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, StartWithString)
{
    string src = "";
    string start = "";
    bool result = CommonUtils::StartWithString(src, start);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, StartWithString_1)
{
    string src = "abc12345";
    string start = "abc";
    bool result = CommonUtils::StartWithString(src, start);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, EndWithString)
{
    string src = "";
    string start = "";
    bool result = CommonUtils::EndWithString(src, start);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, EndWithString_1)
{
    string src = "12345abc";
    string end = "abc";
    bool result = CommonUtils::EndWithString(src, end);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, IsValidInteger)
{
    string src = "abc";
    bool result = CommonUtils::IsValidInteger(src);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, IsValidInteger_1)
{
    string src = "123";
    bool result = CommonUtils::IsValidInteger(src);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, IsValidInteger_2)
{
    string src = "2147483647";
    bool result = CommonUtils::IsValidInteger(src);
    EXPECT_EQ(result, true);
}

TEST_F(CommonUtilsTest, IsValidInteger_3)
{
    string src = "214748364700";
    bool result = CommonUtils::IsValidInteger(src);
    EXPECT_EQ(result, false);
}

TEST_F(CommonUtilsTest, SplitString)
{
    string str = "";
    string delimiter = "";
    vector<string> result = CommonUtils::SplitString(str, delimiter);
    EXPECT_EQ(result.empty(), true);
}

TEST_F(CommonUtilsTest, SplitString_1)
{
    string str = "aa/bb/cc";
    string delimiter = "/";
    vector<string> result = CommonUtils::SplitString(str, delimiter);
    EXPECT_EQ(result[0], "aa");
    EXPECT_EQ(result[1], "bb");
    EXPECT_EQ(result[thirdSplitPartIndex], "cc");

    result = CommonUtils::SplitString(str, delimiter, true);
    EXPECT_EQ(result[0], "aa/");
}

TEST_F(CommonUtilsTest, StringToInt)
{
    string str = "abc";
    int integer = 0;
    bool result = CommonUtils::StringToInt(str, integer);
    EXPECT_EQ(result, false);
    EXPECT_EQ(integer, 0);
}

TEST_F(CommonUtilsTest, StringToInt_1)
{
    string str = "2147483647";
    int integer = 0;
    bool result = CommonUtils::StringToInt(str, integer);
    EXPECT_EQ(result, true);
    EXPECT_EQ(integer, std::numeric_limits<int>::max());
}

TEST_F(CommonUtilsTest, StringToLongLong)
{
    string str = "abc";
    int64_t integer = 0;
    bool result = CommonUtils::StringToLongLong(str, integer);
    EXPECT_EQ(result, false);
    EXPECT_EQ(integer, 0);
}

TEST_F(CommonUtilsTest, StringToLongLong_1)
{
    string str = "9223372036854775807";
    int64_t integer = 0;
    bool result = CommonUtils::StringToLongLong(str, integer);
    EXPECT_EQ(result, true);
    EXPECT_EQ(integer, std::numeric_limits<int64_t>::max());
}

TEST_F(CommonUtilsTest, GenRandomString)
{
    int length = -1;
    std::string res = CommonUtils::GenRandomString(length);
    EXPECT_TRUE(res.empty());
    length = 1;
    res = CommonUtils::GenRandomString(length);
    EXPECT_TRUE(res.length() == static_cast<size_t>(length));
    length = largeRandomStringLength;
    res = CommonUtils::GenRandomString(length);
    EXPECT_TRUE(res.length() == static_cast<size_t>(length));
}

TEST_F(CommonUtilsTest, CheckFilePath)
{
    std::string filePath = "./nonexistence.tmp";
    std::string error;
    bool res = CommonUtils::CheckFilePath(filePath, error);
    EXPECT_FALSE(res);
    res = CommonUtils::CheckFilePath("", error);
    EXPECT_FALSE(res);
#ifdef _WIN32
    filePath = "./Testgdb.exe";
#else
    filePath = "./Testgdb";
#endif
    res = CommonUtils::CheckFilePath(filePath, error);
    EXPECT_TRUE(res);
    std::string filePath2 = "./nonexistence*?.tmp";
    res = CommonUtils::CheckFilePath(filePath2, error);
#ifdef _WIN32
    EXPECT_EQ(error, "path has illegal characters.");
#else
    EXPECT_EQ(error, "path does not exist../nonexistence*?.tmp");
#endif
    EXPECT_FALSE(res);
}

TEST_F(CommonUtilsTest, TrimString)
{
    string data = "111abc111";
    string trim = "111";
    string res = CommonUtils::TrimString(data, trim, TrimType::BOTH);
    EXPECT_EQ(res, "abc");

    trim = "";
    res = CommonUtils::TrimString(data, trim, TrimType::BOTH);
    EXPECT_EQ(res, data);
}

TEST_F(CommonUtilsTest, Dec2HexAddress)
{
    long long num1 = decimalAddress;
    long long num2 = 0;
    int hex = CommonUtils::HEX_ADDRESS_WIDTH_32_BITS_CPU;
    string res1 = "0X00000064";
    string res2 = "0x00000064";
    string res3 = "0x00000000";
    string ret1 = CommonUtils::Dec2HexAddress(num1, hex, true);
    EXPECT_EQ(ret1, res1);
    string ret2 = CommonUtils::Dec2HexAddress(num1, hex, false);
    EXPECT_EQ(ret2, res2);
    string ret3 = CommonUtils::Dec2HexAddress(num2, hex, false);
    EXPECT_EQ(ret3, res3);
}

TEST_F(CommonUtilsTest, FormatCharPointer)
{
    char str[] = "abc";
    int opWidth = formatWidth;
    char opFill = 'z';
    string res1 = "abczzzzzzz";
    string res2 = "zzzzzzzabc";
    string ret1 = CommonUtils::FormatCharPointer(str, opWidth, opFill, true);
    EXPECT_EQ(ret1, res1);
    ret1 = CommonUtils::FormatCharPointer(str, opWidth, opFill, false);
    EXPECT_EQ(ret1, res2);
}

TEST_F(CommonUtilsTest, NormalizeFilePath)
{
    string address(overlongPathLength, 'a');
    string errorMSG;
    bool ret = CommonUtils::NormalizeFilePath(address, errorMSG);
#ifdef _WIN32
    EXPECT_EQ(errorMSG, "path is too long!");
#else
    EXPECT_EQ(errorMSG, "path does not exist or is illegal!");
#endif
    EXPECT_FALSE(ret);
}

TEST_F(CommonUtilsTest, IsValidAlphaAndNumeric)
{
    char *p = NULL;
    bool ret = CommonUtils::IsValidAlphaAndNumeric(p);
    EXPECT_FALSE(ret);
    char str[] = "";
    ret = CommonUtils::IsValidAlphaAndNumeric(str);
    EXPECT_FALSE(ret);
    char str2[] = "12abAB@#";
    ret = CommonUtils::IsValidAlphaAndNumeric(str2);
    EXPECT_FALSE(ret);
    char str3[] = "123abcABC";
    ret = CommonUtils::IsValidAlphaAndNumeric(str3);
    EXPECT_TRUE(ret);
}

TEST_F(CommonUtilsTest, Base64Encode)
{
    char str[] = "ManM";
    int len = strlen(str);
    string ret = CommonUtils::Base64Encode(str, len);
    EXPECT_EQ(ret, "TWFuTQ==");

    char str2[] = "Man";
    len = strlen(str2);
    ret = CommonUtils::Base64Encode(str2, len);
    EXPECT_EQ(ret, "TWFu");
}

TEST_F(CommonUtilsTest, JsonDump)
{
    json j = "ä\xA9ü";
    string ret = CommonUtils::JsonDump(j);
    EXPECT_TRUE(ret.empty());
}

TEST_F(CommonUtilsTest, IsUtf8String)
{
    string str = "\x20\xfc\x80\xfc";
    bool ret = CommonUtils::IsUtf8String(str);
    EXPECT_EQ(ret, false);
}

TEST_F(CommonUtilsTest, CreateFolderRecursively)
{
#ifdef _WIN32
    std::string folder = "D:/aaaa";
    bool ret = CommonUtils::CreateFolderRecursively(folder);
    rmdir(folder.c_str());
#else
    std::string folder = "/home/aaaa";
    bool ret = CommonUtils::CreateFolderRecursively(folder);
    rmdir(folder.c_str());
#endif
    EXPECT_EQ(ret, true);
}

TEST_F(CommonUtilsTest, HexToChar)
{
    string str = "\\x61";
    signed char res;
    bool ret = CommonUtils::HexToChar(str, res);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(res, 'a');
    str = "\\x6g";
    ret = CommonUtils::HexToChar(str, res);
    EXPECT_EQ(ret, false);
}

TEST_F(CommonUtilsTest, OctToChar)
{
    string str = "\\141";
    signed char res;
    bool ret = CommonUtils::OctToChar(str, res);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(res, 'a');
    str = "\\148";
    ret = CommonUtils::OctToChar(str, res);
    EXPECT_EQ(ret, false);
    str = "\\777";
    ret = CommonUtils::OctToChar(str, res);
    EXPECT_EQ(ret, false);
}

TEST_F(CommonUtilsTest, ReverseEscapeChar)
{
    signed char res;
    std::vector<string> escapeChar = { "\\0", "\\a", "\\b", "\\t", "\\n", "\\v",
                                       "\\f", "\\r", "\\\"", "\\\'", "\\\?", "\\\\"};
    for (const auto &esc : escapeChar) {
        bool ret = CommonUtils::ReverseEscapeChar(esc, res);
        EXPECT_EQ(ret, true);
    }
    string str = "\\aa";
    bool ret = CommonUtils::ReverseEscapeChar(str, res);
    EXPECT_EQ(ret, false);
}

TEST_F(CommonUtilsTest, GetCurrentPid)
{
    int pid = CommonUtils::GetCurrentPid();
    EXPECT_TRUE(pid > 0);
}

TEST_F(CommonUtilsTest, HexStringToUnsignedLongLong)
{
    string str = "  0x";
    uint64_t ull = 0;
    bool result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, false);
    EXPECT_EQ(ull, zeroValue);
    str = "qwer";
    result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, false);
    EXPECT_EQ(ull, zeroValue);
    str = "12345678901234567";
    result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, false);
    EXPECT_EQ(ull, zeroValue);
    str = "ff";
    result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, true);
    EXPECT_EQ(ull, static_cast<uint64_t>(CommonUtils::MAX_UNSIGNED_CHAR));
    str = "0x0000";
    result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, true);
    EXPECT_EQ(ull, zeroValue);
    str = "0x00000000000000000000000000000000ff";
    result = CommonUtils::HexStringToUnsignedLongLong(str, ull);
    EXPECT_EQ(result, true);
    EXPECT_EQ(ull, static_cast<uint64_t>(CommonUtils::MAX_UNSIGNED_CHAR));
}

TEST_F(CommonUtilsTest, ExecShellCmdWithResult)
{
    std::string command = "ipconfig";
    vector<string> results;
    bool ret = CommonUtils::ExecShellCmdWithResult(command, results);
    EXPECT_TRUE(ret);
}

TEST_F(CommonUtilsTest, Base64Decode)
{
    string base64 = "TWFuTQ==";
    string ret = CommonUtils::Base64Decode(base64);
    EXPECT_EQ(ret, "ManM");
    char str[] = "ax35./=+";
    int len = strlen(str);
    base64 = CommonUtils::Base64Encode(str, len);
    ret = CommonUtils::Base64Decode(base64);
    EXPECT_EQ(ret, "ax35./=+");
    base64 = "";
    ret = CommonUtils::Base64Decode(base64);
    EXPECT_EQ(ret, "");
    base64 = "qwe";
    ret = CommonUtils::Base64Decode(base64);
    EXPECT_EQ(ret, "");
    base64 = "<>,.";
    ret = CommonUtils::Base64Decode(base64);
    EXPECT_EQ(ret, "");
}

TEST_F(CommonUtilsTest, ConvertDriveLetterToUppercase)
{
    string filePath = "d:/CommonUtilsTest_RemoveFile.exe";
    string ret = CommonUtils::ConvertDriveLetterToUppercase(filePath);
    EXPECT_EQ(ret, "D:/CommonUtilsTest_RemoveFile.exe");
}

TEST_F(CommonUtilsTest, GetFilesByNumbers)
{
    std::vector<std::pair<std::string, std::int64_t>> vec = {
        {"1", 1}, {"2", secondFileTimestamp}, {"3", thirdFileTimestamp}
    };
    int number = 1;
    std::vector<std::string> vecStr;
    vecStr = CommonUtils::GetFilesByNumbers(vec, number);
    EXPECT_EQ(vecStr.size(), expectedFileCount);
}

TEST_F(CommonUtilsTest, GetAddressLength)
{
    string addr = "0x0123456789abcdef";
    int length = 0;
    length = CommonUtils::GetAddressLength(addr);
    EXPECT_EQ(length, CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU);

    addr = "0x01234567";
    length = CommonUtils::GetAddressLength(addr);
    EXPECT_EQ(length, CommonUtils::HEX_ADDRESS_WIDTH_32_BITS_CPU);

    addr = "0x0123";
    length = CommonUtils::GetAddressLength(addr);
    EXPECT_EQ(length, CommonUtils::HEX_ADDRESS_WIDTH_16_BITS_CPU);

    addr = "0123456789abcdef";
    length = CommonUtils::GetAddressLength(addr);
    EXPECT_EQ(length, -1);

    addr = "0x01";
    length = CommonUtils::GetAddressLength(addr);
    EXPECT_EQ(length, -1);
}

TEST_F(CommonUtilsTest, GetMaxValue)
{
    int length = CommonUtils::HEX_ADDRESS_WIDTH_64_BITS_CPU;
    int64_t value = 0;
    value = CommonUtils::GetMaxValidAddrValue(length);
    EXPECT_EQ(value, INT64_MAX);

    length = CommonUtils::HEX_ADDRESS_WIDTH_32_BITS_CPU;
    value = CommonUtils::GetMaxValidAddrValue(length);
    EXPECT_EQ(value, INT32_MAX);

    length = CommonUtils::HEX_ADDRESS_WIDTH_16_BITS_CPU;
    value = CommonUtils::GetMaxValidAddrValue(length);
    EXPECT_EQ(value, INT16_MAX);

    length = invalidAddressLength;
    value = CommonUtils::GetMaxValidAddrValue(length);
    EXPECT_EQ(value, -1);
}

TEST_F(CommonUtilsTest, GetEnv)
{
    string envName = "HOME";
    string ret = CommonUtils::GetEnv(envName);
#ifdef _WIN32
    EXPECT_EQ(ret, ret);
#endif

    envName = "OS";
    ret = CommonUtils::GetEnv(envName);
}
