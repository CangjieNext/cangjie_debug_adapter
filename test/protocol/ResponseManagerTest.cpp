// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "ResponseManager.h"
#include "EnumUtils.h"
#include "ProtocolSuite.h"
#include "TypeUtils.h"

using namespace Dap::Protocol;

class ResponseManagerTest : public ProtocolSuite {
    void SetUp() override
    {
        ResponseManager::Instance().Register();
        EXPECT_TRUE(ResponseManager::Instance().IsRegister());
    }
    void TearDown() override
    {
        ResponseManager::Instance().UnRegister();
    }

protected:
    int arraySize = 10;
};

TEST_F(ResponseManagerTest, ResponseTest)
{
    Response response("aaa");
    response.seq = 3;
    response.message = "message";
    json_t json = ToResponseJson(response).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "aaa");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, ResponseTestCommandError)
{
    json_t json;
    json["seq"] = 2;
    json["message"] = "message";
    json["type"] = RESPONSE_NAME;
    std::unique_ptr<Dap::Protocol::Response> responsePtr = ResponseManager::Instance().ToResponse(json.dump());
    EXPECT_TRUE(responsePtr == nullptr);
}

TEST_F(ResponseManagerTest, errorResponseTest)
{
    ErrorResponse errorResponse("continue");
    errorResponse.seq = 3;
    errorResponse.message = "message";
    json_t json = ToResponseJson(errorResponse).value();
    EXPECT_FALSE(json.is_null());

    Dap::Protocol::Message message;
    message.id = 1;
    message.format = "utf-8";
    errorResponse.error = std::move(message);
    json = ToResponseJson(errorResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "continue");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["body"]["error"]["id"], 1);
    EXPECT_EQ(json["body"]["error"]["format"], "utf-8");
}

TEST_F(ResponseManagerTest, errorResponseTest_EX)
{
    ErrorResponse errorResponse("continue");
    errorResponse.seq = 3;
    errorResponse.message = "message";
    Dap::Protocol::Message message;
    message.id = 1;
    message.format = "utf-8";
    errorResponse.error = std::move(message);
    json_t json = ResponseManager::Instance().ToJson(errorResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "continue");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["body"]["error"]["id"], 1);
    EXPECT_EQ(json["body"]["error"]["format"], "utf-8");
}

TEST_F(ResponseManagerTest, initializeResponseTest)
{
    InitializeResponse initializeResponse;
    initializeResponse.seq = 3;
    json_t json = ToResponseJson(initializeResponse).value();

    Capabilities capabilities;
    std::vector<std::string> vector;
    vector.push_back("faf");
    vector.push_back("bbb");
    capabilities.completionTriggerCharacters = vector;
    initializeResponse.body = capabilities;
    json = ToResponseJson(initializeResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "initialize");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["completionTriggerCharacters"].at(0), "faf");
    EXPECT_EQ(json["seq"], 3);

    capabilities.supportedTimeTravelRecordPointTypes = vector;
    initializeResponse.body = capabilities;
    json = ToResponseJson(initializeResponse).value();
    EXPECT_EQ(json["body"]["supportedTimeTravelRecordPointTypes"].at(0), "faf");
}

TEST_F(ResponseManagerTest, initializeResponseTest_EX)
{
    InitializeResponse initializeResponse;
    initializeResponse.seq = 3;
    Capabilities capabilities;
    std::vector<std::string> vector;
    vector.push_back("faf");
    vector.push_back("bbb");
    capabilities.completionTriggerCharacters = vector;
    capabilities.supportsWriteMemoryRequest = true;
    initializeResponse.body = capabilities;
    json_t json = ResponseManager::Instance().ToJson(initializeResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "initialize");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["completionTriggerCharacters"].at(0), "faf");
    EXPECT_EQ(json["body"]["supportsWriteMemoryRequest"], true);
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, configurationDoneResponseTest)
{
    ConfigurationDoneResponse configurationDoneResponse;
    configurationDoneResponse.seq = 3;
    json_t json = Dap::Protocol::ToResponseJson(configurationDoneResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "configurationDone");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, configurationDoneResponseTest_EX)
{
    ConfigurationDoneResponse configurationDoneResponse;
    configurationDoneResponse.seq = 3;
    json_t json = ResponseManager::Instance().ToJson(configurationDoneResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "configurationDone");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, attachResponseTest)
{
    AttachResponse attachResponse;
    attachResponse.seq = 3;
    json_t json = Dap::Protocol::ToResponseJson(attachResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "attach");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, attachResponseTest_EX)
{
    AttachResponse attachResponse;
    attachResponse.seq = 3;
    json_t json = ResponseManager::Instance().ToJson(attachResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "attach");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
}

TEST_F(ResponseManagerTest, launchResponseTest)
{
    LaunchResponse launchResponse;
    launchResponse.seq = 5;
    json_t json = Dap::Protocol::ToResponseJson(launchResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "launch");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 5);
}

TEST_F(ResponseManagerTest, launchResponseTest_EX)
{
    LaunchResponse launchResponse;
    launchResponse.seq = 5;
    json_t json = ResponseManager::Instance().ToJson(launchResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "launch");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 5);
}

TEST_F(ResponseManagerTest, disconnectResponseTest)
{
    DisconnectResponse disconnectResponse;
    disconnectResponse.seq = 3;
    disconnectResponse.message = "message";

    json_t json = Dap::Protocol::ToResponseJson(disconnectResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "disconnect");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, disconnectResponseTest_EX)
{
    DisconnectResponse disconnectResponse;
    disconnectResponse.seq = 3;
    disconnectResponse.message = "message";

    json_t json = ResponseManager::Instance().ToJson(disconnectResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "disconnect");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, terminateResponseTest)
{
    TerminateResponse terminateResponse;
    terminateResponse.seq = 3;
    terminateResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(terminateResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "terminate");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, terminateResponseTest_EX)
{
    TerminateResponse terminateResponse;
    terminateResponse.seq = 3;
    terminateResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(terminateResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "terminate");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, breakpointLocationsResponseTest)
{
    BreakpointLocationsResponse breakpointLocationsResponse;
    breakpointLocationsResponse.message = "message";
    breakpointLocationsResponse.seq = 3;
    std::vector<BreakpointLocation> breakpoints;
    BreakpointLocation breakpointLocation;
    breakpointLocation.line = 5;
    breakpoints.push_back(std::move(breakpointLocation));
    breakpointLocationsResponse.breakpoints = std::move(breakpoints);
    json_t json = Dap::Protocol::ToResponseJson(breakpointLocationsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "breakpointLocations");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["line"], 5);
}

