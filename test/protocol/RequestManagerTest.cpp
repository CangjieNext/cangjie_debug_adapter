// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "RequestManager.h"
#include "EnumUtils.h"
#include "ProtocolSuite.h"

using namespace Dap::Protocol;

class RequestManagerTest : public ProtocolSuite {
    void SetUp() override
    {
        RequestManager::Instance().Register();
        RequestManager::Instance().Register();
        EXPECT_TRUE(RequestManager::Instance().IsRegister());
    }
    void TearDown() override
    {
        RequestManager::Instance().UnRegister();
        RequestManager::Instance().UnRegister();
    }

protected:
    int arraySize = 10;
};

TEST_F(RequestManagerTest, notRequestTest)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::RESPONSE).value();
    json["seq"] = 1;
    json["command"] = "initialize ";
    json["arguments"]["adapterID"] = "adapter";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, findFuncError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "initialize ";
    json["arguments"]["adapterID"] = "adapter";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    Dap::Protocol::InitializeRequest* ptr = dynamic_cast<Dap::Protocol::InitializeRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, parseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "initialize";
    json["arguments"]["adapterID"] = "adapter";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump() + "}");
    Dap::Protocol::InitializeRequest* ptr = dynamic_cast<Dap::Protocol::InitializeRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, initializeRequestTest_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "initialize";
    json["arguments"]["adapterID"] = "adapter";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    Dap::Protocol::InitializeRequest* ptr = dynamic_cast<Dap::Protocol::InitializeRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(Dap::Protocol::ENUM_TO_STR(ptr->type).value(), "request");
    EXPECT_EQ(ptr->command, "initialize");
    EXPECT_EQ(ptr->arguments.adapterID, "adapter");
}

TEST_F(RequestManagerTest, initializeRequestTest_errorCast_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "configurationDone";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::ConfigurationDoneRequest* configurationDoneRequestPtr = dynamic_cast
        <Dap::Protocol::ConfigurationDoneRequest*>(requestPtr.get());
    Dap::Protocol::InitializeRequest* initializeRequestPtr = dynamic_cast<Dap::Protocol::InitializeRequest*>
        (requestPtr.get());
    Dap::Protocol::LaunchRequest* launchRequestPtr = dynamic_cast<Dap::Protocol::LaunchRequest*>(requestPtr.get());
    Dap::Protocol::DisconnectRequest* disconnectRequestPtr = dynamic_cast<Dap::Protocol::DisconnectRequest*>
        (requestPtr.get());
    Dap::Protocol::ContinueRequest* continueRequestPtr = dynamic_cast<Dap::Protocol::ContinueRequest*>
        (requestPtr.get());
    Dap::Protocol::BreakpointLocationsRequest* breakpointLocationsRequestPtr = dynamic_cast
        <Dap::Protocol::BreakpointLocationsRequest*>(requestPtr.get());
    Dap::Protocol::SetBreakpointsRequest* setBreakpointsRequestPtr = dynamic_cast<Dap::Protocol::SetBreakpointsRequest*>
        (requestPtr.get());
    Dap::Protocol::TerminateRequest* terminateRequestPtr = dynamic_cast<Dap::Protocol::TerminateRequest*>
        (requestPtr.get());
    Dap::Protocol::ThreadsRequest* threadsRequestPtr = dynamic_cast<Dap::Protocol::ThreadsRequest*>(requestPtr.get());

    EXPECT_FALSE(requestPtr == nullptr);
    EXPECT_FALSE(configurationDoneRequestPtr == nullptr);
    EXPECT_TRUE(initializeRequestPtr == nullptr);
    EXPECT_TRUE(launchRequestPtr == nullptr);
    EXPECT_TRUE(disconnectRequestPtr == nullptr);
    EXPECT_TRUE(continueRequestPtr == nullptr);
    EXPECT_TRUE(breakpointLocationsRequestPtr == nullptr);
    EXPECT_TRUE(setBreakpointsRequestPtr == nullptr);
    EXPECT_TRUE(terminateRequestPtr == nullptr);
    EXPECT_TRUE(threadsRequestPtr == nullptr);
}

TEST_F(RequestManagerTest, initializeRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "initialize";
    json["arguments"]["adapterID"] = "adapter";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, initializeRequestTest_NoArgument)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "initialize";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    Dap::Protocol::InitializeRequest* ptr = dynamic_cast<Dap::Protocol::InitializeRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, configurationDoneRequestTest_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "configurationDone";
    json["arguments"] = "configurationDone";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::ConfigurationDoneRequest* ptr = dynamic_cast<Dap::Protocol::ConfigurationDoneRequest*>
        (requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(Dap::Protocol::ENUM_TO_STR(ptr->type).value(), "request");
    EXPECT_EQ(ptr->command, "configurationDone");
}

