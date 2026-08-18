// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "UvLoopMgr.h"
#include <cstdlib>

using namespace Dap;

UvLoopMgr::UvLoopMgr() : defaultId(-1)
{
    this->loopMap[defaultId] = uv_default_loop();
}

UvLoopMgr::~UvLoopMgr()
{
    this->AllLoopStop();
}

UvLoopMgr &UvLoopMgr::Instance()
{
    static UvLoopMgr instance;
    return instance;
}

uv_loop_t *UvLoopMgr::Loop(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        uv_loop_t *loop = uv_loop_new();
        if (loop == nullptr) {
            return nullptr;
        }
        loopMap[id] = loop;
    }
    return loopMap.at(id);
}

bool UvLoopMgr::LoopInit(const int &id)
{
    uv_loop_t *loop = Loop(id);
    if (loop == nullptr) {
        return false;
    }
    if (uv_loop_init(loop) != 0) {
        LoopRemove(id);
    }
    return true;
}

void UvLoopMgr::LoopStart(const int &id)
{
    uv_loop_t *loop = this->Loop(id); // if loop does not exist, new it
    if (loop != nullptr) {
        uv_run(loop, UV_RUN_DEFAULT);
    }
}

void UvLoopMgr::LoopStop(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        return;
    }
    uv_loop_t *loop = loopMap.at(id);
    if (loop == nullptr) {
        return;
    }
    uv_stop(loop);
    uv_loop_close(loop);
}

bool UvLoopMgr::AllLoopStop()
{
    std::lock_guard<std::mutex> lock(loopMutex);
    for (auto loopKv : loopMap) {
        uv_loop_t *loop = loopMap.at(loopKv.first);
        if (loop == nullptr) {
            continue;
        }
        if (uv_loop_alive(loop) != 0) {
            uv_stop(loop);
            uv_loop_close(loop);
        }
        free(loop);
    }
    loopMap.clear();
    return false;
}

bool UvLoopMgr::IsLoopStart(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        return false;
    }
    uv_loop_t *loop = this->loopMap.at(id);
    if (loop == nullptr) {
        return false;
    }
    return (loop->stop_flag == 0);
}

bool UvLoopMgr::IsLoopExist(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    return (loopMap.count(id) > 0);
}

void UvLoopMgr::LoopRemove(const int &id)
{
    if (id == defaultId) {
        return;
    }
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        return;
    }
    if (loopMap.at(id) != nullptr) {
        free(loopMap.at(id));
    }
    loopMap.erase(id);
}