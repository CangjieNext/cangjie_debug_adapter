// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <string>
#include <vector>
#include "Option.h"
#include "ChannelFactory.h"
#include "SocketServer.h"
#include "DebuggerHelper.h"
#include "DapLog.h"
#include "DapParamParser.h"
#include "CommonUtils.h"
#include "DumpUtils.h"

using namespace std;
using namespace Dap;

// enable dynamic_base compile option
#ifdef WIN32
__declspec(dllexport)
#endif
int main(int argc, const char *argv[])
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(std::string(argv[i]));
    }
    Option startOption;
    if (!DapParamParser::Instance().ParseDebugOption(args, startOption)) {
        LOG_INFO("Dap server parameters parse failed, return directly.");
        return -1;
    }
    // Start record dump information for windows
    DumpUtils::Instance().SetDumpFileDir(startOption.logPath).StartRecordDumpInfo();
    // delete obsolete logs
    CommonUtils::DeleteObsoleteLogs(startOption.logPath);
    // session
    const int waitMS = 20;
    std::shared_ptr<SocketServer> socketServer = std::make_shared<SocketServer>(startOption);
    if (socketServer == nullptr) {
        LOG_ERROR("ERROR: socket Server is nullptr!");
        return 0;
    }
    socketServer->Initialize();
    socketServer->Start();
    while (socketServer->IsStarted()) {
        if (startOption.debugSessionMode == DebugSessionMode::SINGLE && socketServer->AllSessionExited()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(waitMS));
    }
    Dap::DapLog::SetLogArchivedPermissions();
    return 0;
}