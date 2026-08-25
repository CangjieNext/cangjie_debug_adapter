// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_COMMON_UTILS_H
#define DAP_COMMON_UTILS_H

#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <optional>
#include <json.hpp>

namespace Dap {
enum class TrimType : int {
    BOTH = 0,
    LEFT = 1,
    RIGHT = 2
};

#if defined __thumb__
inline constexpr int INSTRUCTION_BYTES = 2;
#elif (defined __aarch64__) || (defined __arm__)
inline constexpr int INSTRUCTION_BYTES = 4;
#else // cpu architecture with variable instruction length, like x86_64
inline constexpr int INSTRUCTION_BYTES = 8;
#endif

class CommonUtils {
public:
    // regex
    static bool RegexSearch(const std::string &data, const std::string &pattern);
    static bool RegexSearchIgnoreCase(const std::string &data, const std::string &pattern);
    static std::optional<std::smatch> RegexMatch(const std::string &data, const std::string &pattern);
    static bool RegexSearch(const std::string &data, std::smatch &results, const std::string &pattern);

    // file
    static bool IsFileExist(const std::string &filePath);
    static std::string GetFileName(const std::string &filePath);
    static bool RemoveFile(const std::string &filePath);
    static std::vector<std::string> GetFilesByNumbers(std::vector<std::pair<std::string, std::int64_t>> &restLog,
                                                      const int diffNums);
    static std::vector<std::string> GetFilesToBeDeleted(const std::string &folder, const std::string &start,
                                             const std::string &ext, const float &limitDays, const int &limitNums);
    static void DeleteObsoleteLogs(const std::string &logPath, const std::string &start = "dap_server",
                                   const std::string &ext = "log", const float &limitDays = limitDays,
                                   const int &limitFileNums = limitFileNumbers);

    // directory
    static bool IsFolderExist(const std::string &folderPath);
    static bool CreateFolder(const std::string &folderPath);
    static bool CreateFolderRecursively(const std::string &folderPath);
    static bool CheckFilePath(const std::string &filePath, std::string &errorMsg);
    static bool NormalizeFilePath(std::string &filePath, std::string &errorMsg);
    static std::string ConvertDriveLetterToUppercase(std::string filePath);
    static std::string GetEnv(const std::string &envName);
    static std::string FindLongestCommonPathPrefix(const std::string &pathStr1, const std::string &pathStr2);
    static std::string GetFilePathByPlatform(const std::string& filePath);

    // string
    static std::string ReplaceString(const std::string &src, const std::string &oldStr, const std::string &newStr);
    static std::string ReplaceStringEx(const std::string &src, const std::string &oldStr, const std::string &newStr);
    static bool StartWithString(const std::string &src, const std::string &start);
    static bool EndWithString(const std::string &src, const std::string &end);
    // trim string, type 0 - both, 1 - left, 2 - right
    static std::string TrimString(const std::string &src, const std::string &trim,
        const TrimType &type = TrimType::BOTH);

    // Verifying the Validity of a Numeric Character String
    // param isInt64:  true: int64, false: int32
    static bool IsValidInteger(const std::string &str, bool isInt64 = false);
    static bool IsValidHexInteger(const std::string &src, int maxLength = hex32BitsStringLength);
    // if the text is all valid alpha and numeric characters
    static bool IsValidAlphaAndNumeric(const char *pText);

    // Split a String to a vector by delimiter
    static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter,
        bool remainDeli = false);

    // converts strings to integers securely, if the conversion fails, the integer remains unchanged.
    static bool StringToInt(const std::string &str, int &integer);
    static bool StringToIntExt(const std::string &str, int &integer, int maxLength = hex32BitsStringLength);
    static bool StringToLongLong(const std::string &str, int64_t &integer);
    static bool HexStringToUnsignedLongLong(const std::string &src, uint64_t &ull);
    // converts HEX strings to char, if the conversion fails, the char remains unchanged.
    static bool HexToChar(const std::string &hex, signed char &res);
    // converts OCT strings to char, if the conversion fails, the char remains unchanged.
    static bool OctToChar(const std::string &oct, signed char &res);
    // converts escape Char strings to char, if the conversion fails, the char remains unchanged.
    static bool ReverseEscapeChar(const std::string &escapeStr, signed char &res);
    // translate integer from dec to hex
    static std::string Dec2HexAddress(unsigned long long i, int hexWidth = hexAddressWidth64BitsCpu,
                                      bool uppercase = false);
    static std::string FormatCharPointer(const char *input, int outputWidth,
                                         char outputFill = ' ', bool fillRight = false);

    // Generate random string with spesific length, include numbers and uppercase/lowercase letters.
    // Note: use rand() in this function, this string can not used in security domain.
    static std::string GenRandomString(int strLen);

    static int GetCurrentPid();
    // Execute shell command with result
    // Args:    command - shell command to execute.
    //          results - command result, Each element of the vector is a row of the result of the command execution.
    static bool ExecShellCmdWithResult(const std::string &command, std::vector<std::string> &results);

    // json dump
    static std::string JsonDump(const nlohmann::json &json);

    // utf-8 string
    static bool IsUtf8String(const std::string &str);

    static std::uint8_t ByteNum(const std::uint8_t &byte);

    static std::string Base64Encode(const char *inputBytes, unsigned int inputLen);
    // decode Base64 to string of char(ASCII)
    static std::string Base64Decode(const std::string &base64Code);
    // get the length of memory address
    static int GetAddressLength(const std::string &address);
    // get the max value of int64_t/int32_t/int16_t
    static int64_t GetMaxValidAddrValue(const int &length);
    // Anonymize sensitive information for harmony
    static std::string AnonymizeRemoteAddress(const std::string &remoteAddress);

    static constexpr int hexAddressWidth64BitsCpu = 16;
    static constexpr int hexAddressWidth32BitsCpu = 8;
    static constexpr int hexAddressWidth16BitsCpu = 4;
    static constexpr int hex64BitsStringLength = 16;
    static constexpr int hex32BitsStringLength = 8;
    static constexpr int BASE_16 = 16;
    static constexpr int maxUnsignedChar = 255;
    static constexpr int bufferSize = 1024;
    static constexpr float limitDays = 3.0f;
    static constexpr int limitFileNumbers = 500;

private:
    CommonUtils() = default;
    ~CommonUtils() = default;
};
}

#endif // DAP_COMMON_UTILS_H
