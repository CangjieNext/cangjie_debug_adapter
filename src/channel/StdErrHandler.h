// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_STDERR_HANDLER_H
#define DAP_STDERR_HANDLER_H

namespace Dap {
class StdErrHandler {
public:
    virtual void OnStderrData(const int &channelId, const std::string &data) = 0;
};
}

#endif // DAP_STDERR_HANDLER_H
