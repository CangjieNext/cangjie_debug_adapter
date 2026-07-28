// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <regex>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#ifdef CROSS_WINDOWS_FLAG
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef _DARWIN
#include <crt_externs.h>
#endif
#endif
#include "DapLog.h"
#include "PlatformFactory.h"
#include "CommonUtils.h"

// Mac OS not support environ after X10.5, need to define environ as (*_NSGetEnviron())
#ifdef _DARWIN
#define environ (*_NSGetEnviron())
#endif

using namespace Dap;
using namespace std;

bool CommonUtils::RegexSearch(const string &data, const string &pattern)
{
    try {
        std::regex regex(pattern);
        if (!std::regex_search(data, regex)) {
            return false;
        }
        return true;
    } catch (const std::regex_error&) {
        return false;
    }
}

bool CommonUtils::RegexSearchIgnoreCase(const string &data, const string &pattern)
{
    try {
        std::regex regex(pattern);
        if (!std::regex_search(data, std::regex(pattern, regex::icase))) {
            return false;
        }
        return true;
    } catch (const std::regex_error&) {
        return false;
    }
}

std::optional<std::smatch> CommonUtils::RegexMatch(const string &data, const string &pattern)
{
    try {
        std::regex regex(pattern);
        std::smatch result;
        if (!std::regex_match(data, result, regex)) {
            return std::nullopt;
        }
        return result;
    } catch (const std::regex_error&) {
        return std::nullopt;
    }
}

bool CommonUtils::RegexSearch(const string &data, std::smatch &results, const string &pattern)
{
    try {
        std::regex regex(pattern);
        return std::regex_search(data, results, regex);
    } catch (const std::regex_error&) {
        return false;
    }
}

bool CommonUtils::IsFileExist(const string &filePath)
{
    struct stat st{};
    return (stat(filePath.c_str(), &st) == 0);
}

string CommonUtils::GetFileName(const string &filePath)
{
    string filename;
#ifdef _WIN32
    string filePathTmp = CommonUtils::ReplaceString(filePath, "/", "\\");
    filename = filePathTmp.substr(filePathTmp.find_last_of('\\') + 1);
#else
    filename = filePath.substr(filePath.find_last_of('/') + 1);
#endif
    return filename;
}

bool CommonUtils::RemoveFile(const std::string &filePath)
{
    struct stat st{};
    if (stat(filePath.c_str(), &st) != 0) {
        return true;
    }
    return (remove(filePath.c_str()) == 0);
}

std::vector<std::string> CommonUtils::GetFilesByNumbers(std::vector<std::pair<std::string, std::int64_t>> &restLog,
                                                        const int diffNums)
{
    std::vector<std::string> res;
    std::sort(restLog.begin(), restLog.end(),
              [=](std::pair<std::string, std::int64_t> &a,
              std::pair<std::string, std::int64_t> &b) {return a.second < b.second;});
    for (int i = 0; i < diffNums; i++) {
        res.push_back(restLog[i].first);
    }
    return res;
}

