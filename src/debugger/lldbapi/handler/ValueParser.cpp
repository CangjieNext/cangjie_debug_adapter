// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ValueParser.h"
#include "DapLog.h"
#include "LLDBApiDebugger.h"
#include "CommonUtils.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

ValueParser::ValueParser(const lldb::SBValue &value, std::shared_ptr<LLDBApiDebugger> debugger) : sbValue(value)
{
    debugger->UpdateCategory();
    languageType = debugger->GetLanguageType();
}

bool ValueParser::GetValue(string &result)
{
    if (!sbValue.IsValid() || sbValue.GetError().Fail()) {
        result = sbValue.GetError().GetCString();
        return false;
    }
    if (GetSimpleValue(result)) {
        return true;
    } else if (GetCompositeValue(result)) {
        return true;
    } else {
        result = "unknown";
        return true;
    }
}

bool ValueParser::GetSimpleValue(string &result)
{
    unsigned childrenNum = sbValue.GetNumChildren();
    if (childrenNum == 0) {
        if (sbValue.GetValueType() == lldb::eValueTypeRegister) {
            // Register default hexadecimal display
            return GetRegisterValue(result, UNRESOLVED_COMPOSITE_VALUE);
        }
        return GetValueSummary(result, false, UNRESOLVED_COMPOSITE_VALUE);
    } else if (sbValue.GetType().IsPointerType()) {
        return GetValueSummary(result, false, UNRESOLVED_COMPOSITE_VALUE);
    } else if (sbValue.GetType().IsArrayType()) {
        GetValueSummary(result, false, "");
        if (result.empty()) {
            result = "[" + std::to_string(childrenNum) + "]";
        }
        return true;
    } else {
        // Treat composite value which has registered summary (for example with AddCXXSummary) as simple value
        GetValueSummary(result, false, "");
        if (!result.empty()) {
            return true;
        }
    }

    // Composite variable type i.e. struct
    return false;
}

bool ValueParser::GetCompositeValue(string &result) const
{
    result = UNRESOLVED_COMPOSITE_VALUE;
    return true;
}

bool ValueParser::GetValueSummary(string &result, bool valueOnly, const string &failValue)
{
    if (!sbValue.IsValid()) {
        result = sbValue.GetError().GetCString();
        return false;
    }
    const char *charValue = sbValue.GetValue();
    if (valueOnly) {
        result =  charValue == nullptr ? failValue : charValue;
        return true;
    }

    SBTypeSummaryOptions summaryOptions;
    SBStream sbStream;
    summaryOptions.SetLanguage(languageType);
    // GetSummary() means eLanguageTypeUnknown, will return nullptr not "" when sbValue is child of std::map
    const char *charSummary = sbValue.GetSummary(sbStream, summaryOptions);
    string value;
    if (charValue) {
        value = charValue;
    } else if (charSummary == nullptr) {
        result = sbValue.GetNumChildren() > 0 ? UNRESOLVED_COMPOSITE_VALUE : "error: No value";
        return false;
    }
    string valSummary;
    if (charSummary && charSummary[0]) {
        valSummary = charSummary;
        SBTypeSummary summary = sbValue.GetTypeSummary();
        if (summary.IsValid() && summary.DoesPrintValue(sbValue) && !value.empty()) {
            valSummary.insert(0, value + " ");
        }
        result = valSummary;
        return true;
    } else if (GetCharPointeeSummary(result)) {
        result = value + " " + result;
        return true;
    }
    // no summary, return just value
    result = value;
    result = result.empty() ? failValue : result;
    return true;
}

bool ValueParser::GetRegisterValue(string &result, const string &failValue)
{
    if (!sbValue.IsValid()) {
        result = sbValue.GetError().GetCString();
        return false;
    }

    const char *charValue = sbValue.GetValue();
    SBTypeSummaryOptions summaryOptions;
    SBStream sbStream;
    summaryOptions.SetLanguage(languageType);
    const char *charSummary = sbValue.GetSummary(sbStream, summaryOptions);
    if (charSummary == nullptr) {
        charSummary = "";
    }
    if (charValue) {
        if (CommonUtils::StartWithString(charValue, "0x")) {
            result = charValue;
        } else {
            int64_t val = 0;
            bool strToLongLongResult = CommonUtils::StringToLongLong(charSummary, val);
            result = strToLongLongResult ?
                CommonUtils::Dec2HexAddress(val, sbValue.GetByteSize() * HEX_NUM_OF_BYTE) : charValue;
        }
    } else {
        result = charSummary;
    }
    result = result.empty() ? failValue : result;
    return true;
}

bool ValueParser::IsArrayType(lldb::SBValue value, DebuggeeLanguageType debuggeeLanguageType)
{
    if (debuggeeLanguageType == DebuggeeLanguageType::CANGJIE &&
        value.GetTypeName() != nullptr && CommonUtils::RegexSearch(value.GetTypeName(), "(\\S*)?(Array<.+?>$)$")) {
        return true;
    }
    return value.GetType().IsArrayType();
}

bool ValueParser::GetCharPointeeSummary(string &result)
{
    if (sbValue.GetTypeName() == nullptr) {
        return false;
    }
    string varType = sbValue.GetTypeName();
    // only for char *, such as signed char *, const unsigned char *
    if (!CommonUtils::EndWithString(varType, "char *")) {
        return false;
    }
    if (sbValue.GetValue() == nullptr) {
        return false;
    }
    string value = sbValue.GetValue();

    lldb::addr_t address;
    if (!CommonUtils::HexStringToUnsignedLongLong(value, address)) {
        return false;
    }
    if (!sbValue.GetProcess().IsValid()) {
        return false;
    }
    const int bufLen = 1024;
    //
    unique_ptr<char[]> buf = make_unique<char[]>(bufLen);
    SBError sbError;
    string bufContent;
    do {
        sbValue.GetProcess().ReadCStringFromMemory(address, buf.get(), bufLen, sbError);
        if (sbError.Success() && buf != nullptr) {
            bufContent = buf.get();
            result += bufContent;
        }
        // the length of "buf" from ReadCStringFromMemory is "bufLen" - 1
        address += bufLen - 1;
        // When '\0' is not read, buf is full, read cyclically
    } while (bufContent.size() == bufLen - 1);
    if (!result.empty()) {
        result = "\"" + result + "\"";
    }
    return true;
}
