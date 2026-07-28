// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "MessageBuffer.h"
#include <regex>
#include "CommonUtils.h"
#include "DapLog.h"
#include "RequestManager.h"
#include "ResponseManager.h"

using namespace Dap;

namespace {
const std::string REQ_DELIMITER = "\r\n\r\n";
const std::string HEAD_START = "Content-Length:";
const std::string BODY_START = "{";
const std::string BODY_END = "}";
const int MATCH_MIN_NUM = 2;
const int DUMP_NUM = 4;
}

MessageBuffer::MessageBuffer() {}

MessageBuffer::~MessageBuffer()
{
    buffer.clear();
}

int MessageBuffer::GetBodyLength(const int &pos, const int &len) const
{
    std::string lenStr = buffer.substr(pos, len);
    LOG_INFO("Head length string: ", lenStr);
    std::optional<std::smatch> matchRes = CommonUtils::RegexMatch(lenStr, "Content-Length:\\s*(\\d+)");
    if (!matchRes.has_value() || matchRes.value().size() < MATCH_MIN_NUM) {
        return -1;
    }
    int res;
    try {
        res = std::stoi(matchRes.value()[1].str());
    } catch (std::invalid_argument&) {
        res = -1;
    } catch (std::out_of_range&) {
        res = -1;
    } catch (...) {
        res = -1;
    }
    LOG_INFO("Get body length from head = ", res);
    return res;
}

Dap::Protocol::ProtocolMessage::Type MessageBuffer::GetMessageType(const std::string &body) const
{
    json_t json;
    try {
        json = json_t::parse(body);
    } catch (const json_t::parse_error &e) {
        LOG_ERROR("Occurs an exception when parse body string, body string:\n", body);
        return Dap::Protocol::ProtocolMessage::Type::NONE;
    }
    if (!json.contains("type")) {
        LOG_ERROR("Body is not dap message, body string:\n", body);
        return Dap::Protocol::ProtocolMessage::Type::NONE;
    }
    // Anonymize sensitive information for harmony
    if (!json["arguments"]["remoteAddress"].is_null()) {
        json["arguments"]["remoteAddress"] = CommonUtils::AnonymizeRemoteAddress(json["arguments"]["remoteAddress"]);
    }
    LOG_INFO("Body string: \n", json.dump(DUMP_NUM));
    if (json["type"] == Dap::Protocol::REQUEST_NAME) {
        return Dap::Protocol::ProtocolMessage::Type::REQUEST;
    } else if (json["type"] == Dap::Protocol::RESPONSE_NAME) {
        return Dap::Protocol::ProtocolMessage::Type::RESPONSE;
    } else if (json["type"] == Dap::Protocol::EVENT_NAME) {
        return Dap::Protocol::ProtocolMessage::Type::EVENT;
    } else {
        return Dap::Protocol::ProtocolMessage::Type::NONE;
    }
}

MessageBuffer &MessageBuffer::operator >> (const std::string &data)
{
    std::unique_lock<std::mutex> lock(mutex);
    buffer.append(data);
    return *this;
}

std::unique_ptr<Dap::Protocol::ProtocolMessage> MessageBuffer::Pop()
{
    std::unique_lock<std::mutex> lock(mutex);
    headPos = buffer.find(HEAD_START);
    if (headPos == std::string::npos) {
        return nullptr;
    }
    if (headPos != 0) {
        buffer = buffer.substr(headPos);
        headPos = 0;
    }
    std::string::size_type splitPos = buffer.find(REQ_DELIMITER);
    if (splitPos == std::string::npos) {
        return nullptr;
    }
    headLen = static_cast<int>(splitPos - headPos);
    bodyLen = GetBodyLength(headPos, headLen);
    if (bodyLen == -1) {
        LOG_ERROR("The value of \"Content-Length\" is invalid!");
        buffer = buffer.substr(splitPos + REQ_DELIMITER.length());
        return nullptr;
    }
    bodyPos = splitPos + REQ_DELIMITER.length();
    LOG_INFO("Head length = ", headLen, ", body length = ", bodyLen);
    // Prevent integer overflow
    std::string::size_type msgLen = static_cast<std::string::size_type>(headLen) + REQ_DELIMITER.length() +
                                    static_cast<std::string::size_type>(bodyLen);
    if (buffer.length() < msgLen) {
        LOG_WARN("The message is incomplete, waiting...");
        return nullptr;
    }
    std::string bodyStr = buffer.substr(bodyPos, bodyLen);
    buffer = buffer.substr(bodyPos + bodyLen); // buffer removes head and body string
    Dap::Protocol::ProtocolMessage::Type msgType = GetMessageType(bodyStr);
    if (msgType == Dap::Protocol::ProtocolMessage::Type::REQUEST) {
        std::unique_ptr<Dap::Protocol::Request> req = Dap::Protocol::RequestManager::Instance().FromJson(bodyStr);
        return std::unique_ptr<Dap::Protocol::ProtocolMessage>(req.release());
    } else if (msgType == Dap::Protocol::ProtocolMessage::Type::RESPONSE) {
        std::unique_ptr<Dap::Protocol::Response> res = Dap::Protocol::ResponseManager::Instance().ToResponse(bodyStr);
        return std::unique_ptr<Dap::Protocol::ProtocolMessage>(res.release());
    } else {
        LOG_INFO("Pop useless message.");
    }
    return nullptr;
}