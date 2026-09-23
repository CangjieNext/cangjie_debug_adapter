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

class RequestManagerExtendTest : public ProtocolSuite {
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

TEST_F(RequestManagerExtendTest, DebugInConsoleRequestTest)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["debugCommand"] = "-exec-next";

    json_t debugInConsoleRequestJson;
    debugInConsoleRequestJson["arguments"] = argumentsJson;
    debugInConsoleRequestJson["command"] = REQ_RES_DEBUG_IN_CONSOLE;
    debugInConsoleRequestJson["type"] = "request";
    debugInConsoleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            debugInConsoleRequestJson.dump());
    auto ptr = dynamic_cast<DebugInConsoleRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_DEBUG_IN_CONSOLE);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.debugCommand, "-exec-next");
}

TEST_F(RequestManagerExtendTest, DebugInConsoleRequestTest_baseInfoError)
{
    json_t debugInConsoleRequestJson;
    debugInConsoleRequestJson["command"] = REQ_RES_DEBUG_IN_CONSOLE;
    debugInConsoleRequestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            debugInConsoleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, DebugInConsoleRequestTest_noArgumentsError)
{
    json_t debugInConsoleRequestJson;
    debugInConsoleRequestJson["command"] = REQ_RES_DEBUG_IN_CONSOLE;
    debugInConsoleRequestJson["type"] = "request";
    debugInConsoleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            debugInConsoleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, DebugInConsoleRequestTest_noNecessaryArgumentsError)
{
    json_t argumentsJson = json_t::object();

    json_t debugInConsoleRequestJson;
    debugInConsoleRequestJson["arguments"] = argumentsJson;
    debugInConsoleRequestJson["command"] = REQ_RES_DISASSEMBLE;
    debugInConsoleRequestJson["type"] = "request";
    debugInConsoleRequestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            debugInConsoleRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, UndoTest)
{
    json_t undoRequestJson;
    undoRequestJson["command"] = REQ_RES_UNDO;
    undoRequestJson["seq"] = 3;
    undoRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    undoRequestJson["arguments"]["undoEnable"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            undoRequestJson.dump());
    auto ptr = dynamic_cast<UndoRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_UNDO);
    EXPECT_EQ(ptr->seq, 3);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.undoEnable, true);
}

TEST_F(RequestManagerExtendTest, UndoTest_BaseInfoError)
{
    json_t undoRequestJson;
    undoRequestJson["command"] = REQ_RES_UNDO;
    undoRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            undoRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, UndoTest_NoArgumentsError)
{
    json_t undoRequestJson;
    undoRequestJson["command"] = REQ_RES_UNDO;
    undoRequestJson["seq"] = 3;
    undoRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            undoRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, UndoTest_NoArguments_UndoEnableError)
{
    json_t undoRequestJson;
    undoRequestJson["command"] = REQ_RES_UNDO;
    undoRequestJson["seq"] = 3;
    undoRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    undoRequestJson["arguments"]["errorArgument"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            undoRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ReverseStepTest)
{
    json_t reverseStepRequestJson;
    reverseStepRequestJson["command"] = REQ_RES_REVERSE_STEP;
    reverseStepRequestJson["seq"] = 4;
    reverseStepRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    reverseStepRequestJson["arguments"]["threadId"] = 1;
    reverseStepRequestJson["arguments"]["granularity"] =
            Dap::Protocol::ENUM_TO_STR(SteppingGranularity::LINE).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseStepRequestJson.dump());
    auto ptr = dynamic_cast<ReverseStepRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_REVERSE_STEP);
    EXPECT_EQ(ptr->seq, 4);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.threadId, 1);
    EXPECT_EQ(ptr->arguments.granularity, SteppingGranularity::LINE);
}

