// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ChannelServerThread.h"
#include "UvLoopMgr.h"

using namespace Dap;

ChannelServerThread::ChannelServerThread(const std::shared_ptr<ChannelServer> channelServerPtr)
    : isStart(false), channelServerPtr(channelServerPtr)
{
    this->channelServerPtr->SetLoopId(id);
    UvLoopMgr::Instance().LoopInit(id);
}

ChannelServerThread::~ChannelServerThread()
{
    this->Stop();
}

void ChannelServerThread::ThreadFunc(ChannelServerThread &ct)
{
    if ((ct.channelServerPtr == nullptr) || ct.channelServerPtr->IsStart() || !ct.channelServerPtr->Start()) {
        ct.resultCv.notify_one();
        return;
    }
    ct.isStart = true;
    ct.resultCv.notify_one();
    UvLoopMgr::Instance().LoopStart(ct.id);
}

bool ChannelServerThread::Start()
{
    if (this->isStart) {
        return true;
    }
    threadPtr = std::make_unique<std::thread>(ChannelServerThread::ThreadFunc, std::ref(*this));
    if (threadPtr == nullptr) {
        return false;
    }
    threadPtr->detach();
    std::unique_lock<std::mutex> lock(resultMutex);
    resultCv.wait_for(lock, std::chrono::milliseconds(resultTimeout), [&] { return this->isStart; });
    // wait for notification from resultCv
    return this->isStart;
}

bool ChannelServerThread::Stop()
{
    if (!this->isStart) {
        return true;
    }
    if (channelServerPtr == nullptr) {
        return false;
    }
    channelServerPtr->Stop();
    UvLoopMgr::Instance().LoopStop(this->id);
    this->isStart = false;
    return true;
}

bool ChannelServerThread::IsStart() const
{
    return this->isStart;
}

int ChannelServerThread::ThreadId() const
{
    return this->id;
}
int ChannelServerThread::ChannelServerId() const
{
    if (this->channelServerPtr == nullptr) {
        return -1;
    }
    return this->channelServerPtr->Id();
}

std::shared_ptr<ChannelServer> ChannelServerThread::GetChannelServer() const
{
    return this->channelServerPtr;
}

std::shared_ptr<Channel> ChannelServerThread::GetChannel(const int &channelId) const
{
    if (this->channelServerPtr == nullptr) {
        return nullptr;
    }
    return this->channelServerPtr->GetChannel(channelId);
}