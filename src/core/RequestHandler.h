// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_REQUEST_HANDLER_H
#define DAP_SERVER_REQUEST_HANDLER_H

#include "Protocol.h"

namespace Dap {
class RequestHandler {
public:
    virtual void HandleRequest(std::unique_ptr<Dap::Protocol::Request> request) = 0;
};
}

#endif // DAP_SERVER_REQUEST_HANDLER_H