TEST_F(RequestManagerExtendTest, ReverseStepTest_BaseInfoError)
{
    json_t reverseStepRequestJson;
    reverseStepRequestJson["command"] = REQ_RES_REVERSE_STEP;
    reverseStepRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseStepRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ReverseStepTest_NoArgumentsError)
{
    json_t reverseStepRequestJson;
    reverseStepRequestJson["command"] = REQ_RES_REVERSE_STEP;
    reverseStepRequestJson["seq"] = 4;
    reverseStepRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseStepRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, UndoTest_NoArguments_ThreadIdError)
{
    json_t reverseStepRequestJson;
    reverseStepRequestJson["command"] = REQ_RES_REVERSE_STEP;
    reverseStepRequestJson["seq"] = 4;
    reverseStepRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    reverseStepRequestJson["arguments"]["granularity"] =
            Dap::Protocol::ENUM_TO_STR(SteppingGranularity::LINE).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseStepRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ReverseContinueTest)
{
    json_t reverseContinueJson;
    reverseContinueJson["command"] = REQ_RES_REVERSE_CONTINUE;
    reverseContinueJson["seq"] = 5;
    reverseContinueJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    reverseContinueJson["arguments"]["threadId"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseContinueJson.dump());
    auto ptr = dynamic_cast<ReverseContinueRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_REVERSE_CONTINUE);
    EXPECT_EQ(ptr->seq, 5);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.threadId, 1);
}

TEST_F(RequestManagerExtendTest, ReverseContinueTest_NoArgumentsError)
{
    json_t reverseContinueJson;
    reverseContinueJson["command"] = REQ_RES_REVERSE_CONTINUE;
    reverseContinueJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseContinueJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ReverseContinueTest_NoArguments_ThreadIdError)
{
    json_t reverseContinueJson;
    reverseContinueJson["command"] = REQ_RES_REVERSE_CONTINUE;
    reverseContinueJson["seq"] = 5;
    reverseContinueJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    reverseContinueJson["arguments"]["errorArgument"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            reverseContinueJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, SetExecutionPointRequestTest_1)
{
    json_t json1;
    json1["arguments"]["instructionReference"] = "0x00d81914";
    json1["seq"] = 8;
    json1["command"] = "setExecutionPoint";
    json1["type"] = "request";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::SetExecutionPointRequest* ptr =
        dynamic_cast<Dap::Protocol::SetExecutionPointRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->seq, 8);
    EXPECT_EQ(ptr->command, "setExecutionPoint");
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.instructionReference, "0x00d81914");
}

TEST_F(RequestManagerExtendTest, SetExecutionPointRequestTest_2)
{
    json_t json1;
    json1["arguments"]["sourcePath"] = "main.cpp";
    json1["arguments"]["line"] = 88;
    json1["seq"] = 8;
    json1["command"] = "setExecutionPoint";
    json1["type"] = "request";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::SetExecutionPointRequest* ptr =
        dynamic_cast<Dap::Protocol::SetExecutionPointRequest*>(requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->seq, 8);
    EXPECT_EQ(ptr->command, "setExecutionPoint");
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.sourcePath, "main.cpp");
    EXPECT_EQ(ptr->arguments.line, 88);
}

TEST_F(RequestManagerExtendTest, SetExecutionPointRequestTest_baseJsonError_1)
{
    json_t json1;
    json1["arguments"]["instructionReference"] = "0x00d81914";
    json1["command"] = "setExecutionPoint";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::SetExecutionPointRequest* ptr =
        dynamic_cast<Dap::Protocol::SetExecutionPointRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerExtendTest, SetExecutionPointRequestTest_noInstructionReferenceOrSourcePathError_1)
{
    json_t json1;
    json1["arguments"];
    json1["seq"] = 8;
    json1["command"] = "setExecutionPoint";
    json1["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json1);
    Dap::Protocol::SetExecutionPointRequest* ptr =
        dynamic_cast<Dap::Protocol::SetExecutionPointRequest*>(requestPtr.get());
    EXPECT_TRUE(ptr == nullptr);
}

TEST_F(RequestManagerExtendTest,GotoRecordPointRequestTest)
{
    json_t gotoEventJson;
    gotoEventJson["command"] = "gotoRecordPoint";
    gotoEventJson["arguments"]["recordPointId"] = 1;
    gotoEventJson["seq"] = 3;
    gotoEventJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(gotoEventJson.dump());
    Dap::Protocol::GotoRecordPointRequest* ptr = dynamic_cast<Dap::Protocol::GotoRecordPointRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command,"gotoRecordPoint");
    EXPECT_EQ(ptr->type,ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq,3);
    EXPECT_EQ(ptr->arguments.recordPointId,1);
}

