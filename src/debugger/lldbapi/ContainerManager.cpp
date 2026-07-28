// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ContainerManager.h"
#include "LLDB.h"
#include "CommonUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;
using namespace lldb;

lldb::SBValue ContainerManager::FindVarSBValue(const std::string &varName, int64_t &containerRef,
    SBDebugger &sbDebugger)
{
    SBValue value;
    ContainerInfo containerInfo = GetContainerInfo(containerRef);
    switch (containerInfo.containerType) {
        case DapContainer::LOCALS : {
            SBFrame frame = std::get<SBFrame>(containerInfo.container);
            value = frame.FindVariable(varName.c_str());
            break;
        }
        case DapContainer::STATICS :
        case DapContainer::GLOBALS : {
            SBFrame frame = std::get<SBFrame>(containerInfo.container);
            SBValueList valueList = frame.GetVariables(false, false, true, true);
#ifdef __APPLE__
            lldb::SBTypeCategory typeCategory = sbDebugger.GetCategory(lldb::eLanguageTypeC_plus_plus_14);
            typeCategory.SetEnabled(false);
#endif
            value = valueList.GetFirstValueByName(varName.c_str());
            break;
        }
        case DapContainer::VALUE : {
            constexpr int bracketPairLen = 2; // "[]"
            SBValue container = std::get<SBValue>(containerInfo.container);
            if (varName.length() > bracketPairLen && varName.at(0) == '[' && varName.back() == ']') {
                int index;
                if (CommonUtils::StringToInt(varName.substr(1, varName.length() - bracketPairLen), index)) {
                    value = container.GetChildAtIndex(index);
                    break;
                }
            }
            value = container.GetChildMemberWithName(varName.c_str());
            break;
        }
        default :
            break;
    }
    return value;
}