TEST_F(ResponseManagerTest, breakpointLocationsResponseTest_EX)
{
    BreakpointLocationsResponse breakpointLocationsResponse;
    breakpointLocationsResponse.message = "message";
    breakpointLocationsResponse.seq = 3;
    std::vector<BreakpointLocation> breakpoints;
    BreakpointLocation breakpointLocation;
    breakpointLocation.line = 5;
    breakpoints.push_back(std::move(breakpointLocation));
    breakpointLocationsResponse.breakpoints = std::move(breakpoints);
    json_t json = ResponseManager::Instance().ToJson(breakpointLocationsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "breakpointLocations");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["line"], 5);
}

TEST_F(ResponseManagerTest, setBreakpointsResponseTest)
{
    SetBreakpointsResponse setBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        Source source;
        source.name = "source";
        source.presentationHint = PresentationHint::EMPHASIZE;
        std::vector<Checksum> checksums;
        Checksum checksum;
        checksum.checksum = "checksum";
        checksum.algorithm = ChecksumAlgorithmType::SHA256;
        checksums.push_back(std::move(checksum));
        source.checksums = std::move(checksums);
        breakpoint.verified = true;
        breakpoint.source = std::move(source);
        breakpoints.push_back(std::move(breakpoint));
    }

    setBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = Dap::Protocol::ToResponseJson(setBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, setBreakpointsResponseTest_EX)
{
    SetBreakpointsResponse setBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        breakpoint.verified = true;
        breakpoints.push_back(std::move(breakpoint));
    }

    setBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = ResponseManager::Instance().ToJson(setBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, setFunctionBreakpointsResponseTest)
{
    SetFunctionBreakpointsResponse setFunctionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        Source source;
        source.name = "source";
        source.presentationHint = PresentationHint::EMPHASIZE;
        std::vector<Checksum> checksums;
        Checksum checksum;
        checksum.checksum = "checksum";
        checksum.algorithm = ChecksumAlgorithmType::SHA256;
        checksums.push_back(std::move(checksum));
        source.checksums = std::move(checksums);
        breakpoint.verified = true;
        breakpoint.source = std::move(source);
        breakpoints.push_back(std::move(breakpoint));
    }

    setFunctionBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = Dap::Protocol::ToResponseJson(setFunctionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setFunctionBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, setFunctionBreakpointsResponseTest_EX)
{
    SetFunctionBreakpointsResponse setFunctionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        breakpoint.verified = true;
        breakpoints.push_back(std::move(breakpoint));
    }

    setFunctionBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = ResponseManager::Instance().ToJson(setFunctionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setFunctionBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, setExceptionBreakpointsResponseTest)
{
    SetExceptionBreakpointsResponse setExceptionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        Source source;
        source.name = "source";
        source.presentationHint = PresentationHint::EMPHASIZE;
        std::vector<Checksum> checksums;
        Checksum checksum;
        checksum.checksum = "checksum";
        checksum.algorithm = ChecksumAlgorithmType::SHA256;
        checksums.push_back(std::move(checksum));
        source.checksums = std::move(checksums);
        breakpoint.verified = true;
        breakpoint.source = std::move(source);
        breakpoints.push_back(std::move(breakpoint));
    }

    setExceptionBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = Dap::Protocol::ToResponseJson(setExceptionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setExceptionBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, setExceptionBreakpointsResponseTest_EX)
{
    SetExceptionBreakpointsResponse setExceptionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        breakpoint.verified = true;
        breakpoints.push_back(std::move(breakpoint));
    }

    setExceptionBreakpointsResponse.breakpoints = std::move(breakpoints);
    json_t json = ResponseManager::Instance().ToJson(setExceptionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setExceptionBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, dataBreakpointInfoResponseTest)
{
    DataBreakpointInfoResponse dataBreakpointInfoResponse;
    dataBreakpointInfoResponse.message = "message";
    dataBreakpointInfoResponse.seq = 3;
    dataBreakpointInfoResponse.dataId = "1";
    dataBreakpointInfoResponse.description = "abc";
    json_t json = Dap::Protocol::ToResponseJson(dataBreakpointInfoResponse).value();
    EXPECT_FALSE(json.is_null());
}