TEST_F(RequestManagerTest, configurationDoneRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "configurationDone";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::ConfigurationDoneRequest* ptr = dynamic_cast<Dap::Protocol::ConfigurationDoneRequest*>
    (requestSecPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

TEST_F(RequestManagerTest, restartRequestTest_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "restart";
    json["arguments"] = "arguments";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::RestartRequest* ptr = dynamic_cast<Dap::Protocol::RestartRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(Dap::Protocol::ENUM_TO_STR(ptr->type).value(), "request");
    EXPECT_EQ(ptr->command, "restart");
}

TEST_F(RequestManagerTest, restartRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "restart";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::RestartRequest* ptr = dynamic_cast<Dap::Protocol::RestartRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

TEST_F(RequestManagerTest, disconnectRequestTest_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "disconnect";
    json["arguments"]["restart"] = true;
    json["arguments"]["terminateDebuggee"] = true;
    json["arguments"]["isDebuggeeCrashed"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::DisconnectRequest* ptr = dynamic_cast<Dap::Protocol::DisconnectRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(Dap::Protocol::ENUM_TO_STR(ptr->type).value(), "request");
    EXPECT_EQ(ptr->command, "disconnect");
    EXPECT_EQ(ptr->arguments->isDebuggeeCrashed.value(), true);
}

TEST_F(RequestManagerTest, disconnectRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "disconnect";
    json["arguments"]["restart"] = true;
    json["arguments"]["terminateDebuggee"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json.clear();
    json["seq"] = 1;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "disconnect";
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::DisconnectRequest* ptr = dynamic_cast<Dap::Protocol::DisconnectRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

TEST_F(RequestManagerTest, terminateRequestTest_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "terminate";
    json["arguments"]["restart"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::TerminateRequest* ptr = dynamic_cast<Dap::Protocol::TerminateRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(Dap::Protocol::ENUM_TO_STR(ptr->type).value(), "request");
    EXPECT_EQ(ptr->command, "terminate");
    EXPECT_EQ(ptr->arguments.value().restart, false);
}

TEST_F(RequestManagerTest, terminateRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "terminate";
    json["arguments"]["restart"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json.clear();
    json["seq"] = 1;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "terminate";
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::TerminateRequest* ptr = dynamic_cast<Dap::Protocol::TerminateRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

void SetJsonOfLaunchRequest(json_t &json)
{
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "launch";
    json["arguments"]["noDebug"] = true;
    json["arguments"]["name"] = "gdb startup";
    json["arguments"]["type"] = "cppdbg";
    json["arguments"]["request"] = "launch";
    json["arguments"]["program"] = "C:\\test.exe";
    json["arguments"]["args"] = json_t::array();
    json["arguments"]["args"].push_back("C:\\mingw\\gdb.exe");
    json["arguments"]["args"].push_back("C:\\text.exe");
    json["arguments"]["args"].push_back("9995");
    json["arguments"]["stopAtEntry"] = false;
    json["arguments"]["cwd"] = "C:\\";
    json["arguments"]["env"]["PATH"] = "C:\\";
    json["arguments"]["externalConsole"] = false;
    json["arguments"]["setupCommands"] = json_t::array();
    json_t setupCmdJson1;
    setupCmdJson1["description"] = "setup command 1";
    setupCmdJson1["text"] = "setup command 1 text";
    setupCmdJson1["ignoreFailures"] = true;
    json["arguments"]["setupCommands"].push_back(setupCmdJson1);
    json_t setupCmdJson2;
    setupCmdJson2["description"] = "setup command 2";
    setupCmdJson2["text"] = "setup command 2 text";
    setupCmdJson2["ignoreFailures"] = false;
    json["arguments"]["setupCommands"].push_back(setupCmdJson2);
    json["arguments"]["__configurationTarget"] = 3;
    json["arguments"]["__sessionId"] = "1234567890";
    json["arguments"]["launcherPath"] = "C:\\Users\\debugAdapters\\bin\\WindowsDebugLauncher.exe";
    std::string remotePath = "/home/project/Clionhelloworld";
    std::string localPath = "D:\\ide\\dap\\helloworld";
    json["arguments"]["sourceFileMap"][remotePath] = localPath;
    json["arguments"]["debuggeeStarted"] = true;
    json["arguments"]["remoteConnected"] = true;
}

TEST_F(RequestManagerTest, launchRequestTest_1)
{
    json_t jsonStrCommands = json_t::array();
    for (int i = 0; i < arraySize; i++) {
        jsonStrCommands.push_back("Command");
    }
    json_t json;
    SetJsonOfLaunchRequest(json);
    json["arguments"]["scriptCommands"] = jsonStrCommands;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::LaunchRequest* ptr = dynamic_cast<Dap::Protocol::LaunchRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "launch");
    EXPECT_EQ(ptr->arguments.noDebug, true);
    // vscode
    EXPECT_EQ(ptr->arguments.name.value(), "gdb startup");
    EXPECT_EQ(ptr->arguments.type.value(), "cppdbg");
    EXPECT_EQ(ptr->arguments.request.value(), "launch");
    EXPECT_EQ(ptr->arguments.program.value(), "C:\\test.exe");
    EXPECT_EQ(ptr->arguments.args->size(), 3);
    EXPECT_EQ(ptr->arguments.args->at(0), "C:\\mingw\\gdb.exe");
    EXPECT_EQ(ptr->arguments.args->at(1), "C:\\text.exe");
    EXPECT_EQ(ptr->arguments.args->at(2), "9995");
    EXPECT_EQ(ptr->arguments.stopAtEntry.value(), false);
    EXPECT_EQ(ptr->arguments.cwd.value(), "C:\\");
    EXPECT_EQ(ptr->arguments.env->size(), 1);
    EXPECT_EQ(ptr->arguments.env.value().at("PATH"), "C:\\");
    EXPECT_EQ(ptr->arguments.externalConsole.value(), false);
    EXPECT_EQ(ptr->arguments.setupCommands->size(), 2);
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).description, "setup command 1");
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).text, "setup command 1 text");
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).ignoreFailures, true);
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).description, "setup command 2");
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).text, "setup command 2 text");
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).ignoreFailures, false);
    EXPECT_EQ(ptr->arguments.__configurationTarget.value(), 3);
    EXPECT_EQ(ptr->arguments.__sessionId.value(), "1234567890");
    EXPECT_EQ(ptr->arguments.sourceFileMap.value().remotePath, "/home/project/Clionhelloworld");
    EXPECT_EQ(ptr->arguments.sourceFileMap.value().localPath, "D:/ide/dap/helloworld");
    EXPECT_EQ(ptr->arguments.debuggeeStarted, true);
    EXPECT_EQ(ptr->arguments.remoteConnected, true);
}

TEST_F(RequestManagerTest, launchRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "launch";
    json["arguments"]["noDebug"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, launchRequestTest_baseJsonError_2)
{
    json_t json;
    SetJsonOfLaunchRequest(json);
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    (void)dynamic_cast<Dap::Protocol::LaunchRequest*>(requestPtr.get());
}

TEST_F(RequestManagerTest, launchRequestTest_NoArgumentsError)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "launch";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, launchRequestTest_noSourceFileMap)
{
    json_t json;
    SetJsonOfLaunchRequest(json);
    json["arguments"]["sourceFileMap"] = "";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
}

TEST_F(RequestManagerTest, launchRequestTest_RequestError_brachtest)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "launch";
    json["arguments"]["args"] = "";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::LaunchRequest* ptr = dynamic_cast<Dap::Protocol::LaunchRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

void SetJsonOfAttachRequest(json_t &json)
{
    json_t jsonSetupCommands = json_t::array();
    json_t setupCmdJson1;
    setupCmdJson1["description"] = "setup command 1";
    setupCmdJson1["text"] = "setup command 1 text";
    setupCmdJson1["ignoreFailures"] = true;
    jsonSetupCommands.push_back(setupCmdJson1);
    json_t setupCmdJson2;
    setupCmdJson2["description"] = "setup command 2";
    setupCmdJson2["text"] = "setup command 2 text";
    setupCmdJson2["ignoreFailures"] = false;
    jsonSetupCommands.push_back(setupCmdJson2);
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "attach";
    json["arguments"]["name"] = "gdb startup";
    json["arguments"]["type"] = "cppdbg";
    json["arguments"]["request"] = "attach";
    json["arguments"]["program"] = "C:\\test.exe";
    json["arguments"]["processId"] = 18916;
    json["arguments"]["setupCommands"] = jsonSetupCommands;
    json["arguments"]["__configurationTarget"] = 3;
    json["arguments"]["__sessionId"] = "1234567890";
    json["arguments"]["showStaticGlobalVars"] = false;
    json["arguments"]["isContainer"] = true;
    std::string remotePath = "/home/project/Clionhelloworld";
    std::string localPath = "D:\\ide\\dap\\helloworld";
    json["arguments"]["sourceFileMap"][remotePath] = localPath;
}

TEST_F(RequestManagerTest, attachRequestTest)
{
    json_t jsonStrCommands = json_t::array();
    for (int i = 0; i < arraySize; i++) {
        jsonStrCommands.push_back("Command");
    }
    json_t json;
    SetJsonOfAttachRequest(json);
    json["arguments"]["scriptCommands"] = jsonStrCommands;
    json["arguments"]["startupCommands"] = jsonStrCommands;
    json["arguments"]["afterConnectCommands"] = jsonStrCommands;
    json["arguments"]["postAttachCommands"] = jsonStrCommands;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::AttachRequest* ptr = dynamic_cast<Dap::Protocol::AttachRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "attach");
    // vscode
    EXPECT_EQ(ptr->arguments.name.value(), "gdb startup");
    EXPECT_EQ(ptr->arguments.type.value(), "cppdbg");
    EXPECT_EQ(ptr->arguments.request.value(), "attach");
    EXPECT_EQ(ptr->arguments.program.value(), "C:\\test.exe");
    EXPECT_EQ(ptr->arguments.processId.value(), "18916");
    EXPECT_EQ(ptr->arguments.setupCommands->size(), 2);
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).description, "setup command 1");
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).text, "setup command 1 text");
    EXPECT_EQ(ptr->arguments.setupCommands->at(0).ignoreFailures, true);
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).description, "setup command 2");
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).text, "setup command 2 text");
    EXPECT_EQ(ptr->arguments.setupCommands->at(1).ignoreFailures, false);
    EXPECT_EQ(ptr->arguments.__configurationTarget.value(), 3);
    EXPECT_EQ(ptr->arguments.__sessionId.value(), "1234567890");
    EXPECT_EQ(ptr->arguments.showStaticGlobalVars.value(), false);
    EXPECT_EQ(ptr->arguments.isContainer.value(), true);
    EXPECT_EQ(ptr->arguments.sourceFileMap.value().remotePath, "/home/project/Clionhelloworld");
    EXPECT_EQ(ptr->arguments.sourceFileMap.value().localPath, "D:/ide/dap/helloworld");
}

