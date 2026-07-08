// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "MultiMsgHandler.h"
#include "ResponseManager.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

void MultiMsgHandler::OnDebuggerResponse(const Dap::Protocol::Response &response)
{
    std::optional<json_t> curJson = ResponseManager::Instance().ToJson(response);
    if (!curJson.has_value()) {
        return;
    }
    json = curJson.value();
}

json_t MultiMsgHandler::GetJson()
{
    return json;
}