// Get the files that are more than 3 days first, then get files that if exceed the number limit
std::vector<std::string> CommonUtils::GetFilesToBeDeleted(const std::string &folder, const std::string &start,
                                                          const std::string &ext, const float &limitDays,
                                                          const int &limitFileNums)
{
    std::vector<std::string> res;
    std::vector<std::pair<std::string, std::int64_t>> restLog;
    time_t tt = system_clock::to_time_t(system_clock::now());
    std::int64_t standardTime = tt - static_cast<int>(limitDays * 24 * 60 * 60);
#ifdef _WIN32
    std::string folderTemp = folder + "\\*";
    _finddata_t findData;
    intptr_t handle = _findfirst(folderTemp.c_str(), &findData);
    bool findFile = (handle != -1);
    while (findFile) {
        std::string filename(findData.name);
        std::string filePath = folder + "\\" + filename;
        if (!(findData.attrib & _A_SUBDIR) && (filename != "." && filename != "..") &&
            StartWithString(filename, start) && EndWithString(filename, ext)) {
            if (standardTime > findData.time_create) {
                res.push_back(filePath);
            } else {
                restLog.push_back(std::make_pair(filePath, findData.time_create));
            }
        }
        findFile = _findnext(handle, &findData) == 0;
    }
    _findclose(handle);
#else
    DIR *dir = opendir(folder.c_str());
    if (dir == nullptr) {
        return res;
    }
    struct dirent *tmp = readdir(dir);
    while ((tmp = readdir(dir)) != nullptr) {
        std::string filename(tmp->d_name);
        std::string filePath = folder + "/" + filename;
        struct stat st;
        stat(filePath.c_str(), &st);
#ifdef _LINUX
        std::int64_t tmpTime = st.st_ctim.tv_sec;
#else
        std::int64_t tmpTime = st.st_ctimespec.tv_sec;
#endif
        if (!S_ISDIR(st.st_mode) && (filename != "." && filename != "..") && StartWithString(filename, start) &&
            EndWithString(filename, ext)) {
            if (standardTime > tmpTime) {
                res.push_back(filePath);
            } else {
                restLog.push_back(std::make_pair(filePath, tmpTime));
            }
        }
    }
    closedir(dir);
#endif
    std::vector<std::string> temp;
    if (restLog.size() > static_cast<size_t>(limitFileNums)) {
        int diff = static_cast<int>(restLog.size()) - limitFileNums;
        temp = CommonUtils::GetFilesByNumbers(restLog, diff);
        res.resize(res.size() + temp.size());
        res.insert(res.end(), temp.begin(), temp.end());
    }
    return res;
}

void CommonUtils::DeleteObsoleteLogs(const std::string &logPath, const std::string &start, const std::string &ext,
                                     const float &limitDays, const int &limitFileNums)
{
    if (limitDays <= 0.0f || (limitFileNums <= 0)) {
        return;
    }
    std::vector<std::string> logFiles = CommonUtils::GetFilesToBeDeleted(logPath, start, ext, limitDays, limitFileNums);
    for (auto logFile : logFiles) {
        CommonUtils::RemoveFile(logFile);
    }
}

bool CommonUtils::IsFolderExist(const string &folderPath)
{
    if (folderPath.empty()) {
        return false;
    }
    struct stat st{};
    return (stat(folderPath.c_str(), &st) == 0);
}

bool CommonUtils::CreateFolder(const string &folderPath)
{
    if (IsFolderExist(folderPath)) {
        return true;
    }
    int ret;
#ifdef _WIN32
    ret = mkdir(folderPath.c_str());
#else
    int permission = 0750;
    ret = mkdir(folderPath.c_str(), permission);
#endif
    return (ret == 0);
}

bool CommonUtils::CreateFolderRecursively(const std::string &folderPath)
{
    if (IsFolderExist(folderPath)) {
        return true;
    }
    std::string folder = folderPath;
    std::vector<std::string> pathVec;
#ifdef _WIN32
    folder = CommonUtils::ReplaceString(folder, "/", "\\");
    while (folder.find("\\\\") != std::string::npos) {
        folder = CommonUtils::ReplaceString(folder, "\\\\", "\\");
    }
    pathVec = CommonUtils::SplitString(folder, "\\", true);
    if ((pathVec.size() == 0) || !IsFolderExist(pathVec.at(0))) {
        return false;
    }
#else
    pathVec = CommonUtils::SplitString(folder, "/");
    if ((pathVec.size() == 0)) {
        return false;
    }
#endif
    std::string tmp = pathVec.at(0);
#ifndef _WIN32
    if (tmp == "~") {
        string home = CommonUtils::GetEnv("HOME");
        if (home.empty()) {
            LOG_ERROR("Unable to get HOME path!");
            return false;
        } else {
            tmp = home;
        }
    } else if ((tmp != ".") && (tmp != "..")) {
        tmp = "/" + tmp;
    }
#endif
    for (unsigned i = 1; i < pathVec.size(); i++) {
#ifdef _WIN32
        tmp += ("\\" + pathVec.at(i));
#else
        tmp += ("/" + pathVec.at(i));
#endif
        if (!CreateFolder(tmp)) {
            return false;
        }
    }
    return true;
}

string CommonUtils::ReplaceString(const string &src, const string &oldStr, const string &newStr)
{
    string tmp(src);
    std::size_t oldPos = 0U;
    std::size_t pos;
    while ((pos = tmp.find(oldStr, oldPos)) != string::npos) {
        tmp.replace(pos, oldStr.length(), newStr);
        oldPos = (pos + newStr.length());
    }
    return tmp;
}