TEST_F(RequestManagerTest, attachRequestTest_RequestError)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "attach";
    json["arguments"]["name"] = "gdb startup";
    json["arguments"]["type"] = "cppdbg";
    json["arguments"]["request"] = "attach";
    json["arguments"]["program"] = "C:\\test.exe";
    json["arguments"]["processId"] = "18916";
    json["arguments"]["setupCommands"] = json_t::array();
    json_t setupCmdJson1;
    setupCmdJson1["description"] = "setup command 1";
    setupCmdJson1["text"] = "setup command 1 text";
    setupCmdJson1["ignoreFailures"] = true;
    json["arguments"]["setupCommands"].push_back(setupCmdJson1);
    json_t setupCmdJson2;
    setupCmdJson2["description"] = "setup command 2";
    setupCmdJson2["text"] = "setup command 2 text";
    setupCmdJson2["ignoreFailures"] = false;
    json["arguments"]["setupCommands"].push_back(setupCmdJson2);
    json["arguments"]["__configurationTarget"] = 3;
    json["arguments"]["__sessionId"] = "1234567890";
    json["arguments"]["showStaticGlobalVars"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::AttachRequest* ptr = dynamic_cast<Dap::Protocol::AttachRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, attachRequestTest_NoArguments)
{
    json_t json;
    json["seq"] = 1;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "attach";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::AttachRequest* ptr = dynamic_cast<Dap::Protocol::AttachRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, attachRequestTest_noSourceFileMap)
{
    json_t json;
    SetJsonOfAttachRequest(json);
    json["arguments"]["sourceFileMap"] = "";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
}

TEST_F(RequestManagerTest, attachRequestTest_RequestError_brachtest)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "attach";
    json["arguments"]["processId"] = "";
    json["arguments"]["type"] = "cppdbg";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::AttachRequest* ptr = dynamic_cast<Dap::Protocol::AttachRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "attach");
}

TEST_F(RequestManagerTest, setBreakpointsRequestTest_1)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonObject["path"] = getenv("PATH");
        jsonObject["name"] = "fdfdsfd";
        jsonArray.push_back(jsonObject);
    }

    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonBreakpoint["condition"] = getenv("PATH");
        jsonBreakpoint["column"] = 56;
        jsonBreakpoint["line"] = 23;
        jsonBreakpoint["hitCondition"] = "none";
        jsonBreakpoint["logMessage"] = "none";
        jsonBreakpoints.push_back(jsonBreakpoint);
    }

    json_t jsonLines = json_t::array();
    for (int i = 0; i < arraySize; i++) {
        jsonLines.push_back(i);
    }

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));
    checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA1).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setBreakpoints";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["source"]["sources"] = jsonArray;
    json["arguments"]["source"]["checksums"] = checksumsArray;
    json["arguments"]["source"]["presentationHint"] = ENUM_TO_STR(PresentationHint::DEEMPHASIZE).value();
    json["arguments"]["source"]["adapterData"] = "";
    json["arguments"]["breakpoints"] = jsonBreakpoints;
    json["arguments"]["lines"] = jsonLines;
    json["arguments"]["sourceModified"] = false;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetBreakpointsRequest*>(requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "setBreakpoints");
    EXPECT_EQ(ptr->arguments.sourceModified, false);
    EXPECT_EQ(ptr->arguments.source.checksums->at(0).algorithm, ChecksumAlgorithmType::SHA256);
    EXPECT_EQ(ptr->arguments.source.checksums->at(1).algorithm, ChecksumAlgorithmType::SHA1);
    EXPECT_EQ(ptr->arguments.lines.value().at(1), 1);
}

TEST_F(RequestManagerTest, setBreakpointsRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "setBreakpoints";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["source"]["presentationHint"] = ENUM_TO_STR(PresentationHint::DEEMPHASIZE).value();
    json["arguments"]["sourceModified"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, setBreakpointsRequestTest_noSourceError_1)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonObject["path"] = getenv("PATH");
        jsonObject["name"] = "fdfdsfd";
        jsonArray.push_back(jsonObject);
    }

    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonBreakpoint["condition"] = getenv("PATH");
        jsonBreakpoint["column"] = 56;
        jsonBreakpoint["line"] = 23;
        jsonBreakpoints.push_back(jsonBreakpoint);
    }

    json_t jsonLines = json_t::array();
    for (int i = 0; i < arraySize; i++) {
        jsonLines.push_back(i);
    }

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));
    checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA1).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setBreakpoints";
    json["arguments"]["breakpoints"] = jsonBreakpoints;
    json["arguments"]["lines"] = jsonLines;
    json["arguments"]["sourceModified"] = false;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, setBreakpointsRequestTest_breakpointsNoLineError_1)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonObject["path"] = getenv("PATH");
        jsonObject["name"] = "fdfdsfd";
        jsonArray.push_back(jsonObject);
    }

    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonBreakpoint["condition"] = getenv("PATH");
        jsonBreakpoint["column"] = 56;
        jsonBreakpoints.push_back(jsonBreakpoint);
    }

    json_t jsonLines = json_t::array();
    for (int i = 0; i < arraySize; i++) {
        jsonLines.push_back(i);
    }

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));
    checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA1).value();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setBreakpoints";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["source"]["sources"] = jsonArray;
    json["arguments"]["source"]["checksums"] = checksumsArray;
    json["arguments"]["source"]["presentationHint"] = ENUM_TO_STR(PresentationHint::DEEMPHASIZE).value();
    json["arguments"]["breakpoints"] = jsonBreakpoints;
    json["arguments"]["lines"] = jsonLines;
    json["arguments"]["sourceModified"] = false;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, setBreakpointsRequestTest_noAlgorithm)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setBreakpoints";
    json["arguments"]["source"]["path"] = getenv("PATH");
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, "setBreakpoints");

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));
    json["arguments"]["source"]["checksums"] = checksumsArray;

    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestSecPtr == nullptr);
}

