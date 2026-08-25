// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef _WIN32
#include "LinuxPlatform.h"
#include <vector>
#include <string>
#include "CommonUtils.h"
#include "DapLog.h"

using namespace std;
using namespace Dap;

std::string LinuxPlatform::GetShellNameByPID(int pid) const
{
    vector<string> res;
    string pidStr = std::to_string(pid);
    if (!CommonUtils::ExecShellCmdWithResult("ps -ax | grep " + pidStr, res)) {
        return "";
    }
    // e.g. result:  "21905 pts/106  Ss     0:00 /bin/bash"
    constexpr size_t shellInfoFieldCount = 2;
    for (const auto &result : res) {
        vector<string> split = CommonUtils::SplitString(result, " ");
        if (split.size() < shellInfoFieldCount) {
            continue;
        }
        if (split[0] == pidStr && split[1] != "?") {
            return "/dev/" + split[1];
        }
    }
    return "";
}

#endif // _WIN32
