// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelObjectId.h"

using namespace Dap;

ChannelObjectId &ChannelObjectId::ChannelId()
{
    static ChannelObjectId instance;
    return instance;
}

ChannelObjectId &ChannelObjectId::ChannelServerId()
{
    static ChannelObjectId instance;
    return instance;
}

ChannelObjectId &ChannelObjectId::ThreadId()
{
    static ChannelObjectId instance;
    return instance;
}

int ChannelObjectId::Create()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (id >= MAX_ID) {
        id = 0;
    }
    return id++;
}