TEST_F(RequestManagerTest, breakpointLocationsRequestTest_1)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "breakpointLocations";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["line"] = 58;
    json["arguments"]["column"] = 59;
    json["arguments"]["endLine"] = 60;
    json["arguments"]["endColumn"] = 61;
    json["arguments"]["startAddress"] = "0x58";
    json["arguments"]["filterStepinLocation"] = true;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::BreakpointLocationsRequest* ptr = dynamic_cast<Dap::Protocol::BreakpointLocationsRequest*>
        (requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "breakpointLocations");
    EXPECT_EQ(ptr->arguments->source.path, getenv("PATH"));
    EXPECT_EQ(ptr->arguments->line, 58);
    EXPECT_EQ(ptr->arguments->column, 59);
    EXPECT_EQ(ptr->arguments->endLine, 60);
    EXPECT_EQ(ptr->arguments->endColumn, 61);
    EXPECT_EQ(ptr->arguments->startAddress, "0x58");
    EXPECT_EQ(ptr->arguments->filterStepinLocation, true);
}

TEST_F(RequestManagerTest, breakpointLocationsRequestTest_baseJsonError_1)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "breakpointLocations";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["line"] = 58;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, breakpointLocationsRequestTest_noSourceError_1)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "breakpointLocations";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::BreakpointLocationsRequest* ptr = dynamic_cast<Dap::Protocol::BreakpointLocationsRequest*>
    (requestPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);

    json["arguments"]["line"] = 58;
    std::unique_ptr<Dap::Protocol::Request> requestFirstPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestFirstPtr == nullptr);

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value();
    checksumsArray.push_back(std::move(checksumsObject));
    json["arguments"]["source"]["checksums"] = checksumsArray;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestSecPtr, nullptr);
}

TEST_F(RequestManagerTest, breakpointLocationsRequestTest_noLineError_1)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "breakpointLocations";
    json["arguments"]["source"]["path"] = getenv("PATH");

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

static json_t FunctionBreakpointsDataJson()
{
    json_t data = json_t::object();
    data["supportsConditionalBreakpoints"]  = true;
    data["supportsHitConditionalBreakpoints"]  = true;
    data["supportsLogPoints"]  = true;
    data["supportsFunctionBreakpoints"]  = true;
    data["supportsDataBreakpoints"]  = true;
    data["id"]  = 2;
    data["verified"]  = true;
    data["line"]  = 13;
    data["sessionId"]  = "27da6f02-89ce-4f9c-a108-b66b0d90c8f5";
    return data;
}

TEST_F(RequestManagerTest, setFunctionBreakpointsRequestTest)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    jsonObject["name"] = "functionName";
    jsonObject["condition"] = "condition";
    jsonObject["hitCondition"] = "hitCondition";
    jsonObject["enabled"] = true;
    jsonObject["id"] = "db73c682-82a3-4d41-b0d4-bf0873705f6d";
    jsonObject["sessionData"] = "{}";
    jsonObject["data"] = FunctionBreakpointsDataJson();
    jsonObject["hitOnce"] = true;
    jsonObject["isRegex"] = true;
    jsonObject["moduleName"] = "libjni.so";
    jsonArray.push_back(jsonObject);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setFunctionBreakpoints";
    json["arguments"]["breakpoints"] = jsonArray;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetFunctionBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetFunctionBreakpointsRequest*>(
        requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "setFunctionBreakpoints");
    EXPECT_EQ(ptr->arguments.breakpoints.size(), 1);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).name, "functionName");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).condition, "condition");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).hitCondition, "hitCondition");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).enabled, true);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).id, "db73c682-82a3-4d41-b0d4-bf0873705f6d");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).sessionData, "{}");
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->supportsConditionalBreakpoints);
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->supportsHitConditionalBreakpoints);
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->supportsLogPoints);
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->supportsFunctionBreakpoints);
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->supportsDataBreakpoints);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).data->id, 2);
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).data->verified);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).data->line, 13);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).data->sessionId, "27da6f02-89ce-4f9c-a108-b66b0d90c8f5");
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).isRegex.value());
    EXPECT_TRUE(ptr->arguments.breakpoints.at(0).hitOnce.value());
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).moduleName, "libjni.so");
}

TEST_F(RequestManagerTest, setFunctionBreakpointsRequestTest_baseJsonError)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    jsonObject["name"] = "functionName";
    jsonObject["condition"] = "condition";
    jsonObject["hitCondition"] = "hitCondition";
    jsonArray.push_back(jsonObject);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "setFunctionBreakpoints";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    json["arguments"]["breakpoints"] = jsonArray;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetFunctionBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetFunctionBreakpointsRequest*>(
            requestSecPtr.get());
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
}

TEST_F(RequestManagerTest, dataBreakpointInfoRequestTest)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "dataBreakpointInfo";
    json["arguments"]["variablesReference"] = 23;
    json["arguments"]["name"] = "name";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::DataBreakpointInfoRequest* ptr = dynamic_cast<Dap::Protocol::DataBreakpointInfoRequest*>(
            requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "dataBreakpointInfo");
    EXPECT_EQ(ptr->arguments.variablesReference, 23);
    EXPECT_EQ(ptr->arguments.name, "name");
}

TEST_F(RequestManagerTest, dataBreakpointInfoRequestTest_baseJsonError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "dataBreakpointInfo";
    json["arguments"]["variablesReference"] = 23;
    json["arguments"]["name"] = "name";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, dataBreakpointInfoRequestTest_noNameError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "dataBreakpointInfo";
    json["arguments"]["variablesReference"] = 23;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    (void)dynamic_cast<Dap::Protocol::DataBreakpointInfoRequest*>(
            requestPtr.get());

    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, setDataBreakpointsRequestTest)
{
    json_t breakpoints = json_t::array();
    json_t breakpoint = json_t::object();
    breakpoint["dataId"] = "dataId";
    breakpoint["accessType"] = "readWrite";
    breakpoint["condition"] = "condition";
    breakpoint["hitCondition"] = "hitCondition";
    breakpoints.push_back(breakpoint);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setDataBreakpoints";
    json["arguments"]["breakpoints"] = breakpoints;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetDataBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetDataBreakpointsRequest*>(
            requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "setDataBreakpoints");
    EXPECT_EQ(ptr->arguments.breakpoints.size(), 1);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).dataId, "dataId");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).accessType, DataBreakpointAccessType::READ_WRITE);
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).condition, "condition");
    EXPECT_EQ(ptr->arguments.breakpoints.at(0).hitCondition, "hitCondition");
}

TEST_F(RequestManagerTest, setDataBreakpointsRequestTest_baseJsonError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "setDataBreakpoints";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, setDataBreakpointsRequestTest_noDataIdError)
{
    json_t breakpoints = json_t::array();
    json_t breakpoint = json_t::object();
    breakpoint["accessType"] = "readWrite";
    breakpoint["condition"] = "condition";
    breakpoint["hitCondition"] = "hitCondition";
    breakpoints.push_back(breakpoint);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setDataBreakpoints";
    json["arguments"]["breakpoints"] = breakpoints;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);

    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, continueRequestTest_1)
{
    json_t json1;
    json1["arguments"]["threadId"] = 4004;
    json1["seq"] = 3;
    json1["command"] = "continue";
    json1["type"] = "request";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::ContinueRequest* ptr = dynamic_cast<Dap::Protocol::ContinueRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "continue");
    EXPECT_EQ(ptr->arguments.threadId, 4004);
}