string CommonUtils::ReplaceStringEx(const string &src, const string &oldStr, const string &newStr)
{
    if (src.empty()) {
        return src;
    }
    string tmp(src);
    std::size_t pos;
    while ((pos = tmp.find(oldStr, 0U)) != string::npos) {
        tmp.replace(pos, oldStr.length(), newStr);
    }
    return tmp;
}

bool CommonUtils::StartWithString(const std::string &src, const std::string &start)
{
    if (src.length() < start.length() || start.empty()) {
        return false;
    }
    std::string srcStart = src.substr(0, start.length());
    return (srcStart == start);
}

bool CommonUtils::EndWithString(const std::string &src, const std::string &end)
{
    if (src.length() < end.length() || end.empty()) {
        return false;
    }
    std::string srcEnd = src.substr(src.length() - end.length(), end.length());
    return (srcEnd == end);
}

std::string CommonUtils::TrimString(const std::string &src, const std::string &trim, const TrimType &type)
{
    if (trim.empty()) {
        return src;
    }
    std::string res = src;
    if ((type == TrimType::BOTH) || (type == TrimType::LEFT)) {
        while (StartWithString(res, trim)) {
            res = res.substr(trim.length());
        }
    }
    if ((type == TrimType::BOTH) || (type == TrimType::RIGHT)) {
        while (EndWithString(res, trim)) {
            res = res.substr(0, res.length() - trim.length());
        }
    }
    return res;
}

bool CommonUtils::IsValidInteger(const string &str, bool isInt64)
{
    std::regex match("-?[0-9]{1,}");
    bool isNumber = std::regex_match(str, match);
    if (!isNumber) {
        return false;
    }
    string max = isInt64 ? "9223372036854775807" : "2147483647";
    string min = isInt64 ? "9223372036854775808" : "2147483648";
    string cmp;
    std::string temp = str;
    // if start with "-", delete "-" and comparison to minimum
    if (StartWithString(temp, "-")) {
        temp = temp.substr(1);
        cmp = min;
    } else {
        // comparison to maximum
        cmp = max;
    }
    // if start with "0", delete "0"
    while (StartWithString(temp, "0")) {
        temp = temp.substr(1);
    }
    if (temp.size() < cmp.size()) {
        return true;
    } else if (temp.size() == cmp.size()) {
        // If the length is the same, compare the value.
        return temp.compare(cmp) <= 0;
    } else {
        return false;
    }
}

vector<string> CommonUtils::SplitString(const string &str, const string &delimiter, bool remainDeli)
{
    vector<string> result;
    if (str.empty() || delimiter.empty()) {
        return result;
    }

    string extendStr = str + delimiter;
    std::size_t pos;
    std::size_t size = extendStr.size();

    for (std::size_t currentPos = 0; currentPos < size; currentPos++) {
        pos = extendStr.find(delimiter, currentPos);
        if (pos != string::npos) {
            string s = extendStr.substr(currentPos, pos - currentPos);
            if (!s.empty() && remainDeli) {
                s += delimiter;
            }
            if (!s.empty()) {
                result.push_back(s);
            }
            currentPos = pos + delimiter.size() - 1;
        }
    }
    return result;
}

bool CommonUtils::StringToInt(const string &str, int &integer)
{
    if (!IsValidInteger(str)) {
        return false;
    }
    integer = std::stoi(str);
    return true;
}

bool CommonUtils::StringToIntExt(const std::string &str, int &integer, int maxLength)
{
    if (IsValidInteger(str)) {
        integer = std::stoi(str);
        return true;
    } else if (IsValidHexInteger(str, maxLength)) {
        integer = std::stoi(str, nullptr, BASE_16);
        return true;
    } else {
        return false;
    }
}

bool CommonUtils::StringToLongLong(const string &str, int64_t &integer)
{
    if (!IsValidInteger(str, true)) {
        return false;
    }
    integer = std::stoll(str);
    return true;
}

