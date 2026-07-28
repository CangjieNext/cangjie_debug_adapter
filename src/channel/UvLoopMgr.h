// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_UV_LOOP_MGR_H
#define DAP_UV_LOOP_MGR_H

#include <map>
#include <mutex>
#include <uv.h>

namespace Dap {
class UvLoopMgr {
public:
    static UvLoopMgr &Instance();

    uv_loop_t *Loop(const int &id = -1);
    bool LoopInit(const int &id = -1);
    void LoopStart(const int &id = -1);
    void LoopStop(const int &id = -1);
    bool IsLoopStart(const int &id = -1);
    bool IsLoopExist(const int &id = -1);
    void LoopRemove(const int &id = -1);  // internal api
    bool AllLoopStop();

private:
    UvLoopMgr();
    ~UvLoopMgr();

private:
    int defaultId;
    std::mutex loopMutex;
    std::map<int, uv_loop_t *> loopMap;
};
}

#endif // DAP_UV_LOOP_MGR_H