TEST_F(RequestManagerTest, continueRequestTest_baseJsonError_1)
{
    json_t json1;
    json1["arguments"]["threadId"] = 4004;
    json1["command"] = "continue";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::ContinueRequest* ptr = dynamic_cast<Dap::Protocol::ContinueRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, continueRequestTest_noThreadIdError_1)
{
    json_t json1;
    json1["arguments"];
    json1["seq"] = 3;
    json1["command"] = "continue";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::ContinueRequest* ptr = dynamic_cast<Dap::Protocol::ContinueRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, pauseRequestTest_1)
{
    json_t json1;
    json1["arguments"]["threadId"] = 4004;
    json1["seq"] = 3;
    json1["command"] = "pause";
    json1["type"] = "request";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::PauseRequest* ptr = dynamic_cast<Dap::Protocol::PauseRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "pause");
    EXPECT_EQ(ptr->arguments.threadId, 4004);
}

TEST_F(RequestManagerTest, pauseRequestTest_baseJsonError_1)
{
    json_t json1;
    json1["arguments"]["threadId"] = 4004;
    json1["command"] = "pause";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::PauseRequest* ptr = dynamic_cast<Dap::Protocol::PauseRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, pauseRequestTest_noThreadIdError_1)
{
    json_t json1;
    json1["arguments"];
    json1["seq"] = 3;
    json1["command"] = "pause";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::PauseRequest* ptr = dynamic_cast<Dap::Protocol::PauseRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, NextRequestTest)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "next";
    json["type"] = "request";
    json["arguments"]["threadId"] = 4004;
    json["arguments"]["granularity"] = ENUM_TO_STR(SteppingGranularity::INSTRUCTION).value();

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::NextRequest* ptr = dynamic_cast<Dap::Protocol::NextRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "next");
    EXPECT_EQ(ptr->arguments.threadId, 4004);
    EXPECT_EQ(ptr->arguments.granularity, SteppingGranularity::INSTRUCTION);
}

TEST_F(RequestManagerTest, nextRequestTest_baseJsonError_1)
{
    json_t json;
    json["arguments"]["threadId"] = 4004;
    json["command"] = "next";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::NextRequest* ptr = dynamic_cast<Dap::Protocol::NextRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerTest, NextRequestTest_noArgumentsError)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "next";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, StepInRequestTest)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "stepIn";
    json["type"] = "request";
    json["arguments"]["threadId"] = 4004;
    json["arguments"]["targetId"] = 3003;
    json["arguments"]["granularity"] = ENUM_TO_STR(SteppingGranularity::INSTRUCTION).value();

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StepInRequest* ptr = dynamic_cast<Dap::Protocol::StepInRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "stepIn");
    EXPECT_EQ(ptr->arguments.threadId, 4004);
    EXPECT_EQ(ptr->arguments.targetId, 3003);
    EXPECT_EQ(ptr->arguments.granularity, SteppingGranularity::INSTRUCTION);
}

TEST_F(RequestManagerTest, StepInRequestTest_baseJsonError)
{
    json_t json;
    json["arguments"]["threadId"] = 4004;
    json["command"] = "stepIn";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StepInRequest* ptr = dynamic_cast<Dap::Protocol::StepInRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerTest, StepInRequestTest_noArgumentsError)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "stepIn";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    (void)dynamic_cast<Dap::Protocol::StepInRequest*>(requestPtr.get());
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, StepOutRequestTest)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "stepOut";
    json["type"] = "request";
    json["arguments"]["threadId"] = 4004;
    json["arguments"]["granularity"] = ENUM_TO_STR(SteppingGranularity::LINE).value();

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StepOutRequest* ptr = dynamic_cast<Dap::Protocol::StepOutRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "stepOut");
    EXPECT_EQ(ptr->arguments.threadId, 4004);
    EXPECT_EQ(ptr->arguments.granularity, SteppingGranularity::LINE);
}

TEST_F(RequestManagerTest, StepOutRequestTest_baseJsonError)
{
    json_t json;
    json["arguments"]["threadId"] = 4004;
    json["command"] = "stepOut";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StepOutRequest* ptr = dynamic_cast<Dap::Protocol::StepOutRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerTest, StepOutRequestTest_noArgumentsError)
{
    json_t json;
    json["seq"] = 3;
    json["command"] = "stepOut";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StepOutRequest* ptr = dynamic_cast<Dap::Protocol::StepOutRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerTest, stackTraceRequestTest)
{
    json_t json1;
    json1["seq"] = 3;
    json1["command"] = REQ_RES_STACK_TRACE;
    json1["type"] = "request";
    json1["arguments"]["threadId"] = 4003;
    json1["arguments"]["startFrame"] = 32;
    json1["arguments"]["levels"] = 2;
    json_t format;
    format["parameters"] = true;
    format["parameterTypes"] = true;
    format["parameterNames"] = true;
    format["parameterValues"] = true;
    format["line"] = true;
    format["module"] = true;
    format["includeAll"] = true;
    json1["arguments"]["format"] = format;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::StackTraceRequest* ptr = dynamic_cast<Dap::Protocol::StackTraceRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, REQ_RES_STACK_TRACE);
    EXPECT_EQ(ptr->arguments.threadId, 4003);
    EXPECT_EQ(ptr->arguments.levels, 2);
    EXPECT_EQ(ptr->arguments.startFrame, 32);
    EXPECT_TRUE(ptr->arguments.format.value().parameters.value());
    EXPECT_TRUE(ptr->arguments.format.value().parameterTypes.value());
    EXPECT_TRUE(ptr->arguments.format.value().parameterNames.value());
    EXPECT_TRUE(ptr->arguments.format.value().parameterValues.value());
    EXPECT_TRUE(ptr->arguments.format.value().line.value());
    EXPECT_TRUE(ptr->arguments.format.value().module.value());
    EXPECT_TRUE(ptr->arguments.format.value().includeAll.value());
}

TEST_F(RequestManagerTest, stackTraceRequestTest_baseJsonError_1)
{
    json_t json;
    json["command"] = REQ_RES_STACK_TRACE;
    json["type"] = "request";
    json["arguments"]["threadId"] = 4003;
    json["arguments"]["startFrame"] = 32;
    json["arguments"]["levels"] = 2;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_TRUE(requestPtr == nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::StackTraceRequest* ptr = dynamic_cast<Dap::Protocol::StackTraceRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerTest, stackTraceRequestTest_noThreadId_1)
{
    json_t json1;
    json1["seq"] = 1;
    json1["command"] = REQ_RES_STACK_TRACE;
    json1["type"] = "request";
    json1["arguments"]["startFrame"] = 32;
    json1["arguments"]["levels"] = 2;
    json_t format;
    format["parameters"] = true;
    format["parameterTypes"] = true;
    format["parameterNames"] = true;
    format["parameterValues"] = true;
    format["line"] = true;
    format["module"] = true;
    format["includeAll"] = true;
    json1["arguments"]["format"] = format;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1.dump());
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, threadsRequestTest)
{
    json_t json1;
    json1["seq"] = 3;
    json1["command"] = REQ_RES_THREADS;
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::ThreadsRequest* ptr = dynamic_cast<Dap::Protocol::ThreadsRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, REQ_RES_THREADS);
}

TEST_F(RequestManagerTest, threadsRequestTest_baseJsonError_1)
{
    json_t json1;
    json1["command"] = REQ_RES_THREADS;
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1.dump());
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerTest, scopesRequestTest)
{
    json_t scopesRequestJson;
    scopesRequestJson["seq"] = 4;
    scopesRequestJson["command"] = REQ_RES_SCOPES;
    scopesRequestJson["type"] = "request";
    scopesRequestJson["arguments"]["frameId"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(scopesRequestJson.dump());
    auto ptr = dynamic_cast<ScopesRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_SCOPES);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.frameId, 1001);
}