TEST_F(ResponseManagerTest, continueResponseTest)
{
    ContinueResponse continueResponse;
    continueResponse.message = "message";
    continueResponse.success = true;
    json_t json = Dap::Protocol::ToResponseJson(continueResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "continue");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, continueResponseTest_EX)
{
    ContinueResponse continueResponse;
    continueResponse.message = "message";
    continueResponse.success = true;
    json_t json = ResponseManager::Instance().ToJson(continueResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "continue");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, nextResponseTest)
{
    NextResponse nextResponse;
    nextResponse.message = "message";
    nextResponse.success = true;
    json_t json = Dap::Protocol::ToResponseJson(nextResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "next");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, nextResponseTest_EX)
{
    NextResponse nextResponse;
    nextResponse.message = "message";
    nextResponse.success = true;
    json_t json = ResponseManager::Instance().ToJson(nextResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "next");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, stepInResponseTest)
{
    StepInResponse stepInResponse;
    stepInResponse.message = "message";
    stepInResponse.success = true;
    stepInResponse.seq = 7;
    json_t json = Dap::Protocol::ToResponseJson(stepInResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stepIn");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 7);
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, stepInResponseTest_EX)
{
    StepInResponse stepInResponse;
    stepInResponse.message = "message";
    stepInResponse.success = true;
    stepInResponse.seq = 7;
    json_t json = ResponseManager::Instance().ToJson(stepInResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stepIn");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 7);
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, stepOutResponseTest)
{
    StepOutResponse stepOutResponse;
    stepOutResponse.message = "message";
    stepOutResponse.success = true;
    stepOutResponse.seq = 8;
    json_t json = Dap::Protocol::ToResponseJson(stepOutResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stepOut");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 8);
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, stepOutResponseTest_EX)
{
    StepOutResponse stepOutResponse;
    stepOutResponse.message = "message";
    stepOutResponse.success = true;
    stepOutResponse.seq = 8;
    json_t json = ResponseManager::Instance().ToJson(stepOutResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stepOut");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 8);
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, stackTraceResponseTest)
{
    StackTraceResponse stackTraceResponse;
    stackTraceResponse.message = "message";
    stackTraceResponse.success = true;
    std::vector<StackFrame> stackFrames;
    StackFrame stackFrame;
    stackFrame.id = 3004;
    stackFrame.name = "stack-1";
    stackFrame.line = 4;
    stackFrame.column = 6;
    stackFrame.endLine = 7;
    stackFrame.endColumn = 9;
    stackFrame.moduleId = 1;
    stackFrame.presentationHint = StackFramePresentationHint::LABEL;
    stackFrames.push_back(std::move(stackFrame));
    stackTraceResponse.stackFrames = std::move(stackFrames);
    stackTraceResponse.totalFrames = 1;
    json_t json = Dap::Protocol::ToResponseJson(stackTraceResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stackTrace");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["id"], 3004);
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["name"], "stack-1");
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["presentationHint"], ENUM_TO_STR(StackFramePresentationHint::LABEL));
}

TEST_F(ResponseManagerTest, stackTraceResponseTest_EX)
{
    StackTraceResponse stackTraceResponse;
    stackTraceResponse.message = "message";
    stackTraceResponse.success = true;
    std::vector<StackFrame> stackFrames;
    StackFrame stackFrame;
    stackFrame.id = 3004;
    stackFrame.name = "stack-1";
    stackFrame.line = 4;
    stackFrame.column = 6;
    stackFrame.endLine = 7;
    stackFrame.endColumn = 9;
    stackFrame.moduleId = 1;
    stackFrame.presentationHint = StackFramePresentationHint::LABEL;
    stackFrame.symbolInstructionRange = "0x00a234c4-0x00a234d0";
    stackFrame.instructionPointerReference = "0x00a234c4";
    stackFrames.push_back(std::move(stackFrame));
    stackTraceResponse.stackFrames = std::move(stackFrames);
    stackTraceResponse.totalFrames = 1;
    json_t json = ResponseManager::Instance().ToJson(stackTraceResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "stackTrace");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["id"], 3004);
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["name"], "stack-1");
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["presentationHint"], ENUM_TO_STR(StackFramePresentationHint::LABEL));
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["symbolInstructionRange"], "0x00a234c4-0x00a234d0");
    EXPECT_EQ(json["body"]["stackFrames"].at(0)["instructionPointerReference"], "0x00a234c4");
}

TEST_F(ResponseManagerTest, threadsResponseTest)
{
    ThreadsResponse threadsResponse;
    threadsResponse.message = "message";
    threadsResponse.success = true;
    std::vector<Thread> threads;
    Thread thread;
    thread.id = 3004;
    thread.name = "thread-1";
    threads.push_back(std::move(thread));
    threadsResponse.threads = std::move(threads);
    json_t json = Dap::Protocol::ToResponseJson(threadsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_THREADS);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["threads"].at(0)["id"], 3004);
    EXPECT_EQ(json["body"]["threads"].at(0)["name"], "thread-1");
}

TEST_F(ResponseManagerTest, threadsResponseTest_EX)
{
    ThreadsResponse threadsResponse;
    threadsResponse.message = "message";
    threadsResponse.success = true;
    std::vector<Thread> threads;
    Thread thread;
    thread.id = 3004;
    thread.name = "thread-1";
    threads.push_back(std::move(thread));
    threadsResponse.threads = std::move(threads);
    json_t json = ResponseManager::Instance().ToJson(threadsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_THREADS);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["threads"].at(0)["id"], 3004);
    EXPECT_EQ(json["body"]["threads"].at(0)["name"], "thread-1");
}

TEST_F(ResponseManagerTest, scopesResponseTest)
{
    ScopesResponse scopesResponse;
    scopesResponse.success = true;
    scopesResponse.command = REQ_RES_SCOPES;
    scopesResponse.type = ProtocolMessage::Type::RESPONSE;
    Scope scope;
    scope.variablesReference = 1001;
    scope.name = "Locals";
    scope.expensive = true;
    scope.presentationHint = "locals";
    std::vector<Scope> scopes;
    scopes.push_back(scope);
    scopesResponse.scopes = scopes;
    json_t json = ResponseManager::Instance().ToJson(scopesResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_SCOPES);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["scopes"].at(0)["variablesReference"], 1001);
    EXPECT_EQ(json["body"]["scopes"].at(0)["name"], "Locals");
    EXPECT_TRUE(json["body"]["scopes"].at(0)["expensive"]);
    EXPECT_EQ(json["body"]["scopes"].at(0)["presentationHint"], "locals");
}