string CommonUtils::Dec2HexAddress(unsigned long long i, int hexWidth, bool uppercase)
{
    std::stringstream ss;
    if (uppercase) {
        ss << setiosflags(std::ios::uppercase);
    } else {
        ss << resetiosflags(std::ios::uppercase); // lowercase
    }
    if (i != 0) {
        ss << std::setw(hexWidth + strlen("0x"))  << std::setfill('0')
        << std::hex << std::showbase << std::internal << i;
    } else {
        ss << std::setw(hexWidth + strlen("0x")) << std::left << std::setfill('0') << "0x";
    }
    return ss.str();
}

string CommonUtils::FormatCharPointer(const char *input, int outputWidth, char outputFill, bool fillRight)
{
    std::stringstream ss;
    if (fillRight) {
        ss << std::left;
    }
    ss  << std::setw(outputWidth)  << std::setfill(outputFill)  << input;
    return ss.str();
}

string CommonUtils::GenRandomString(int strLen)
{
    if (strLen <= 0) {
        LOG_WARN("strLen = ", strLen, "return empty string !");
        return "";
    }
    string str(strLen, ' ');
    std::unique_ptr<Platform> platform = PlatformFactory::Generate();

    constexpr int charTypeNum = 3;
    constexpr int upperLetterType = 1;
    constexpr int lowerLetterType = 2;
    for (int i = 0; i < strLen; ++i) {
        switch ((platform->GenRandomUInt() % charTypeNum)) {
            case upperLetterType:
                str[i] = 'A' + platform->GenRandomUInt() % ('Z' - 'A' + 1);
                break;
            case lowerLetterType:
                str[i] = 'a' + platform->GenRandomUInt() % ('z' - 'a' + 1);
                break;
            default:
                str[i] = '0' + platform->GenRandomUInt() % ('9' - '0' + 1);
                break;
        }
    }
    return str;
}

bool CommonUtils::CheckFilePath(const std::string &filePath, std::string &errorMsg)
{
    if (filePath.empty()) {
        errorMsg = "path is empty.";
        return false;
    }
#ifdef _WIN32
    std::string fileName = CommonUtils::GetFileName(filePath);
    if (CommonUtils::RegexSearch(fileName, "[\\/:*?\"<>|]")) {
        errorMsg = "path has illegal characters.";
        return false;
    }
#endif
    if (!CommonUtils::IsFileExist(filePath)) {
        errorMsg = "path does not exist." + filePath;
        return false;
    }
    return true;
}

int CommonUtils::GetCurrentPid()
{
    return static_cast<int>(getpid());
}

std::string CommonUtils::JsonDump(const nlohmann::json &json)
{
    const int indendedNum = 4;
    std::string res;
    try {
        res = json.dump(indendedNum);
    } catch (nlohmann::json::type_error &err) {
        LOG_ERROR("json dump type error, what: ", err.what());
    } catch (nlohmann::json::parse_error &err) {
        LOG_ERROR("json dump parse error, what: ", err.what());
    } catch (nlohmann::json::other_error &err) {
        LOG_ERROR("json dump other error, what: ", err.what());
    } catch (...) {
        LOG_ERROR("json dump unknown error.");
    }
    return res;
}

std::uint8_t CommonUtils::ByteNum(const std::uint8_t &byte)
{
    const static std::map<std::uint8_t, std::uint8_t> mapBytes = { { 0xE0, 0xC0 },
                                                                   { 0xF0, 0xE0 },
                                                                   { 0xF8, 0xF0 },
                                                                   { 0xFC, 0xF8 },
                                                                   { 0xFE, 0xFC } };
    int index = 0;
    int byteNum = 0;
    for (auto kv : mapBytes) {
        index++;
        if ((byte & kv.first) == kv.second) {
            byteNum = index;
            break;
        }
    }
    return byteNum;
}

bool CommonUtils::IsUtf8String(const std::string &str)
{
    std::uint32_t byteNum = 0;
    std::uint8_t byte;
    for (int i = 0; str[i] != '\0'; i++) {
        byte = static_cast<std::uint8_t>(str[i]);
        // ASCII
        if ((byteNum == 0) && ((byte & 0x80) == 0)) {
            continue;
        }
        // UTF8 first byte
        if (byteNum == 0) {
            byteNum = ByteNum(byte);
            if (byteNum == 0) {
                return false;
            }
        } else { // UTF8 other bytes
            if ((byte & 0xC0) != 0x80) {
                return false;
            }
            byteNum--;
        }
    }
    return true;
}