TEST_F(RequestManagerExtendTest,RecordPointsRequestTest)
{
    json_t stoppedEventsJson;
    json_t array = json_t::array();
    std::vector<std::string> vec = {"step","breakpoint"};
    array.push_back("step");
    array.push_back("breakpoint");
    stoppedEventsJson["command"] = "recordPoints";
    stoppedEventsJson["seq"] = 4;
    stoppedEventsJson["type"] = "request";
    stoppedEventsJson["arguments"]["filters"] = array;
    stoppedEventsJson["arguments"]["start"] = 1;
    stoppedEventsJson["arguments"]["count"] = 10;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(stoppedEventsJson.dump());
    Dap::Protocol::RecordPointsRequest* ptr = dynamic_cast<Dap::Protocol::RecordPointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command,"recordPoints");
    EXPECT_EQ(ptr->type,ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq,4);
    EXPECT_EQ(ptr->arguments.filters, vec);
    EXPECT_EQ(ptr->arguments.start, 1);
    EXPECT_EQ(ptr->arguments.count, 10);
}

TEST_F(RequestManagerExtendTest, ContinueInReverseTest)
{
    json_t continueInReverseJson;
    continueInReverseJson["command"] = REQ_RES_CONTINUE_IN_REVERSE;
    continueInReverseJson["seq"] = 5;
    continueInReverseJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    continueInReverseJson["arguments"]["threadId"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            continueInReverseJson.dump());
    auto ptr = dynamic_cast<ContinueInReverseRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_CONTINUE_IN_REVERSE);
    EXPECT_EQ(ptr->seq, 5);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.threadId, 1);
}

TEST_F(RequestManagerExtendTest, ContinueInReverseTest_NoArgumentsError)
{
    json_t continueInReverseJson;
    continueInReverseJson["command"] = REQ_RES_CONTINUE_IN_REVERSE;
    continueInReverseJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            continueInReverseJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ContinueInReverseest_NoArguments_ThreadIdError)
{
    json_t continueInReverseJson;
    continueInReverseJson["command"] = REQ_RES_CONTINUE_IN_REVERSE;
    continueInReverseJson["seq"] = 5;
    continueInReverseJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    continueInReverseJson["arguments"]["errorArgument"] = false;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            continueInReverseJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, StepInReverseTest)
{
    json_t stepInReverseRequestJson;
    stepInReverseRequestJson["command"] = REQ_RES_STEP_IN_REVERSE;
    stepInReverseRequestJson["seq"] = 4;
    stepInReverseRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    stepInReverseRequestJson["arguments"]["threadId"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            stepInReverseRequestJson.dump());
    auto ptr = dynamic_cast<StepInReverseRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_STEP_IN_REVERSE);
    EXPECT_EQ(ptr->seq, 4);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.threadId, 1);
}

TEST_F(RequestManagerExtendTest, StepInReverseTest_BaseInfoError)
{
    json_t stepInReverseRequestJson;
    stepInReverseRequestJson["command"] = REQ_RES_STEP_IN_REVERSE;
    stepInReverseRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            stepInReverseRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, StepInReverseTest_NoArgumentsError)
{
    json_t stepInReverseRequestJson;
    stepInReverseRequestJson["command"] = REQ_RES_STEP_IN_REVERSE;
    stepInReverseRequestJson["seq"] = 4;
    stepInReverseRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            stepInReverseRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, StepInReverseTest_NoArguments_ThreadIdError)
{
    json_t stepInReverseRequestJson;
    stepInReverseRequestJson["command"] = REQ_RES_STEP_IN_REVERSE;
    stepInReverseRequestJson["seq"] = 4;
    stepInReverseRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    stepInReverseRequestJson["arguments"]["granularity"] =
            Dap::Protocol::ENUM_TO_STR(SteppingGranularity::LINE).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            stepInReverseRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, CacheDataConfigTest)
{
    json_t cacheDataConfigRequestJson;
    cacheDataConfigRequestJson["command"] = REQ_RES_CACHE_DATA_CONFIG;
    cacheDataConfigRequestJson["seq"] = 1;
    cacheDataConfigRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    cacheDataConfigRequestJson["arguments"]["startCacheData"] = true;
    cacheDataConfigRequestJson["arguments"]["saveHistoryCache"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            cacheDataConfigRequestJson.dump());
    auto ptr = dynamic_cast<CacheDataConfigRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_CACHE_DATA_CONFIG);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.startCacheData, true);
    EXPECT_EQ(ptr->arguments.saveHistoryCache, true);
}