TEST_F(ResponseManagerTest, variablesResponseTest)
{
    VariablesResponse variablesResponse;
    variablesResponse.command = REQ_RES_VARIABLES;
    variablesResponse.seq = 12;
    variablesResponse.type = ProtocolMessage::Type::RESPONSE;
    json_t json = ResponseManager::Instance().ToJson(variablesResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_VARIABLES);

    variablesResponse.variablesCount = 10;
    std::vector<Variable> vector;

    Variable variable1;
    variable1.type = "std::string";
    variable1.variablesReference = 0;
    variable1.value = "Jack";
    variable1.name = "name";
    variable1.evaluateName = "var0";
    variable1.declaredLine = 11;
    Source source;
    source.path = "d:/abc/a.cpp";
    variable1.declaredSource = source;
    vector.push_back(variable1);

    Variable variable2;
    variable2.type = "void (*)()";
    variable2.variablesReference = 0;
    variable2.value = "0x0000000000401623";
    variable2.name = "functionPointer";
    variable2.evaluateName = "var1";
    variable2.memoryReference = "0x0000000000401623";
    variable2.memoryAddress = "0x0000000000401623";
    vector.push_back(variable2);

    variablesResponse.variables = vector;
    variablesResponse.success = true;
    json = ResponseManager::Instance().ToJson(variablesResponse).value();
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["variablesCount"], 10);

    EXPECT_EQ(json["body"]["variables"].at(0)["type"], "std::string");
    EXPECT_EQ(json["body"]["variables"].at(1)["variablesReference"], 0);
    EXPECT_EQ(json["body"]["variables"].at(0)["value"], "Jack");
    EXPECT_EQ(json["body"]["variables"].at(0)["name"], "name");
    EXPECT_EQ(json["body"]["variables"].at(0)["evaluateName"], "var0");
    EXPECT_EQ(json["body"]["variables"].at(0)["declaredLine"], 11);
    EXPECT_EQ(json["body"]["variables"].at(0)["declaredSource"]["path"], "d:/abc/a.cpp");

    EXPECT_EQ(json["body"]["variables"].at(1)["type"], "void (*)()");
    EXPECT_EQ(json["body"]["variables"].at(1)["variablesReference"], 0);
    EXPECT_EQ(json["body"]["variables"].at(1)["value"], "0x0000000000401623");
    EXPECT_EQ(json["body"]["variables"].at(1)["name"], "functionPointer");
    EXPECT_EQ(json["body"]["variables"].at(1)["evaluateName"], "var1");
    EXPECT_EQ(json["body"]["variables"].at(1)["memoryReference"], "0x0000000000401623");
    EXPECT_EQ(json["body"]["variables"].at(1)["memoryAddress"], "0x0000000000401623");
}

TEST_F(ResponseManagerTest, variablesResponseTestError) {
    VariablesResponse variablesResponse;
    variablesResponse.command = REQ_RES_VARIABLES;
    variablesResponse.seq = 12;
    variablesResponse.type = ProtocolMessage::Type::RESPONSE;
    variablesResponse.variablesCount = 10;
    std::vector<Variable> vector;

    Variable variable1;
    variable1.name = "name";
    variable1.declaredLine.reset();
    variable1.isVarCreated.reset();
    vector.push_back(variable1);
    variablesResponse.variables = vector;
    variablesResponse.success = true;
    json_t json = ResponseManager::Instance().ToJson(variablesResponse).value();
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, setVariableResponseTest)
{
    SetVariableResponse setVariableResponse;
    setVariableResponse.command = REQ_RES_SET_VARIABLE;
    setVariableResponse.seq = 13;
    setVariableResponse.value = "100";
    setVariableResponse.success = true;

    json_t json = ResponseManager::Instance().ToJson(setVariableResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_SET_VARIABLE);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["value"], "100");
}

TEST_F(ResponseManagerTest, disassembleResponseTest)
{
    Source source;
    source.name = "source";
    source.presentationHint = PresentationHint::EMPHASIZE;
    std::vector<Checksum> checksums;
    Checksum checksum;
    checksum.checksum = "checksum";
    checksum.algorithm = ChecksumAlgorithmType::SHA256;
    checksums.push_back(std::move(checksum));
    source.checksums = std::move(checksums);

    DisassembledInstruction disassembledInstruction;
    disassembledInstruction.address = "address";
    disassembledInstruction.instructionBytes = "instructionBytes";
    disassembledInstruction.instruction = "instruction";
    disassembledInstruction.symbol = "symbol";
    disassembledInstruction.location = std::move(source);
    disassembledInstruction.line = 1;
    disassembledInstruction.column = 2;
    disassembledInstruction.endLine = 9;
    disassembledInstruction.endColumn = 10;

    DisassembleResponse disassembleResponse;
    disassembleResponse.command = REQ_RES_DISASSEMBLE;
    disassembleResponse.seq = 15;
    disassembleResponse.request_seq = 16;
    disassembleResponse.instructions.push_back(disassembledInstruction);
    disassembleResponse.success = true;

    json_t json = ResponseManager::Instance().ToJson(disassembleResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_DISASSEMBLE);
    EXPECT_EQ(json["type"], "response");
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["seq"], 15);
    EXPECT_EQ(json["request_seq"], 16);
    EXPECT_EQ(json["body"]["instructions"].at(0)["address"], "address");
    EXPECT_EQ(json["body"]["instructions"].at(0)["instructionBytes"], "instructionBytes");
    EXPECT_EQ(json["body"]["instructions"].at(0)["instruction"], "instruction");
    EXPECT_EQ(json["body"]["instructions"].at(0)["symbol"], "symbol");
    EXPECT_EQ(json["body"]["instructions"].at(0)["line"], 1);
    EXPECT_EQ(json["body"]["instructions"].at(0)["column"], 2);
    EXPECT_EQ(json["body"]["instructions"].at(0)["endLine"], 9);
    EXPECT_EQ(json["body"]["instructions"].at(0)["endColumn"], 10);
    EXPECT_EQ(json["body"]["instructions"].at(0)["location"]["name"], "source");
    EXPECT_EQ(json["body"]["instructions"].at(0)["location"]["checksums"].at(0)["checksum"], "checksum");
}

TEST_F(ResponseManagerTest, EvaluateResponseTest)
{
    EvaluateResponse response;
    response.command = REQ_RES_EVALUATE;
    response.seq = 10;
    response.request_seq = 9;
    response.success = true;
    json_t json = ResponseManager::Instance().ToJson(response).value();
    EXPECT_EQ(json["seq"], response.seq);

    VariablePresentationHint hint;
    hint.kind.reset();
    response.presentationHint = hint;
    json = ResponseManager::Instance().ToJson(response).value();
    EXPECT_EQ(json["seq"], response.seq);

    hint.kind = Kind::PROPERTY;
    hint.attributes = { Attributes::STATIC, Attributes::RAW_STRING };
    hint.visibility = Visibility::PUBLIC;
    response.presentationHint = hint;

    response.result = "true";
    response.resultType = "bool";
    response.variablesReference = 0;
    response.namedVariables = 0;
    response.memoryReference = "memoryReference";
    response.memoryAddress = "0x0b7e238c";

    json = ResponseManager::Instance().ToJson(response).value();
    EXPECT_EQ(json["command"], response.command);
    EXPECT_EQ(json["type"], ENUM_TO_STR(response.type).value());
    EXPECT_EQ(json["request_seq"], response.request_seq);
    EXPECT_EQ(json["success"], response.success);
    EXPECT_EQ(json["body"]["result"], response.result);
    EXPECT_EQ(json["body"]["type"], response.resultType);
    EXPECT_EQ(json["body"]["presentationHint"]["kind"],
            ENUM_TO_STR(response.presentationHint.value().kind.value()).value());
    EXPECT_EQ(json["body"]["presentationHint"]["attributes"][0],
              ENUM_TO_STR(response.presentationHint.value().attributes.value().at(0)).value());
    EXPECT_EQ(json["body"]["presentationHint"]["attributes"][1],
              ENUM_TO_STR(response.presentationHint.value().attributes.value().at(1)).value());
    EXPECT_EQ(json["body"]["presentationHint"]["visibility"],
              ENUM_TO_STR(response.presentationHint.value().visibility.value()).value());
    EXPECT_EQ(json["body"]["namedVariables"], response.namedVariables.value());
    EXPECT_EQ(json["body"]["memoryReference"], response.memoryReference.value());
    EXPECT_EQ(json["body"]["memoryAddress"], "0x0b7e238c");
}

