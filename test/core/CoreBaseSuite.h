// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_COREBASESUITE_H
#define DAP_SERVER_COREBASESUITE_H

#include <gtest/gtest.h>
#include "DebugSession.h"
#include "RequestManager.h"
#include "EventManager.h"
#include "ResponseManager.h"
#include "SocketServer.h"

class CoreBaseSuite : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
#endif // DAP_SERVER_COREBASESUITE_H

