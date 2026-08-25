// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelContainerThread.h"
#include "UvLoopMgr.h"

using namespace Dap;

ChannelContainerThread::ChannelContainerThread() : isStart(false) {}

ChannelContainerThread::~ChannelContainerThread()
{
    this->Stop();
}

ChannelContainerThread &ChannelContainerThread::Instance()
{
    static ChannelContainerThread instance;
    return instance;
}

void ChannelContainerThread::ThreadFunc()
{
    // just start
    ChannelContainer::Instance().Start();
}

bool ChannelContainerThread::Start()
{
    if (this->isStart) {
        return true;
    }
    threadPtr = std::make_unique<std::thread>(ChannelContainerThread::ThreadFunc);
    if (threadPtr == nullptr) {
        return false;
    }
    this->isStart = true;
    return true;
}

bool ChannelContainerThread::Stop()
{
    if (this->isStart) {
        ChannelContainer::Instance().Stop();
        UvLoopMgr::Instance().LoopStop(this->id);
        this->isStart = false;
    }
    if (threadPtr != nullptr && threadPtr->joinable()) {
        threadPtr->join();
    }
    return true;
}

bool ChannelContainerThread::IsStart()
{
    return this->isStart;
}