TEST_F(ResponseManagerTest, DebugInConsoleResponseTest)
{
    DebugInConsoleResponse debugInConsoleResponse;
    debugInConsoleResponse.command = REQ_RES_DEBUG_IN_CONSOLE;
    debugInConsoleResponse.request_seq = 1;
    debugInConsoleResponse.seq = 2;
    debugInConsoleResponse.success = true;
    debugInConsoleResponse.output = "^done";

    json_t json = ResponseManager::Instance().ToJson(debugInConsoleResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_DEBUG_IN_CONSOLE);
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], "response");
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["output"], "^done");
}

TEST_F(ResponseManagerTest, runInTerminalResponseTest)
{
    json_t json;
    json["seq"] = 2;
    json["type"] = RESPONSE_NAME;
    json["request_seq"] = 1;
    json["success"] = true;
    json["command"] = REQ_RES_RUN_IN_TERMINAL;
    json["message"] = "message";
    json["body"]["processId"] = 10;
    json["body"]["shellProcessId"] = 11;

    std::unique_ptr<Dap::Protocol::Response> responsePtr = ResponseManager::Instance().ToResponse(json.dump());
    auto ptr = dynamic_cast<RunInTerminalResponse*>(responsePtr.get());
    EXPECT_EQ(ptr->seq, 2);
    EXPECT_EQ(ptr->type, STR_TO_ENUM<ProtocolMessage::Type>(RESPONSE_NAME));
    EXPECT_EQ(ptr->request_seq, 1);
    EXPECT_TRUE(ptr->success);
    EXPECT_EQ(ptr->command, REQ_RES_RUN_IN_TERMINAL);
    EXPECT_EQ(ptr->message, "message");
    EXPECT_EQ(ptr->processId, 10);
    EXPECT_EQ(ptr->shellProcessId, 11);
}

TEST_F(ResponseManagerTest, runInTerminalResponseTest_Error)
{
    json_t json;
    json["type"] = RESPONSE_NAME;
    json["request_seq"] = 1;
    json["success"] = true;
    json["command"] = REQ_RES_RUN_IN_TERMINAL;
    std::unique_ptr<Dap::Protocol::Response> responsePtr = ResponseManager::Instance().ToResponse(json.dump());
    EXPECT_EQ(responsePtr, nullptr);

    json["seq"] = 2;
    json["body"]["aaa"] = 10;
    std::unique_ptr<Dap::Protocol::Response> responseSecPtr = ResponseManager::Instance().ToResponse(json.dump());
    auto ptr = dynamic_cast<RunInTerminalResponse*>(responseSecPtr.get());
    EXPECT_EQ(ptr->seq, 2);
}

TEST_F(ResponseManagerTest, SourceResponseTest)
{
    SourceResponse sourceResponse;
    sourceResponse.command = REQ_RES_SOURCE;
    sourceResponse.request_seq = 1;
    sourceResponse.seq = 2;
    sourceResponse.success = true;
    sourceResponse.content = "content";
    sourceResponse.mimeType = "mimeType";

    json_t json = ResponseManager::Instance().ToJson(sourceResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_SOURCE);
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], "response");
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["content"], "content");
    EXPECT_EQ(json["body"]["mimeType"], "mimeType");
}

TEST_F(ResponseManagerTest, ReadMemoryResponseTest)
{
    ReadMemoryResponse readMemoryResponse;
    readMemoryResponse.command = REQ_RES_READ_MEMORY;
    readMemoryResponse.request_seq = 1;
    readMemoryResponse.seq = 2;
    readMemoryResponse.success = true;
    readMemoryResponse.address = "0x00000000004060a7";
    readMemoryResponse.unreadableBytes = 0;
    readMemoryResponse.data = "YWJjZA==";

    json_t json = Dap::Protocol::ToResponseJson(readMemoryResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_READ_MEMORY);
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], "response");
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["address"], "0x00000000004060a7");
    EXPECT_EQ(json["body"]["unreadableBytes"], 0);
    EXPECT_EQ(json["body"]["data"], "YWJjZA==");
}

TEST_F(ResponseManagerTest, ReadMemoryResponseTest_EX)
{
    ReadMemoryResponse readMemoryResponse;
    readMemoryResponse.command = REQ_RES_READ_MEMORY;
    readMemoryResponse.request_seq = 1;
    readMemoryResponse.seq = 2;
    readMemoryResponse.success = true;
    readMemoryResponse.address = "0x00000000004060a7";
    readMemoryResponse.unreadableBytes = 0;
    readMemoryResponse.data = "YWJjZA==";

    json_t json = ResponseManager::Instance().ToJson(readMemoryResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_READ_MEMORY);
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], "response");
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["address"], "0x00000000004060a7");
    EXPECT_EQ(json["body"]["unreadableBytes"], 0);
    EXPECT_EQ(json["body"]["data"], "YWJjZA==");
}