TEST_F(RequestManagerTest, scopesRequestTest_baseInfoError)
{
    json_t scopesRequestJson;
    scopesRequestJson["command"] = REQ_RES_SCOPES;
    scopesRequestJson["type"] = "request";
    scopesRequestJson["arguments"]["frameId"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(scopesRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, scopesRequestTest_noArgumentsError)
{
    json_t scopesRequestJson;
    scopesRequestJson["seq"] = 4;
    scopesRequestJson["command"] = REQ_RES_SCOPES;
    scopesRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(scopesRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, variablesRequestTest)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_VARIABLES;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    auto ptr = dynamic_cast<VariablesRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_VARIABLES);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.variablesReference, 1001);
}

TEST_F(RequestManagerTest, variablesRequestTest_baseInfoError)
{
    json_t variablesJson;
    variablesJson["command"] = REQ_RES_VARIABLES;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, variablesRequestTest_noArgumentsError)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_VARIABLES;
    variablesJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, variablesExpandRequestTest)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_VARIABLES_EXPAND;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    auto ptr = dynamic_cast<VariablesExpandRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_VARIABLES_EXPAND);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.variablesReference, 1001);
}

TEST_F(RequestManagerTest, preVariablesRequestTest)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_PRE_VARIABLES;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    auto ptr = dynamic_cast<PreVariablesRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_PRE_VARIABLES);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.variablesReference, 1001);
}

TEST_F(RequestManagerTest, preVariablesRequestTest_baseInfoError)
{
    json_t variablesJson;
    variablesJson["command"] = REQ_RES_PRE_VARIABLES;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, preVariablesRequestTest_noArgumentsError)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_PRE_VARIABLES;
    variablesJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, variablesExpandRequestTest_baseInfoError)
{
    json_t variablesJson;
    variablesJson["command"] = REQ_RES_VARIABLES_EXPAND;
    variablesJson["type"] = "request";
    variablesJson["arguments"]["variablesReference"] = 1001;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, variablesExpandRequestTest_noArgumentsError)
{
    json_t variablesJson;
    variablesJson["seq"] = 5;
    variablesJson["command"] = REQ_RES_VARIABLES_EXPAND;
    variablesJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(variablesJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, setVariableRequestTest)
{
    json_t setVariableJson;
    setVariableJson["seq"] = 1;
    setVariableJson["command"] = REQ_RES_SET_VARIABLE;
    setVariableJson["type"] = "request";
    setVariableJson["arguments"]["variablesReference"] = 1000;
    setVariableJson["arguments"]["name"] = "var1";
    setVariableJson["arguments"]["value"] = "100";
    setVariableJson["arguments"]["format"]["hex"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(setVariableJson.dump());
    auto ptr = dynamic_cast<SetVariableRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_SET_VARIABLE);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.variablesReference, 1000);
    EXPECT_EQ(ptr->arguments.name, "var1");
    EXPECT_EQ(ptr->arguments.value, "100");
    EXPECT_TRUE(ptr->arguments.format.value().hex.value());
}

TEST_F(RequestManagerTest, setVariableRequestTest_baseInfoError)
{
    json_t setVariableJson;
    setVariableJson["command"] = REQ_RES_SET_VARIABLE;
    setVariableJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(setVariableJson.dump());
    EXPECT_EQ(requestPtr, nullptr);

    setVariableJson["seq"] = 1;
    setVariableJson["arguments"]["variablesReference"] = 1000;
    setVariableJson["arguments"]["name"] = "var1";
    setVariableJson["arguments"]["value"] = "100";
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(setVariableJson);
    Dap::Protocol::SetVariableRequest* ptr = dynamic_cast<Dap::Protocol::SetVariableRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerTest, setVariableRequestTest_noNameError)
{
    json_t setVariableJson;
    setVariableJson["seq"] = 1;
    setVariableJson["command"] = REQ_RES_SET_VARIABLE;
    setVariableJson["type"] = "request";
    setVariableJson["arguments"]["variablesReference"] = 1000;
    setVariableJson["arguments"]["value"] = "100";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(setVariableJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, DisassembleRequestTest)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["memoryReference"] = "0x0000000000405536";
    argumentsJson["offset"] = 0;
    argumentsJson["instructionOffset"] = 0;
    argumentsJson["instructionCount"] = 1;
    argumentsJson["resolveSymbols"] = true;

    json_t disassembleRequestJson;
    disassembleRequestJson["arguments"] = argumentsJson;
    disassembleRequestJson["command"] = REQ_RES_DISASSEMBLE;
    disassembleRequestJson["type"] = "request";
    disassembleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            disassembleRequestJson.dump());
    auto ptr = dynamic_cast<DisassembleRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_DISASSEMBLE);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.instructionCount, 1);
    EXPECT_EQ(ptr->arguments.instructionOffset, 0);
    EXPECT_EQ(ptr->arguments.offset, 0);
    EXPECT_EQ(ptr->arguments.memoryReference, "0x0000000000405536");
    EXPECT_TRUE(ptr->arguments.resolveSymbols);
}

TEST_F(RequestManagerTest, DisassembleRequestTest_baseInfoError)
{
    json_t disassembleRequestJson;
    disassembleRequestJson["command"] = REQ_RES_DISASSEMBLE;
    disassembleRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            disassembleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, DisassembleRequestTest_noArgumentsError)
{
    json_t disassembleRequestJson;
    disassembleRequestJson["command"] = REQ_RES_DISASSEMBLE;
    disassembleRequestJson["type"] = "request";
    disassembleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            disassembleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, DisassembleRequestTest_noNecessaryArgumentsError)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["memoryReference"] = "0x0000000000405536";
    argumentsJson["offset"] = 0;
    argumentsJson["instructionOffset"] = 0;
    argumentsJson["resolveSymbols"] = true;

    json_t disassembleRequestJson;
    disassembleRequestJson["arguments"] = argumentsJson;
    disassembleRequestJson["command"] = REQ_RES_DISASSEMBLE;
    disassembleRequestJson["type"] = "request";
    disassembleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            disassembleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, EvaluateRequestTest)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["expression"] = "a == 1";
    argumentsJson["frameId"] = 1000;
    argumentsJson["context"] = "watch";
    argumentsJson["format"]["hex"] = true;
    argumentsJson["isViewMore"] = true;

    json_t requestJson;
    requestJson["arguments"] = argumentsJson;
    requestJson["command"] = REQ_RES_EVALUATE;
    requestJson["type"] = "request";
    requestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr =
            RequestManager::Instance().FromJson(requestJson.dump());
    auto ptr = dynamic_cast<EvaluateRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_EVALUATE);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_TRUE(ptr->arguments.expression == "a == 1");
    EXPECT_EQ(ptr->arguments.frameId.value(), 1000);
    EXPECT_TRUE(ptr->arguments.context.value() == Context::WATCH);
    ValueFormat format = ptr->arguments.format.value();
    EXPECT_TRUE(format.hex.value());
    EXPECT_TRUE(ptr->arguments.isViewMore.value());
}

