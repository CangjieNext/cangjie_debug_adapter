// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CoreBaseSuite.h"
#include "EventManager.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;

class SocketServerUT : public CoreBaseSuite {
protected:
    void SetUp() override
    {
        socketServer->Initialize();
    }
    void TearDown() override {}
protected:
    Option option;
    std::shared_ptr<SocketServer> socketServer = std::make_shared<SocketServer>(option);
};

TEST_F(SocketServerUT, OnChannelServerError)
{
    socketServer->OnChannelServerError(1, 1, "error");
}

TEST_F(SocketServerUT, OnSessionClosed)
{
    socketServer->OnSessionClosed(1);
}

TEST_F(SocketServerUT, Start)
{
    socketServer->Start();
}

TEST_F(SocketServerUT, Stop)
{
    socketServer->Stop();
}