TEST_F(ResponseManagerTest, WriteMemoryResponseTest)
{
    WriteMemoryResponse writeMemoryResponse;
    writeMemoryResponse.command = REQ_RES_WRITE_MEMORY;
    writeMemoryResponse.request_seq = 1;
    writeMemoryResponse.seq = 2;
    writeMemoryResponse.success = true;
    writeMemoryResponse.offset = 12;
    writeMemoryResponse.bytesWritten = 144;

    json_t json = ResponseManager::Instance().ToJson(writeMemoryResponse).value();
    EXPECT_EQ(json["command"], REQ_RES_WRITE_MEMORY);
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], "response");
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["offset"], 12);
    EXPECT_EQ(json["body"]["bytesWritten"], 144);
}

TEST_F(ResponseManagerTest, SetInstructionBreakpointsResponseTest)
{
    SetInstructionBreakpointsResponse setInstructionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    Dap::Protocol::Breakpoint breakpoint;
    breakpoint.verified = true;
    breakpoint.instructionReference = "0x000000000040177e";
    breakpoint.id = 5;
    breakpoints.push_back(std::move(breakpoint));

    setInstructionBreakpointsResponse.command = REQ_RES_SET_INSTRUCTION_BREAKPOINTS;
    setInstructionBreakpointsResponse.seq = 1;
    setInstructionBreakpointsResponse.request_seq = 15;
    setInstructionBreakpointsResponse.breakpoints = breakpoints;
    setInstructionBreakpointsResponse.success = true;
    setInstructionBreakpointsResponse.type = ProtocolMessage::Type::RESPONSE;

    json_t json = Dap::Protocol::ToResponseJson(setInstructionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["command"], REQ_RES_SET_INSTRUCTION_BREAKPOINTS);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["instructionReference"], "0x000000000040177e");
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
}

TEST_F(ResponseManagerTest, SetInstructionBreakpointsResponseTest_EX)
{
    SetInstructionBreakpointsResponse setInstructionBreakpointsResponse;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    Dap::Protocol::Breakpoint breakpoint;
    breakpoint.verified = true;
    breakpoint.instructionReference = "0x000000000040177e";
    breakpoint.id = 5;
    breakpoints.push_back(std::move(breakpoint));

    setInstructionBreakpointsResponse.command = REQ_RES_SET_INSTRUCTION_BREAKPOINTS;
    setInstructionBreakpointsResponse.seq = 1;
    setInstructionBreakpointsResponse.request_seq = 15;
    setInstructionBreakpointsResponse.breakpoints = breakpoints;
    setInstructionBreakpointsResponse.success = true;
    setInstructionBreakpointsResponse.type = ProtocolMessage::Type::RESPONSE;

    json_t json = ResponseManager::Instance().ToJson(setInstructionBreakpointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["command"], REQ_RES_SET_INSTRUCTION_BREAKPOINTS);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["instructionReference"], "0x000000000040177e");
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
}

TEST_F(ResponseManagerTest, UndoResponseTest)
{
    UndoResponse undoResponse;
    undoResponse.command = REQ_RES_UNDO;
    undoResponse.type = ProtocolMessage::Type::RESPONSE;
    undoResponse.seq = 3;
    undoResponse.success = true;
    undoResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(undoResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_UNDO);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, UndoResponseTest_EX)
{
    UndoResponse undoResponse;
    undoResponse.command = REQ_RES_UNDO;
    undoResponse.type = ProtocolMessage::Type::RESPONSE;
    undoResponse.seq = 3;
    undoResponse.success = true;
    undoResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(undoResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_UNDO);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 3);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, ReverseStepResponseTest)
{
    ReverseStepResponse reverseStepResponse;
    reverseStepResponse.command = REQ_RES_REVERSE_STEP;
    reverseStepResponse.type = ProtocolMessage::Type::RESPONSE;
    reverseStepResponse.seq = 4;
    reverseStepResponse.success = false;
    reverseStepResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(reverseStepResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_REVERSE_STEP);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 4);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}