TEST_F(RequestManagerTest, runInTerminalRequestTest)
{
    RunInTerminalRequest runInTerminalRequest;
    runInTerminalRequest.seq = 19;
    runInTerminalRequest.arguments.cwd = "cwd";
    runInTerminalRequest.arguments.title = "title";
    runInTerminalRequest.arguments.kind = RunInTerminalKind::INTEGRATED;
    std::vector<std::string> args;
    args.push_back("args0");
    args.push_back("args1");
    runInTerminalRequest.arguments.args = args;
    std::vector<EnvPair> envPairs;
    EnvPair envPair0;
    envPair0.key = "key0";
    envPair0.value = "value0";
    envPairs.push_back(envPair0);
    runInTerminalRequest.arguments.env = envPairs;
    json_t json = RequestManager::Instance().ToJson(runInTerminalRequest).value();
    EXPECT_EQ(json["seq"], 19);
    EXPECT_EQ(json["type"], REQUEST_NAME);
    EXPECT_EQ(json["command"], REQ_RES_RUN_IN_TERMINAL);
    EXPECT_EQ(json["arguments"]["cwd"], "cwd");
    EXPECT_EQ(json["arguments"]["title"], "title");
    EXPECT_EQ(json["arguments"]["kind"], ENUM_TO_STR(RunInTerminalKind::INTEGRATED));
    EXPECT_EQ(json["arguments"]["args"].at(0), "args0");
    EXPECT_EQ(json["arguments"]["env"]["key0"], "value0");
}

TEST_F(RequestManagerTest, unInTerminalRequestTest_requestError)
{
    RunInTerminalRequest runInTerminalRequest;
    runInTerminalRequest.seq = 19;
    runInTerminalRequest.arguments.cwd = "cwd";
    std::vector<std::string> args;
    args.push_back("args0");
    args.push_back("args1");
    runInTerminalRequest.arguments.args = args;
    json_t json = RequestManager::Instance().ToJson(runInTerminalRequest).value();
    EXPECT_EQ(json["seq"], 19);
}

TEST_F(RequestManagerTest, GetRequestJsonFuncTest)
{
    EXPECT_EQ(RequestManager::Instance().GetRequestJsonFunc(""), std::nullopt);
}

TEST_F(RequestManagerTest, NoCommandErrorTest)
{
    json_t deleteSingleBreakpointRequest;
    deleteSingleBreakpointRequest["seq"] = 1;
    deleteSingleBreakpointRequest["type"] = "request";
    deleteSingleBreakpointRequest["arguments"]["id"] = 10;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        deleteSingleBreakpointRequest.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

struct ErrorRequest : public Request {
    ErrorRequest() : Request("error") {}
};

TEST_F(RequestManagerTest, ToJsonTest)
{
    ErrorRequest errorRequest;
    EXPECT_EQ(RequestManager::Instance().ToJson(errorRequest), std::nullopt);
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT) {
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    // filters
    json["arguments"]["filters"].push_back("throw");
    json["arguments"]["filters"].push_back("catch");
    // ExceptionFilterOptions
    json_t exceptionFilterOptions;
    exceptionFilterOptions["filterId"] = "1";
    exceptionFilterOptions["condition"] = "2";
    json["arguments"]["filterOptions"].push_back(exceptionFilterOptions);
    // exceptionOptions
    json_t exceptionOptions;
    exceptionOptions["breakMode"] = Dap::Protocol::ENUM_TO_STR(ExceptionBreakMode::USERUNHANDLED).value();
    // exceptionPathSegment
    json_t exceptionPathSegment;
    exceptionPathSegment["negate"] = true;
    exceptionPathSegment["names"].push_back("names");
    exceptionOptions["path"].push_back(exceptionPathSegment);
    json["arguments"]["exceptionOptions"].push_back(exceptionOptions);

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_SET_EXCEPTION_BREAKPOINTS);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.filters.at(0), "throw");
    EXPECT_EQ(ptr->arguments.filters.at(1), "catch");
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT_RequestInfoError) {
    json_t json;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);

    json["seq"] = 1;
    // filters
    json["arguments"]["filters"].push_back("throw");
    json["arguments"]["filters"].push_back("catch");
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr->seq, 1);
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT_NoFilters) {
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    // ExceptionFilterOptions
    json_t exceptionFilterOptions;
    exceptionFilterOptions["filterId"] = "1";
    exceptionFilterOptions["condition"] = "2";
    json["arguments"]["filterOptions"].push_back(exceptionFilterOptions);
    // exceptionOptions
    json_t exceptionOptions;
    exceptionOptions["breakMode"] = Dap::Protocol::ENUM_TO_STR(ExceptionBreakMode::USERUNHANDLED).value();
    // exceptionPathSegment
    json_t exceptionPathSegment;
    exceptionPathSegment["negate"] = true;
    exceptionPathSegment["names"].push_back("names");
    exceptionOptions["path"].push_back(exceptionPathSegment);
    json["arguments"]["exceptionOptions"].push_back(exceptionOptions);
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT_NoFilterId) {
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    // filters
    json["arguments"]["filters"].push_back("throw");
    json["arguments"]["filters"].push_back("catch");
    // ExceptionFilterOptions
    json_t exceptionFilterOptions;
    exceptionFilterOptions["condition"] = "2";
    json["arguments"]["filterOptions"].push_back(exceptionFilterOptions);
    // exceptionOptions
    json_t exceptionOptions;
    exceptionOptions["breakMode"] = Dap::Protocol::ENUM_TO_STR(ExceptionBreakMode::USERUNHANDLED).value();
    // exceptionPathSegment
    json_t exceptionPathSegment;
    exceptionPathSegment["negate"] = true;
    exceptionPathSegment["names"].push_back("names");
    exceptionOptions["path"].push_back(exceptionPathSegment);
    json["arguments"]["exceptionOptions"].push_back(exceptionOptions);
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT_NoBreakMode) {
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    // filters
    json["arguments"]["filters"].push_back("throw");
    json["arguments"]["filters"].push_back("catch");
    // ExceptionFilterOptions
    json_t exceptionFilterOptions;
    exceptionFilterOptions["filterId"] = "1";
    exceptionFilterOptions["condition"] = "2";
    json["arguments"]["filterOptions"].push_back(exceptionFilterOptions);
    // exceptionOptions
    json_t exceptionOptions;
    // exceptionPathSegment
    json_t exceptionPathSegment;
    exceptionPathSegment["negate"] = true;
    exceptionPathSegment["names"].push_back("names");
    exceptionOptions["path"].push_back(exceptionPathSegment);
    json["arguments"]["exceptionOptions"].push_back(exceptionOptions);
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, SetExceptionBreakpointsRequestUT_NoNames) {
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "setExceptionBreakpoints";
    // filters
    json["arguments"]["filters"].push_back("throw");
    json["arguments"]["filters"].push_back("catch");
    // ExceptionFilterOptions
    json_t exceptionFilterOptions;
    exceptionFilterOptions["filterId"] = "1";
    exceptionFilterOptions["condition"] = "2";
    json["arguments"]["filterOptions"].push_back(exceptionFilterOptions);
    // exceptionOptions
    json_t exceptionOptions;
    exceptionOptions["breakMode"] = Dap::Protocol::ENUM_TO_STR(ExceptionBreakMode::USERUNHANDLED).value();
    // exceptionPathSegment
    json_t exceptionPathSegment;
    exceptionPathSegment["negate"] = true;
    exceptionOptions["path"].push_back(exceptionPathSegment);
    json["arguments"]["exceptionOptions"].push_back(exceptionOptions);
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<Dap::Protocol::SetExceptionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(RequestManagerTest, SourceRequestTest)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "source";
    json["arguments"]["sourceReference"] = 1000;
    json["arguments"]["source"]["sourceReference"] = 1000;
    json["arguments"]["source"]["path"] = "path";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SourceRequest* ptr = dynamic_cast<Dap::Protocol::SourceRequest*>(
            requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "source");
    EXPECT_EQ(ptr->arguments.sourceReference, 1000);
    EXPECT_EQ(ptr->arguments.source->sourceReference, 1000);
    EXPECT_EQ(ptr->arguments.source->path, "path");
}

