// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <memory>
#ifdef CROSS_WINDOWS_FLAG
#include <unistd.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif
#include <gtest/gtest.h>
#include "CommonSuite.h"
#include "ProcessMonitor.h"

using namespace ::testing;
using namespace Dap;

class TestProcessMonitorHandler : public ProcessMonitorHandler {
public:
    void OnExited(int processId) override
    {
        (void)processId;
    }
};

class ProcessMonitorTest : public CommonSuite {
protected:
    void SetUp() override
    {
        monitor = std::make_unique<ProcessMonitor>();
        handler = std::make_shared<TestProcessMonitorHandler>();
    }
    void TearDown() override
    {
        monitor->Stop();
    }

protected:
    std::unique_ptr<ProcessMonitor> monitor = nullptr;
    std::shared_ptr<TestProcessMonitorHandler> handler = nullptr;
};

TEST_F(ProcessMonitorTest, start)
{
    monitor->Start();
    EXPECT_TRUE(monitor->IsStart());
}

TEST_F(ProcessMonitorTest, add_process)
{
    monitor->Start();
    EXPECT_TRUE(monitor->IsStart());
    int processId = -100;
    monitor->AddProcess(processId, handler);
    processId = getpid();
    monitor->AddProcess(processId, handler);
    const int runTime = 2000;
    std::this_thread::sleep_for(std::chrono::milliseconds(runTime));
}

TEST_F(ProcessMonitorTest, remove_process)
{
    monitor->Start();
    EXPECT_TRUE(monitor->IsStart());
    int processId = -100;
    monitor->AddProcess(processId, handler);
    processId = getpid();
    monitor->RemoveProcess(processId);
}