TEST_F(RequestManagerExtendTest,CacheDataConfigTest_BaseInfoError)
{
    json_t cacheDataConfigRequestJson;
    cacheDataConfigRequestJson["command"] = REQ_RES_CACHE_DATA_CONFIG;
    cacheDataConfigRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            cacheDataConfigRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, CacheDataConfigTest_NoArgumentsError)
{
    json_t cacheDataConfigRequestJson;
    cacheDataConfigRequestJson["command"] = REQ_RES_CACHE_DATA_CONFIG;
    cacheDataConfigRequestJson["seq"] = 3;
    cacheDataConfigRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            cacheDataConfigRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, CacheDataConfigTest_NoArguments_CacheDataConfigError)
{
    json_t cacheDataConfigRequestJson;
    cacheDataConfigRequestJson["command"] = REQ_RES_CACHE_DATA_CONFIG;
    cacheDataConfigRequestJson["seq"] = 1;
    cacheDataConfigRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    cacheDataConfigRequestJson["arguments"]["errorArgument"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            cacheDataConfigRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, RecordMessageTest)
{
    json_t recordMessageRequestJson;
    recordMessageRequestJson["command"] = REQ_RES_RECORD_MESSAGE;
    recordMessageRequestJson["seq"] = 1;
    recordMessageRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    recordMessageRequestJson["arguments"]["recordPointIds"] = { 1, 3 };
    recordMessageRequestJson["arguments"]["requireThread"] = true;
    recordMessageRequestJson["arguments"]["requireStackFrame"] = true;
    recordMessageRequestJson["arguments"]["requireLocalVariables"] = true;
    recordMessageRequestJson["arguments"]["requireGlobalVariables"] = true;
    recordMessageRequestJson["arguments"]["requireStaticVariables"] = true;
    json_t monitorVariableJson;
    monitorVariableJson["id"] = 1;
    monitorVariableJson["name"] = "a";
    monitorVariableJson["scopeName"] = "locals";
    monitorVariableJson["type"] = "int";
    monitorVariableJson["value"] = "12";
    recordMessageRequestJson["arguments"]["monitorVariables"] = { monitorVariableJson };
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            recordMessageRequestJson.dump());
    auto ptr = dynamic_cast<RecordMessageRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_RECORD_MESSAGE);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->arguments.recordPointIds.at(0), 1);
    EXPECT_EQ(ptr->arguments.recordPointIds.at(1), 3);
    EXPECT_TRUE(ptr->arguments.requireThread);
    EXPECT_TRUE(ptr->arguments.requireStackFrame);
    EXPECT_TRUE(ptr->arguments.requireLocalVariables);
    EXPECT_TRUE(ptr->arguments.requireGlobalVariables);
    EXPECT_TRUE(ptr->arguments.requireStaticVariables);
    EXPECT_EQ(ptr->arguments.monitorVariables.value().at(0).id, 1);
    EXPECT_EQ(ptr->arguments.monitorVariables.value().at(0).name, "a");
    EXPECT_EQ(ptr->arguments.monitorVariables.value().at(0).scopeName, "locals");
    EXPECT_EQ(ptr->arguments.monitorVariables.value().at(0).type, "int");
    EXPECT_EQ(ptr->arguments.monitorVariables.value().at(0).value, "12");
}

TEST_F(RequestManagerExtendTest, RecordMessageTest_BaseInfoError)
{
    json_t recordMessageRequestJson;
    recordMessageRequestJson["command"] = REQ_RES_RECORD_MESSAGE;
    recordMessageRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            recordMessageRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, RecordMessageTest_ArgumentsError)
{
    json_t recordMessageRequestJson;
    recordMessageRequestJson["command"] = REQ_RES_RECORD_MESSAGE;
    recordMessageRequestJson["seq"] = 1;
    recordMessageRequestJson["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    recordMessageRequestJson["arguments"]["recordPointIds"] = 12;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(
            recordMessageRequestJson.dump());
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, DebugInConsoleRequestTest_noDebugCommand)
{
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "debugInConsole";
    json["arguments"]["abc"] = "";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, ReverseStepRequestTest_requestError)
{
    json_t json;
    json["type"] = "request";
    json["seq"] = 1;
    json["command"] = REQ_RES_REVERSE_STEP;
    json["arguments"]["threadId"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto ptr = dynamic_cast<ReverseStepRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->seq, 1);
}

TEST_F(RequestManagerExtendTest, GotoRecordPointRequestTest_baseInfoError)
{
    json_t json;
    json["command"] = "gotoRecordPoint";
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);

    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    auto ptr1 = dynamic_cast<GotoRecordPointRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr1->seq, 1);
}