TEST_F(ResponseManagerTest, ReverseStepResponseTest_EX)
{
    ReverseStepResponse reverseStepResponse;
    reverseStepResponse.seq = 4;
    reverseStepResponse.success = false;
    reverseStepResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(reverseStepResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_REVERSE_STEP);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 4);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, ReverseContinueResponseTest)
{
    ReverseContinueResponse reverseContinuedResponse;
    reverseContinuedResponse.seq = 5;
    reverseContinuedResponse.success = true;
    reverseContinuedResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(reverseContinuedResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_REVERSE_CONTINUE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 5);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, ReverseContinueResponseTest_EX)
{
    ReverseContinueResponse reverseContinuedResponse;
    reverseContinuedResponse.seq = 5;
    reverseContinuedResponse.success = true;
    reverseContinuedResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(reverseContinuedResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_REVERSE_CONTINUE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 5);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, SetExecutionPointResponseTest)
{
    SetExecutionPointResponse setExecutionPointResponse;
    setExecutionPointResponse.message = "message";
    setExecutionPointResponse.success = true;
    setExecutionPointResponse.seq = 8;
    json_t json = ResponseManager::Instance().ToJson(setExecutionPointResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setExecutionPoint");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["message"], "message");
    EXPECT_EQ(json["seq"], 8);
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, GotoRecordPointResponseTest)
{
    GotoRecordPointResponse gotoRecordPointResp;
    gotoRecordPointResp.seq = 2;
    gotoRecordPointResp.success = true;
    gotoRecordPointResp.type = ProtocolMessage::Type::RESPONSE;
    gotoRecordPointResp.request_seq = 6;
    json_t json = ResponseManager::Instance().ToJson(gotoRecordPointResp).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"],"gotoRecordPoint");
    EXPECT_EQ(json["type"],ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"],2);
    EXPECT_EQ(json["success"],true);
    EXPECT_EQ(json["request_seq"],6);
}

TEST_F(ResponseManagerTest, RecordPointsResponseTest)
{
    RecordPointsResponse recordPointsResponse;
    recordPointsResponse.success = true;
    recordPointsResponse.type = ProtocolMessage::Type::RESPONSE;
    recordPointsResponse.request_seq = 1;
    recordPointsResponse.seq = 1;
    std::vector<RecordPoint> vecRecordPoints;
    RecordPoint recordPoint;
    recordPoint.id = 1;
    recordPoint.dateTimeStamp = "";
    recordPoint.timeStamp = 1681117933196;
    recordPoint.reason = "";
    recordPoint.threadId = 1;
    recordPoint.threadName = "main";
    recordPoint.coroutineId = 2;

    json_t json2 = ResponseManager::Instance().ToJson(recordPointsResponse).value();
    EXPECT_FALSE(json2.is_null());
    json_t json3 = json_t::array();
    EXPECT_EQ(json2["body"]["recordPoints"], json3);

    vecRecordPoints.push_back(recordPoint);
    recordPointsResponse.recordPoints = vecRecordPoints;
    json_t json = ResponseManager::Instance().ToJson(recordPointsResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "recordPoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 1);
    EXPECT_EQ(json["success"], true);
    EXPECT_EQ(json["request_seq"], 1);
    EXPECT_EQ(json["body"]["recordPoints"][0]["threadId"], 1);
    EXPECT_EQ(json["body"]["recordPoints"][0]["threadName"], "main");
    EXPECT_EQ(json["body"]["recordPoints"][0]["timeStamp"], 1681117933196);
    EXPECT_EQ(json["body"]["recordPoints"][0]["coroutineId"], 2);
}

TEST_F(ResponseManagerTest, ContinueInReverseResponseTest)
{
    ContinueInReverseResponse continueInReverseResponse;
    continueInReverseResponse.command = REQ_RES_CONTINUE_IN_REVERSE;
    continueInReverseResponse.type = ProtocolMessage::Type::RESPONSE;
    continueInReverseResponse.seq = 9;
    continueInReverseResponse.success = false;
    continueInReverseResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(continueInReverseResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_CONTINUE_IN_REVERSE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 9);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}


TEST_F(ResponseManagerTest, ContinueInReverseResponseTest_EX)
{
    ContinueInReverseResponse continueInReverseResponse;
    continueInReverseResponse.seq = 10;
    continueInReverseResponse.success = false;
    continueInReverseResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(continueInReverseResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_CONTINUE_IN_REVERSE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 10);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, StepInReverseResponseTest)
{
    StepInReverseResponse stepInReverseResponse;
    stepInReverseResponse.command = REQ_RES_STEP_IN_REVERSE;
    stepInReverseResponse.type = ProtocolMessage::Type::RESPONSE;
    stepInReverseResponse.seq = 10;
    stepInReverseResponse.success = false;
    stepInReverseResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(stepInReverseResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_STEP_IN_REVERSE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 10);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}


TEST_F(ResponseManagerTest, StepInReverseResponseTest_EX)
{
    StepInReverseResponse stepInReverseResponse;
    stepInReverseResponse.seq = 10;
    stepInReverseResponse.success = false;
    stepInReverseResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(stepInReverseResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_STEP_IN_REVERSE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 10);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, CacheDataConfigResponseTest)
{
    CacheDataConfigResponse cacheDataConfigResponse;
    cacheDataConfigResponse.command = REQ_RES_CACHE_DATA_CONFIG;
    cacheDataConfigResponse.type = ProtocolMessage::Type::RESPONSE;
    cacheDataConfigResponse.seq = 1;
    cacheDataConfigResponse.success = false;
    cacheDataConfigResponse.message = "message";
    json_t json = Dap::Protocol::ToResponseJson(cacheDataConfigResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_CACHE_DATA_CONFIG);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 1);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, CacheDataConfigResponseTest_EX)
{
    CacheDataConfigResponse cacheDataConfigResponse;
    cacheDataConfigResponse.seq = 2;
    cacheDataConfigResponse.success = false;
    cacheDataConfigResponse.message = "message";
    json_t json = ResponseManager::Instance().ToJson(cacheDataConfigResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_CACHE_DATA_CONFIG);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 2);
    EXPECT_FALSE(json["success"]);
    EXPECT_EQ(json["message"], "message");
}

TEST_F(ResponseManagerTest, RecordMessageResponseTest)
{
    RecordMessageResponse response;
    response.seq = 2;
    response.success = true;
    response.request_seq = 2;
    RecordMessage recordMessage;
    recordMessage.recordPointId = 3;
    recordMessage.thread = { 1, "thread-1", 0 };
    StackFrame stackFrame;
    stackFrame.id = 1002;
    stackFrame.name = "Function";
    stackFrame.line = 36;
    stackFrame.column = 0;
    Source source;
    source.path = "hello.cpp";
    stackFrame.source = source;
    recordMessage.stackFrame = stackFrame;
    Variable variable;
    variable.name = "a";
    variable.value = "12";
    recordMessage.localVariables = { variable };
    recordMessage.globalVariables = { variable };
    recordMessage.staticVariables = { variable };
    recordMessage.monitorVariables = {{ 1, "a", "locals", "int", "23" }};
    response.recordMessages.push_back(recordMessage);
    json_t json = ResponseManager::Instance().ToJson(response).value();
    EXPECT_EQ(json["command"], REQ_RES_RECORD_MESSAGE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["request_seq"], 2);
    EXPECT_TRUE(json["success"]);
    EXPECT_EQ(json["body"]["recordMessages"][0]["recordPointId"], 3);
    EXPECT_EQ(json["body"]["recordMessages"][0]["thread"]["id"], 1);
    EXPECT_EQ(json["body"]["recordMessages"][0]["thread"]["name"], "thread-1");
    EXPECT_EQ(json["body"]["recordMessages"][0]["stackFrame"]["id"], 1002);
    EXPECT_EQ(json["body"]["recordMessages"][0]["stackFrame"]["name"], "Function");
    EXPECT_EQ(json["body"]["recordMessages"][0]["stackFrame"]["line"], 36);
    EXPECT_EQ(json["body"]["recordMessages"][0]["stackFrame"]["column"], 0);
    EXPECT_EQ(json["body"]["recordMessages"][0]["stackFrame"]["source"]["path"], "hello.cpp");
    EXPECT_EQ(json["body"]["recordMessages"][0]["localVariables"][0]["name"], "a");
    EXPECT_EQ(json["body"]["recordMessages"][0]["localVariables"][0]["value"], "12");
    EXPECT_EQ(json["body"]["recordMessages"][0]["globalVariables"][0]["name"], "a");
    EXPECT_EQ(json["body"]["recordMessages"][0]["globalVariables"][0]["value"], "12");
    EXPECT_EQ(json["body"]["recordMessages"][0]["staticVariables"][0]["name"], "a");
    EXPECT_EQ(json["body"]["recordMessages"][0]["staticVariables"][0]["value"], "12");
    EXPECT_EQ(json["body"]["recordMessages"][0]["monitorVariables"][0]["id"], 1);
    EXPECT_EQ(json["body"]["recordMessages"][0]["monitorVariables"][0]["name"], "a");
    EXPECT_EQ(json["body"]["recordMessages"][0]["monitorVariables"][0]["scopeName"], "locals");
    EXPECT_EQ(json["body"]["recordMessages"][0]["monitorVariables"][0]["type"], "int");
    EXPECT_EQ(json["body"]["recordMessages"][0]["monitorVariables"][0]["value"], "23");
}

TEST_F(ResponseManagerTest, RecordMessageResponseParamTest)
{
    RecordMessageResponse response;
    response.seq = 2;
    response.success = true;
    response.request_seq = 2;
    RecordMessage recordMessage;
    recordMessage.recordPointId = 3;
    response.recordMessages.push_back(recordMessage);
    json_t json = ResponseManager::Instance().ToJson(response).value();
    EXPECT_EQ(json["command"], REQ_RES_RECORD_MESSAGE);
}

TEST_F(ResponseManagerTest, SetMemoryBreakpointsResponseTest)
{
    SetMemoryBreakpointsResponse response;
    std::vector<Dap::Protocol::Breakpoint> breakpoints;
    for (int i = 0; i < arraySize; i++) {
        Dap::Protocol::Breakpoint breakpoint;
        Source source;
        source.name = "source";
        source.presentationHint = PresentationHint::EMPHASIZE;
        std::vector<Checksum> checksums;
        Checksum checksum;
        checksum.checksum = "checksum";
        checksum.algorithm = ChecksumAlgorithmType::SHA256;
        checksums.push_back(std::move(checksum));
        source.checksums = std::move(checksums);
        breakpoint.verified = true;
        breakpoint.source = std::move(source);
        breakpoints.push_back(std::move(breakpoint));
    }

    response.breakpoints = std::move(breakpoints);
    json_t json = Dap::Protocol::ToResponseJson(response).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "setMemoryBreakpoints");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoints"].at(0)["verified"], true);
}

