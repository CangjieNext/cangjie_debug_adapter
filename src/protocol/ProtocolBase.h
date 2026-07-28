// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLBASE_H
#define DAP_PROTOCOLBASE_H

#include "defs.h"
#include "optional"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

struct ProtocolMessage {
    enum class Type {
        REQUEST,
        RESPONSE,
        EVENT,
        NONE
    };

    virtual ~ProtocolMessage() = default;
    unsigned int seq = 0;
    ProtocolMessage::Type type;
};

struct Request : public ProtocolMessage {
    explicit Request(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::REQUEST;
    }

    ~Request() override = default;
    const std::string command;
    // arguments will be placed into specified request
};

struct Response : public ProtocolMessage {
    explicit Response(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::RESPONSE;
    }
    ~Response() override = default;
    unsigned int request_seq = 0;
    bool success = false;
    std::string command;
    std::optional<std::string> message;
    // body will be placed into specified response
};

struct Event : public ProtocolMessage {
    explicit Event(const std::string &e) : event(e)
    {
        type = ProtocolMessage::Type::EVENT;
    }

    ~Event() override = default;
    std::string event;
    // body will be placed into specified event
};

    NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLBASE_H