TEST_F(RequestManagerExtendTest, CacheDataConfigRequestTest_saveHistoryCacheError)
{
    json_t json;
    json["seq"] = 1;
    json["type"] = "request";
    json["command"] = "cacheDataConfig";
    json["arguments"]["startCacheData"] = true;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, RecordMessageRequestTest_requestError)
{
    json_t json;
    json["command"] = REQ_RES_RECORD_MESSAGE;
    json["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);

    json["seq"] = 1;
    json["arguments"]["recordPointIds"] = { 1, 3 };
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    auto ptr1 = dynamic_cast<RecordMessageRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr1->seq, 1);
}

TEST_F(RequestManagerExtendTest, SourceRequestTest_baseInfoError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "source";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);

    json["seq"] = 1;
    json["arguments"]["sourceReference"] = 1000;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SourceRequest* ptr = dynamic_cast<Dap::Protocol::SourceRequest*>(requestSecPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}

TEST_F(RequestManagerExtendTest, SourceRequestTest_noArgumentsError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "source";

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, SourceRequestTest_noSourceError)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonObject["path"] = getenv("PATH");
        jsonObject["name"] = "fdfdsfd";
        jsonObject["origin"] = "abc";
        jsonArray.push_back(jsonObject);
    }

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "source";
    json["arguments"]["sourceReference"] = 1000;
    json["arguments"]["source"]["sourceReference"] = 1000;
    json["arguments"]["source"]["path"] = "path";

    json_t checksumsArray = json_t::array();
    json_t checksumsObject = json_t::object();
    checksumsObject["algorithm"] = ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value();
    checksumsArray.push_back(std::move(checksumsObject));
    json["arguments"]["source"]["checksums"] = checksumsArray;

    std::unique_ptr<Dap::Protocol::Request> requestFirPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestFirPtr, nullptr);

    checksumsArray.clear();
    checksumsObject.clear();
    checksumsObject["checksum"] = "checksum";
    checksumsArray.push_back(std::move(checksumsObject));
    json["arguments"]["source"]["checksums"] = checksumsArray;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestSecPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, EvaluateRequestTest_baseInfoError)
{
    json_t requestJson;
    requestJson["command"] = REQ_RES_EVALUATE;
    requestJson["type"] = "request";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(requestJson);
    EXPECT_EQ(requestPtr, nullptr);

    json_t argumentsJson = json_t::object();
    argumentsJson["expression"] = "a == 1";
    argumentsJson["frameId"] = 1000;
    requestJson["seq"] = 1;
    requestJson["arguments"] = argumentsJson;
    std::unique_ptr<Dap::Protocol::Request> requestSecPtr = RequestManager::Instance().FromJson(requestJson.dump());
    auto ptr = dynamic_cast<EvaluateRequest*>(requestSecPtr.get());
    EXPECT_EQ(ptr->seq, 1);
}

TEST_F(RequestManagerExtendTest, EvaluateRequestTest_noArgumentsError)
{
    json_t requestJson;
    requestJson["command"] = REQ_RES_EVALUATE;
    requestJson["type"] = "request";
    requestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(requestJson);
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, EvaluateRequestTest_noNecessaryArgumentsError)
{
    json_t argumentsJson = json_t::object();
    argumentsJson["frameId"] = 1000;
    argumentsJson["context"] = "watch";
    argumentsJson["format"]["hex"] = true;

    json_t requestJson;
    requestJson["arguments"] = argumentsJson;
    requestJson["command"] = REQ_RES_EVALUATE;
    requestJson["type"] = "request";
    requestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(requestJson);
    EXPECT_EQ(requestPtr, nullptr);
}


TEST_F(RequestManagerExtendTest, setDataBreakpointsRequestTest_noArgumentsError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "setDataBreakpoints";
    json["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_TRUE(requestPtr == nullptr);
}

TEST_F(RequestManagerExtendTest, setDataBreakpointsRequestTest_noAccessTypeError)
{
    json_t breakpoints = json_t::array();
    json_t breakpoint = json_t::object();
    breakpoint["dataId"] = "dataId";
    breakpoint["condition"] = "condition";
    breakpoint["hitCondition"] = "hitCondition";
    breakpoints.push_back(breakpoint);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["command"] = "setDataBreakpoints";
    json["seq"] = 1;
    json["arguments"]["breakpoints"] = breakpoints;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    Dap::Protocol::SetDataBreakpointsRequest* ptr = dynamic_cast<Dap::Protocol::SetDataBreakpointsRequest*>(
            requestPtr.get());
    EXPECT_FALSE(ptr == nullptr);
}
TEST_F(RequestManagerExtendTest, setFunctionBreakpointsRequestTest_noArgumentsError)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setFunctionBreakpoints";
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    EXPECT_EQ(requestPtr, nullptr);
}

