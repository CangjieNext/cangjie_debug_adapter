// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_SESSION_HANDLER_H
#define DAP_SERVER_SESSION_HANDLER_H

namespace Dap {
class SessionHandler {
public:
    SessionHandler() = default;
    virtual ~SessionHandler() = default;
    virtual void OnSessionClosed(int sessionID) = 0;
};
}

#endif // DAP_SERVER_SESSION_HANDLER_H