TEST_F(RequestManagerTest, ReadMemoryRequestTest)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["memoryReference"] = "0x0000000000405536";
    argumentsJson["offset"] = 0;
    argumentsJson["count"] = 1;

    json_t readMemoryRequestJson;
    readMemoryRequestJson["arguments"] = argumentsJson;
    readMemoryRequestJson["command"] = REQ_RES_READ_MEMORY;
    readMemoryRequestJson["type"] = "request";
    readMemoryRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        readMemoryRequestJson.dump());
    auto ptr = dynamic_cast<ReadMemoryRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_READ_MEMORY);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.count, 1);
    EXPECT_EQ(ptr->arguments.offset, 0);
    EXPECT_EQ(ptr->arguments.memoryReference, "0x0000000000405536");
}

TEST_F(RequestManagerTest, ReadMemoryRequestTest_baseInfoError)
{
    json_t readMemoryRequestJson;
    readMemoryRequestJson["command"] = REQ_RES_READ_MEMORY;
    readMemoryRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        readMemoryRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, ReadMemoryRequestTest_noArgumentsError)
{
    json_t readMemoryRequestJson;
    readMemoryRequestJson["command"] = REQ_RES_READ_MEMORY;
    readMemoryRequestJson["type"] = "request";
    readMemoryRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        readMemoryRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, ReadMemoryRequestTest_noNecessaryArgumentsError)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["memoryReference"] = "0x0000000000405536";
    argumentsJson["offset"] = 0;

    json_t disassembleRequestJson;
    disassembleRequestJson["arguments"] = argumentsJson;
    disassembleRequestJson["command"] = REQ_RES_READ_MEMORY;
    disassembleRequestJson["type"] = "request";
    disassembleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        disassembleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, WriteMemoryRequestTest)
{
    json_t writeMemoryRequestJson;
    writeMemoryRequestJson["command"] = REQ_RES_WRITE_MEMORY;
    writeMemoryRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
        writeMemoryRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);

    writeMemoryRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr2 = RequestManager::Instance().FromJson(
        writeMemoryRequestJson.dump());
    EXPECT_EQ(requestPtr2, nullptr);

    json_t argumentsJson = json_t::object();
    argumentsJson["memoryReference"] = "0x0000000000405536";
    argumentsJson["offset"] = 0;
    writeMemoryRequestJson["arguments"] = argumentsJson;
    std::unique_ptr<Dap::Protocol::Request> requestPtr3 = RequestManager::Instance().FromJson(
            writeMemoryRequestJson.dump());
    EXPECT_EQ(requestPtr3, nullptr);

    argumentsJson["allowPartial"] = true;
    argumentsJson["data"] = "a1";
    writeMemoryRequestJson["arguments"] = argumentsJson;
    std::unique_ptr<Dap::Protocol::Request> requestPtr4 = RequestManager::Instance().FromJson(
            writeMemoryRequestJson.dump());
    auto ptr = dynamic_cast<WriteMemoryRequest*>(requestPtr4.get());
    EXPECT_EQ(ptr->command, REQ_RES_WRITE_MEMORY);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.offset, 0);
    EXPECT_EQ(ptr->arguments.memoryReference, "0x0000000000405536");
    EXPECT_EQ(ptr->arguments.allowPartial, true);
    EXPECT_EQ(ptr->arguments.data, "a1");
}

TEST_F(RequestManagerTest, SetInstructionBreakpointstest)
{
    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    jsonBreakpoint["enable"] = true;
    jsonBreakpoint["id"] = "a8b8d01a-be53-4dd3-b5cc-530414e87a98";
    jsonBreakpoint["instructionReference"] = "0x000000000040177e";
    jsonBreakpoint["offset"] = 0;
    jsonBreakpoint["condition"] = "none";
    jsonBreakpoint["hitCondition"] = "none";
    jsonBreakpoint["canPersist"] = false;
    jsonBreakpoints.push_back(jsonBreakpoint);

    json_t SetInstructionBreakpointRequestJson;
    SetInstructionBreakpointRequestJson["arguments"]["breakpoints"] = jsonBreakpoints;
    SetInstructionBreakpointRequestJson["command"] = REQ_RES_SET_INSTRUCTION_BREAKPOINTS;
    SetInstructionBreakpointRequestJson["type"] = "request";
    SetInstructionBreakpointRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            SetInstructionBreakpointRequestJson.dump());
    auto ptr = dynamic_cast<SetInstructionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_SET_INSTRUCTION_BREAKPOINTS);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
}

TEST_F(RequestManagerTest, SetInstructionBreakpoints_baseInfoError)
{
    json_t SetInstructionBreakpointRequestJson;
    SetInstructionBreakpointRequestJson["command"] = REQ_RES_SET_INSTRUCTION_BREAKPOINTS;
    SetInstructionBreakpointRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            SetInstructionBreakpointRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, SetInstructionBreakpoints_noArgumentsError)
{
    json_t SetInstructionBreakpointRequestJson;
    SetInstructionBreakpointRequestJson["command"] = REQ_RES_SET_INSTRUCTION_BREAKPOINTS;
    SetInstructionBreakpointRequestJson["type"] = "request";
    SetInstructionBreakpointRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            SetInstructionBreakpointRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, GetBitmap_BaseInfoError)
{
    json_t json;
    json["command"] = REQ_RES_GET_BITMAP;
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, GetBitmap_ArgumentsError)
{
    json_t json;
    json["command"] = REQ_RES_GET_BITMAP;
    json["type"] = "request";
    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, GetBitmap_ArgumentsNoVarName)
{
    json_t json;
    json["command"] = REQ_RES_GET_BITMAP;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, GetBitmap_ArgumentsPass)
{
    json_t json;
    json["command"] = REQ_RES_GET_BITMAP;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    json["arguments"]["varName"] = "bitmap";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_NE(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, VariableHexView_BaseInfoError)
{
    json_t json;
    json["command"] = REQ_RES_VARIABLE_HEX_VIEW;
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, VariableHexView_ArgumentsError)
{
    json_t json;
    json["command"] = REQ_RES_VARIABLE_HEX_VIEW;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, VariableHexView_ArgumentsPass)
{
    json_t json;
    json["command"] = REQ_RES_VARIABLE_HEX_VIEW;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    json["arguments"]["isEnable"] = true;
    json["arguments"]["isArrayView"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_NE(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, CommandCompletion_ArgumentsError)
{
    json_t json;
    json["command"] = REQ_RES_COMMAND_COMPLETION;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerTest, CommandCompletion_ArgumentsPass)
{
    json_t json;
    json["command"] = REQ_RES_COMMAND_COMPLETION;
    json["type"] = "request";
    json["seq"] = 1;
    json["arguments"] = json_t::object();
    json["arguments"]["command"] = "type format";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    EXPECT_NE(requestPtr, nullptr);
}