TEST_F(RequestManagerExtendTest, setFunctionBreakpointsRequestTest_noNameError)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    jsonObject["condition"] = "condition";
    jsonObject["hitCondition"] = "hitCondition";
    jsonArray.push_back(jsonObject);

    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "setFunctionBreakpoints";
    json["arguments"]["breakpoints"] = jsonArray;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json.dump());
    auto ptr = dynamic_cast<SetFunctionBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, "setFunctionBreakpoints");
}

TEST_F(RequestManagerExtendTest, SetInstructionBreakpoints_noInstructionReference)
{
    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    jsonBreakpoint["enable"] = true;
    jsonBreakpoint["id"] = "a8b8d01a-be53-4dd3-b5cc-530414e87a98";
    jsonBreakpoint["offset"] = 0;
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
}

TEST_F(RequestManagerExtendTest, SetMemoryBreakpoints)
{
    json_t jsonBreakpoints = json_t::array();
    json_t jsonBreakpoint = json_t::object();
    jsonBreakpoint["address"] = "0x0000a432";
    jsonBreakpoint["accessType"] = "readWrite";
    jsonBreakpoints.push_back(jsonBreakpoint);

    json_t requestJson;
    requestJson["arguments"]["breakpoints"] = jsonBreakpoints;
    requestJson["command"] = REQ_RES_SET_MEMORY_BREAKPOINTS;
    requestJson["type"] = "request";
    requestJson["seq"] = 1;
    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(requestJson.dump());
    auto ptr = dynamic_cast<SetMemoryBreakpointsRequest*>(requestPtr.get());
    EXPECT_EQ(ptr->command, REQ_RES_SET_MEMORY_BREAKPOINTS);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->seq, 1);
    EXPECT_EQ(ptr->arguments.breakpoints[0].address, "0x0000a432");
    EXPECT_EQ(ptr->arguments.breakpoints[0].accessType, DataBreakpointAccessType::READ_WRITE);
}

TEST_F(RequestManagerExtendTest, addSourceBreakpointRequestTest)
{
    json_t jsonArray = json_t::array();
    json_t jsonObject = json_t::object();
    for (int i = 0; i < arraySize; i++) {
        jsonObject["path"] = getenv("PATH");
        jsonObject["name"] = "fdfdsfd";
        jsonArray.push_back(jsonObject);
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
    json["command"] = "addSourceBreakpoint";
    json["arguments"]["source"]["path"] = getenv("PATH");
    json["arguments"]["source"]["sources"] = jsonArray;
    json["arguments"]["source"]["checksums"] = checksumsArray;
    json["arguments"]["source"]["presentationHint"] = ENUM_TO_STR(PresentationHint::DEEMPHASIZE).value();
    json["arguments"]["source"]["adapterData"] = "";
    json["arguments"]["line"] = 12;
    json["arguments"]["column"] = 12;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto* ptr = dynamic_cast<Dap::Protocol::AddSourceBreakpointRequest*>(requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "addSourceBreakpoint");
    EXPECT_EQ(ptr->arguments.line, 12);
    EXPECT_EQ(ptr->arguments.source.checksums->at(0).algorithm, ChecksumAlgorithmType::SHA256);
    EXPECT_EQ(ptr->arguments.source.checksums->at(1).algorithm, ChecksumAlgorithmType::SHA1);
}

TEST_F(RequestManagerExtendTest, deleteBreakpointRequestTest)
{
    json_t json;
    json["type"] = Dap::Protocol::ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value();
    json["seq"] = 1;
    json["command"] = "deleteBreakpoint";
    json["arguments"]["id"] = 12;

    std::unique_ptr<Dap::Protocol::Request> requestPtr = RequestManager::Instance().FromJson(json);
    auto* ptr = dynamic_cast<Dap::Protocol::DeleteBreakpointRequest*>(requestPtr.get());

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_EQ(ptr->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(ptr->command, "deleteBreakpoint");
    EXPECT_EQ(ptr->arguments.id, 12);
}