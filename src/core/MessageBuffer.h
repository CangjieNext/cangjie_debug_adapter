// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_REQUEST_BUFFER_H
#define DAP_REQUEST_BUFFER_H

#include <mutex>
#include "ProtocolBase.h"

namespace Dap {
class MessageBuffer {
public:
    MessageBuffer();
    virtual ~MessageBuffer();
    MessageBuffer &operator >> (const std::string &data);
    virtual std::unique_ptr<Dap::Protocol::ProtocolMessage> Pop();

protected:
    int GetBodyLength(const int &pos, const int &len) const;
    Dap::Protocol::ProtocolMessage::Type GetMessageType(const std::string &body) const;

    std::string buffer;
    std::mutex mutex;
    std::string::size_type headPos = static_cast<std::string::size_type>(-1);
    int headLen = -1;
    int bodyPos = -1;
    int bodyLen = -1;
};
}

#endif // DAP_REQUEST_BUFFER_H