TEST_F(ResponseManagerTest, GetBitmapResponseTest)
{
    GetBitmapResponse response;
    response.seq = 2;
    response.success = true;
    response.request_seq = 1;
    response.bitmap.width = 4;
    response.bitmap.height = 3;
    response.bitmap.format.colorFormat = -1;
    response.bitmap.format.alphaFormat = -1;
    std::string data = "/wD6//8A+v//APr//wD/APr//wD6//8A+v//AP8A+v//APr//wD6//8A/wD6//8A+v//APr//wA=";
    response.bitmap.data = data;
    response.bitmap.size = 48;
    json_t json = Dap::Protocol::ToResponseJson(response).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "getBitmap");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["width"], 4);
    EXPECT_EQ(json["body"]["height"], 3);
    EXPECT_EQ(json["body"]["format"]["colorFormat"], -1);
    EXPECT_EQ(json["body"]["format"]["alphaFormat"], -1);
    EXPECT_EQ(json["body"]["data"], data);
    EXPECT_EQ(json["body"]["size"], 48);
}

TEST_F(ResponseManagerTest, addSourceBreakpointResponseTest)
{
    AddSourceBreakpointResponse addSourceBreakpointResponse;
    Dap::Protocol::Breakpoint breakpoint;
    Source source;
    source.name = "source";
    source.presentationHint = PresentationHint::EMPHASIZE;
    std::vector<Checksum> checksums;
    Checksum checksum;
    checksum.checksum = "checksum";
    checksum.algorithm = ChecksumAlgorithmType::SHA256;
    checksums.push_back(std::move(checksum));
    source.checksums = std::move(checksums);
    breakpoint.verified = true;
    breakpoint.source = std::move(source);

    addSourceBreakpointResponse.breakpoint = std::move(breakpoint);
    addSourceBreakpointResponse.success = true;
    json_t json = Dap::Protocol::ToResponseJson(addSourceBreakpointResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "addSourceBreakpoint");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["body"]["breakpoint"]["verified"], true);
    EXPECT_EQ(json["success"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, deleteBreakpointResponseTest)
{
    DeleteBreakpointResponse deleteBreakpointResponse;
    deleteBreakpointResponse.success = true;
    json_t json = Dap::Protocol::ToResponseJson(deleteBreakpointResponse).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], "deleteBreakpoint");
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_EQ(json["success"], true);
    EXPECT_EQ(json["message"].is_null(), true);
}

TEST_F(ResponseManagerTest, VariableHexViewResponseTest)
{
    VariableHexViewResponse response;
    response.seq = 2;
    response.success = true;
    response.request_seq = 1;
    response.message = "";
    json_t json = Dap::Protocol::ToResponseJson(response).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_VARIABLE_HEX_VIEW);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_TRUE(json["success"]);
}

TEST_F(ResponseManagerTest, CommandCompletionResponseTest)
{
    CommandCompletionResponse response;
    response.seq = 2;
    response.success = true;
    response.request_seq = 1;
    response.message = "";
    CommandCompletionObject obj1 = CommandCompletionObject();
    response.isPatch = false;
    obj1.command = "add";
    obj1.alias = "a";
    obj1.spec = "add 123";
    response.objects.push_back(obj1);
    json_t json = Dap::Protocol::ToResponseJson(response).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["command"], REQ_RES_COMMAND_COMPLETION);
    EXPECT_EQ(json["type"], ENUM_TO_STR(ProtocolMessage::Type::RESPONSE));
    EXPECT_TRUE(json["success"]);
    EXPECT_TRUE(json["body"]["objects"][0u]["command"] == "add");
    EXPECT_TRUE(json["body"]["objects"][0u]["alias"] == "a");
    EXPECT_TRUE(json["body"]["objects"][0u]["spec"] == "add 123");
    EXPECT_FALSE(json["body"]["isPatch"]);
    EXPECT_TRUE(json["body"]["objects"].size() == 1);
}