// Normalize the path format and organize the path format into the normal path format.
bool CommonUtils::NormalizeFilePath(string &filePath, string &errorMsg)
{
#ifdef _WIN32
    filePath = CommonUtils::ReplaceString(filePath, "/", "\\");
#endif
    if (filePath.length() > PATH_MAX) {
        errorMsg = "path is too long!";
        return false;
    }
    char realPath[PATH_MAX + 1] = { 0x00 };
#ifdef _WIN32
    if (PathCanonicalize(realPath, filePath.c_str()) == FALSE) {
        errorMsg = "path is illegal!";
        return false;
    }
#else
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        errorMsg = "path does not exist or is illegal!";
        return false;
    }
#endif
    filePath = std::string(realPath);
    return true;
}

bool CommonUtils::IsValidAlphaAndNumeric(const char *pText)
{
    if (pText == nullptr) {
        return false;
    }
    const size_t len = ::strlen(pText);
    if (len == 0) {
        return false;
    }

    for (size_t i = 0; i < len; i++, pText++) {
        const char c = *pText;
        if (::isalnum(c) == 0) {
            return false;
        }
    }

    return true;
}

string CommonUtils::Base64Encode(const char *inputBytes, unsigned int inputLen)
{
    constexpr int inputCharsGroupLen = 3;
    constexpr int outputBase64GroupLen = 4;
    constexpr int shiftBits2 = 2;
    constexpr int shiftBits4 = 4;
    constexpr int shiftBits6 = 6;
    int count = 0;
    string result;
    unsigned char in3CharsGroup[3];  // 3 charactors to encode
    unsigned char out4Base64Index[4];  // indexes of 4 encoded characters
    const std::string base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    while (inputLen > 0) {
        in3CharsGroup[count++] = *(inputBytes++);  // get three bytes (24 bits)
        if (count == inputCharsGroupLen) {
            // eg: 01001101, 01100001, 01101110 --> 010011, 010110, 000101, 101110
            out4Base64Index[0] = in3CharsGroup[0] >> shiftBits2;
            out4Base64Index[1] = ((in3CharsGroup[0] & 0x03) << shiftBits4) + (in3CharsGroup[1] >> shiftBits4);
            out4Base64Index[2] = ((in3CharsGroup[1] & 0x0f) << shiftBits2) + (in3CharsGroup[2] >> shiftBits6);
            out4Base64Index[3] = in3CharsGroup[2] & 0x3f;

            for (int j = 0; j < outputBase64GroupLen; j++) {
                result += base64Table[out4Base64Index[j]];
            }
            count = 0;
        }
        inputLen--;
    }

    if (count > 0) {
        for (int j = count; j < inputCharsGroupLen; j++) {
            in3CharsGroup[j] = '\0';
        }
        out4Base64Index[0] = in3CharsGroup[0] >> shiftBits2;
        out4Base64Index[1] = ((in3CharsGroup[0] & 0x03) << shiftBits4) + (in3CharsGroup[1] >> shiftBits4);
        out4Base64Index[2] = ((in3CharsGroup[1] & 0x0f) << shiftBits2) + (in3CharsGroup[2] >> shiftBits6);
        for (int j = 0; j < (count + 1); j++) {
            result += base64Table[out4Base64Index[j]];
        }

        while (count < inputCharsGroupLen) {
            result += '=';
            count++;
        }
    }
    return result;
}

