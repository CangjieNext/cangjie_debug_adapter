// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_VALUEPARSER_H
#define DAP_SERVER_VALUEPARSER_H

#include <memory>
#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
class LLDBApiDebugger;
class ValueParser {
public:
    ValueParser(const lldb::SBValue &value, std::shared_ptr<LLDBApiDebugger> debugger);
    ~ValueParser() = default;
    bool GetValue(std::string &result);
    static bool IsArrayType(lldb::SBValue value, DebuggeeLanguageType debuggeeLanguageType);

protected:
    bool GetSimpleValue(std::string &result);
    bool GetCompositeValue(std::string &result) const;
    bool GetValueSummary(std::string &result, bool valueOnly, const std::string &failValue);
    // get string content of the char pointer, now used for signed char pointer
    bool GetCharPointeeSummary(std::string &result);
    bool GetRegisterValue(std::string &result, const std::string &failValue);

protected:
    const std::string UNKNOWN_VALUE = "??";
    const std::string UNRESOLVED_COMPOSITE_VALUE = "{...}";

private:
    lldb::SBValue sbValue;
    lldb::LanguageType languageType;
    constexpr static int HEX_NUM_OF_BYTE = 2;
};
};

#endif // DAP_SERVER_VALUEPARSER_H