string CommonUtils::Base64Decode(const string &base64Code)
{
    constexpr int inputBase64GroupLen = 4;
    size_t dataLength = base64Code.length();
    if (base64Code.empty() || dataLength % inputBase64GroupLen != 0) {
        return "";
    }
    regex match("^[a-zA-Z0-9+/]+={0,2}$");
    if (!regex_match(base64Code.substr(dataLength - inputBase64GroupLen, inputBase64GroupLen), match)) {
        return "";
    }
    constexpr int sectionLength = 1000;
    size_t position = 0;
    regex sectionMatch("^([A-Za-z0-9+/]{4})*$");
    string sections = base64Code.substr(0, base64Code.length() - inputBase64GroupLen);
    while (position < dataLength - inputBase64GroupLen) {
        if (!regex_match(sections.substr(position, sectionLength), sectionMatch)) {
            return "";
        }
        position = position + sectionLength;
    }
    constexpr int shiftBits2 = 2;
    constexpr int shiftBits4 = 4;
    constexpr int shiftBits6 = 6;
    const std::string base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t currentPos = 0;
    std::string result;
    unsigned char in4Base64Index[4];  // indexes of 4 encoded characters
    while (currentPos < dataLength) {
        in4Base64Index[0] = base64Table.find(base64Code[currentPos]);
        in4Base64Index[1] = base64Table.find(base64Code[currentPos + 1]);
        in4Base64Index[2] = base64Code[currentPos + 2] != '=' ? base64Table.find(base64Code[currentPos + 2]) : 0;
        in4Base64Index[3] = base64Code[currentPos + 3] != '=' ? base64Table.find(base64Code[currentPos + 3]) : 0;
        result.push_back((in4Base64Index[0] << shiftBits2) + ((in4Base64Index[1] & 0x30) >> shiftBits4));
        if (base64Code[currentPos + 2] == '=') {
            break;
        }
        result.push_back(((in4Base64Index[1] & 0x0f) << shiftBits4) + ((in4Base64Index[2] & 0x3c) >> shiftBits2));
        if (base64Code[currentPos + 3] == '=') {
            break;
        }
        result.push_back(((in4Base64Index[2] & 0x03) << shiftBits6) + in4Base64Index[3]);
        currentPos += inputBase64GroupLen;
    }
    return result;
}

bool CommonUtils::HexToChar(const string &hex, signed char &res)
{
    std::regex match("\\\\x[0-9,a-f,A-F]{1,2}");
    bool isHexChar = std::regex_match(hex, match);
    if (!isHexChar) {
        return false;
    }
    string tmp = hex.substr(2);
    std::stringstream ss;
    ss << std::hex << tmp;
    unsigned int dec;
    ss >> dec;
    res = static_cast<char>(dec);
    return true;
}

bool CommonUtils::OctToChar(const string &oct, signed char &res)
{
    std::regex match("\\\\[0-7]{1,3}");
    bool isOctChar = std::regex_match(oct, match);
    if (!isOctChar) {
        return false;
    }
    string tmp = oct.substr(1);
    std::stringstream ss;
    ss << std::oct << tmp;
    unsigned int dec;
    ss >> dec;
    if (dec > MAX_UNSIGNED_CHAR) {
        return false;
    }
    res = static_cast<char>(dec);
    return true;
}

bool CommonUtils::ReverseEscapeChar(const string &escapeStr, signed char &res)
{
    std::map<string, char> escapeCharMap = { {"\\0", '\0'},
                                             {"\\a", '\a'},
                                             {"\\b", '\b'},
                                             {"\\t", '\t'},
                                             {"\\n", '\n'},
                                             {"\\v", '\v'},
                                             {"\\f", '\f'},
                                             {"\\r", '\r'},
                                             {"\\\"", '\"'},
                                             {"\\\'", '\''},
                                             {"\\\?", '\?'},
                                             {"\\\\", '\\'}, };
    if (escapeCharMap.find(escapeStr) == escapeCharMap.end()) {
        return false;
    }
    res = escapeCharMap[escapeStr];
    return true;
}

// G.FUU.16: forbidden the popen() function unless it sis hard-coded input.
// para "command" must be string literal.
bool CommonUtils::ExecShellCmdWithResult(const string &command, vector<string> &results)
{
    LOG_INFO("execute shell command:", command);
    char buffer[BUFFER_SIZE];
    FILE *pFile = popen(command.c_str(), "r");
    if (!pFile) {
        return false;
    }
    while (fgets(buffer, BUFFER_SIZE, pFile)) {
        string tmp = buffer;
        // delete '\n' that automatically added by fgets()
        tmp = tmp.substr(0, tmp.length() - 1);
        results.push_back(tmp);
    }
    pclose(pFile);
    return true;
}

bool CommonUtils::HexStringToUnsignedLongLong(const string &src, uint64_t &ull)
{
    if (!IsValidHexInteger(src, HEX_64_BITS_STRING_LENGTH)) {
        return false;
    }
    ull = stoull(src, nullptr, BASE_16);
    return true;
}

bool CommonUtils::IsValidHexInteger(const string &src, int maxLength)
{
    string hexStr = TrimString(src, " ");
    hexStr = TrimString(hexStr, "0x", TrimType::LEFT);
    if (hexStr.empty()) {
        return false;
    }
    hexStr = TrimString(hexStr, "0", TrimType::LEFT);
    regex match("[a-fA-F0-9]*");
    if (!regex_match(hexStr, match)) {
        return false;
    }
    if (hexStr.length() > static_cast<size_t>(maxLength)) {
        return false;
    }
    return true;
}

std::string CommonUtils::ConvertDriveLetterToUppercase(std::string filePath)
{
    if (isalpha(filePath[0]) && filePath[1] == ':' &&
        (filePath[2] == '/' || filePath[2] == '\\')) {
        // file path on Windows
        filePath[0] = toupper(filePath[0]);
    }
    return filePath;
}

std::string CommonUtils::GetEnv(const std::string &envName)
{
    for (int i = 0;; i++) {
        // "environ" is from stdlib.h, it contains all environment variables
        if (environ[i] == nullptr) {
            break;
        }
        std::string item = environ[i];
        auto pos = item.find('=');
        if (pos != std::string::npos && item.substr(0, pos) == envName) {
            return item.substr(pos + 1);
        }
    }
    return "";
}

std::string CommonUtils::FindLongestCommonPathPrefix(const std::string &pathStr1, const std::string &pathStr2)
{
    std::string longestCommonPathPrefix;
    for (size_t i = 0; i < std::min(pathStr1.length(), pathStr2.length()); i++) {
        if (pathStr1[i] == pathStr2[i]) {
            longestCommonPathPrefix.push_back(pathStr1[i]);
        } else {
            break;
        }
    }
    return longestCommonPathPrefix;
}

std::string CommonUtils::GetFilePathByPlatform(const string &filePath)
{
#ifdef _WIN32
    return CommonUtils::ReplaceString(filePath, "/", "\\");
#else
    return CommonUtils::ReplaceString(filePath, "\\", "/");
#endif
}

int CommonUtils::GetAddressLength(const std::string &address)
{
    // The address format defaults to the standard format, which means if the number of address bits is not enough,
    // it will be automatically filled with 0 on the left.
    // eg: 0x0000000000402d5b(64 bits CPU)\0x00002996(32 bits CPU)
    if (StartWithString(address, "0x")) {
        string tempAddr = TrimString(address, "0x", TrimType::LEFT);
        int length = static_cast<int>(tempAddr.size());
        switch (length) {
            case HEX_ADDRESS_WIDTH_64_BITS_CPU:
                return HEX_ADDRESS_WIDTH_64_BITS_CPU;
            case HEX_ADDRESS_WIDTH_32_BITS_CPU:
                return HEX_ADDRESS_WIDTH_32_BITS_CPU;
            case HEX_ADDRESS_WIDTH_16_BITS_CPU:
                return HEX_ADDRESS_WIDTH_16_BITS_CPU;
            default:
                return -1;
        }
    }
    return -1;
}

int64_t CommonUtils::GetMaxValidAddrValue(const int &length)
{
    switch (length) {
        case HEX_ADDRESS_WIDTH_64_BITS_CPU:
            return INT64_MAX;
        case HEX_ADDRESS_WIDTH_32_BITS_CPU:
            return INT32_MAX;
        case HEX_ADDRESS_WIDTH_16_BITS_CPU:
            return INT16_MAX;
        default:
            return -1;
    }
}

// The remoteAddress field in the AttachRequest contains the device SN and application bundle name, for example
// "remoteAddress": "unix-abstract-connect://[${SN}]/${bundle name}/platform-1728182323995.sock"
std::string CommonUtils::AnonymizeRemoteAddress(const string &remoteAddress)
{
    string res = remoteAddress;
    if (CommonUtils::StartWithString(remoteAddress, "unix-abstract-connect:") &&
        CommonUtils::EndWithString(remoteAddress, ".sock")) {
        res = res.replace(res.find("//"), res.rfind("/") - res.find("//"), "//[${SN}]/${bundle name}");
    }
    return res;
}
