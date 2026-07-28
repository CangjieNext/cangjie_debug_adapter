// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "Protocol.h"
#include "EventManager.h"
#include "EnumUtils.h"
#include "ProtocolSuite.h"

using namespace Dap::Protocol;

class EventManagerTest : public ProtocolSuite {
    void SetUp() override
    {
        EventManager::Instance().Register();
        EventManager::Instance().Register();
        EXPECT_TRUE(EventManager::Instance().IsRegister());
    }
    void TearDown() override
    {
        EventManager::Instance().UnRegister();
        EventManager::Instance().UnRegister();
    }
};

TEST_F(EventManagerTest, initializedEventTest)
{
    InitializedEvent initializedEvent;
    initializedEvent.seq = 2;
    json_t json = ToEventJson(initializedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "initialized");
}

TEST_F(EventManagerTest, initializedEventTest_EX)
{
    InitializedEvent initializedEvent;
    initializedEvent.seq = 2;
    json_t json = EventManager::Instance().ToJson(initializedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "initialized");
}

TEST_F(EventManagerTest, stoppedEventTest)
{
    StoppedEvent stoppedEvent;
    stoppedEvent.seq = 2;
    stoppedEvent.body.reason = StoppedReason::BREAKPOINT;
    stoppedEvent.body.allThreadsStopped = true;
    stoppedEvent.body.threadId = 3002;
    stoppedEvent.body.description = "description";
    stoppedEvent.body.text = "text";
    stoppedEvent.body.hitBreakpointIds = {1, 2, 3};
    stoppedEvent.body.recordPointId = 6;
    stoppedEvent.body.isReverseMode = true;

    json_t json = ToEventJson(stoppedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "stopped");
    EXPECT_EQ(json["body"]["reason"], ENUM_TO_STR(StoppedReason::BREAKPOINT));
    EXPECT_EQ(json["body"]["allThreadsStopped"], true);
    EXPECT_EQ(json["body"]["threadId"], 3002);
    EXPECT_EQ(json["body"]["description"], "description");
    EXPECT_EQ(json["body"]["text"], "text");
    EXPECT_EQ(json["body"]["hitBreakpointIds"].size(), 3);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(0), 1);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(1), 2);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(2), 3);
    EXPECT_EQ(json["body"]["recordPointId"], 6);
    EXPECT_EQ(json["body"]["isReverseMode"], true);
}

TEST_F(EventManagerTest, stoppedEventTest_EX)
{
    StoppedEvent stoppedEvent;
    stoppedEvent.seq = 2;
    stoppedEvent.body.reason = StoppedReason::BREAKPOINT;
    stoppedEvent.body.allThreadsStopped = true;
    stoppedEvent.body.threadId = 3002;
    stoppedEvent.body.description = "description";
    stoppedEvent.body.text = "text";

    json_t json = EventManager::Instance().ToJson(stoppedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "stopped");
    EXPECT_EQ(json["body"]["reason"], ENUM_TO_STR(StoppedReason::BREAKPOINT));
    EXPECT_EQ(json["body"]["allThreadsStopped"], true);
    EXPECT_EQ(json["body"]["threadId"], 3002);
    EXPECT_EQ(json["body"]["description"], "description");
    EXPECT_EQ(json["body"]["text"], "text");
}

TEST_F(EventManagerTest, continuedEventTest)
{
    ContinuedEvent continuedEvent;
    continuedEvent.seq = 5;
    continuedEvent.body.threadId = 3002;
    continuedEvent.body.allThreadsContinued = true;
    json_t json = ToEventJson(continuedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "continued");
    EXPECT_EQ(json["body"]["threadId"], 3002);
    EXPECT_EQ(json["body"]["allThreadsContinued"], true);
}

TEST_F(EventManagerTest, continuedEventTest_EX)
{
    ContinuedEvent continuedEvent;
    continuedEvent.seq = 5;
    continuedEvent.body.threadId = 3002;
    continuedEvent.body.allThreadsContinued = true;
    json_t json = EventManager::Instance().ToJson(continuedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "continued");
    EXPECT_EQ(json["body"]["threadId"], 3002);
    EXPECT_EQ(json["body"]["allThreadsContinued"], true);
}

TEST_F(EventManagerTest, exitedEventTest)
{
    ExitedEvent exitedEvent;
    exitedEvent.seq = 2;
    exitedEvent.body.exitCode = 0;
    json_t json = ToEventJson(exitedEvent).value();
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "exited");
    EXPECT_EQ(json["body"]["exitCode"], 0);
}

TEST_F(EventManagerTest, exitedEventTest_EX)
{
    ExitedEvent exitedEvent;
    exitedEvent.seq = 2;
    exitedEvent.body.exitCode = 0;
    json_t json = EventManager::Instance().ToJson(exitedEvent).value();
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "exited");
    EXPECT_EQ(json["body"]["exitCode"], 0);
}

TEST_F(EventManagerTest, terminatedEventTest)
{
    TerminatedEvent terminatedEvent;
    terminatedEvent.seq = 2;
    json_t json = ToEventJson(terminatedEvent).value();
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "terminated");
}

TEST_F(EventManagerTest, terminatedEventTest_EX)
{
    TerminatedEvent terminatedEvent;
    terminatedEvent.seq = 2;
    json_t json = EventManager::Instance().ToJson(terminatedEvent).value();
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "terminated");
}

TEST_F(EventManagerTest, breakpointEventTest)
{
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = 2;
    breakpointEvent.body.reason = Reason::CHANGED;
    Breakpoint breakpoint;
    breakpoint.verified = false;
    breakpointEvent.body.breakpoint = breakpoint;
    json_t json = ToEventJson(breakpointEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "breakpoint");
    EXPECT_EQ(json["body"]["breakpoint"]["verified"], false);
    EXPECT_EQ(json["body"]["reason"], "changed");
}

TEST_F(EventManagerTest, breakpointEventTest_EX)
{
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = 2;
    breakpointEvent.body.reason = Reason::CHANGED;
    Breakpoint breakpoint;
    breakpoint.verified = false;
    breakpointEvent.body.breakpoint = breakpoint;
    json_t json = EventManager::Instance().ToJson(breakpointEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "breakpoint");
    EXPECT_EQ(json["body"]["breakpoint"]["verified"], false);
    EXPECT_EQ(json["body"]["reason"], "changed");
}

TEST_F(EventManagerTest, capabilitiesEventTest)
{
    CapabilitiesEvent capabilitiesEvent;
    capabilitiesEvent.seq = 2;

    std::vector<ExceptionBreakpointsFilter> exceptionBreakpointFilters;
    for (int i = 0; i < 10; i++) {
        ExceptionBreakpointsFilter exceptionBreakpointsFilter;
        exceptionBreakpointsFilter.filter = "filter";
        exceptionBreakpointsFilter.label = "label";
        exceptionBreakpointFilters.push_back(std::move(exceptionBreakpointsFilter));
    }
    std::vector<std::string> completionTriggerCharacters;
    for (int i = 0; i < 10; i++) {
        completionTriggerCharacters.push_back("completionTriggerCharacter" + std::to_string(i));
    }
    std::vector<ColumnDescriptor> additionalModuleColumns;
    for (int i = 0; i < 10; i++) {
        ColumnDescriptor columnDescriptor;
        columnDescriptor.attributeName = "attributeName";
        columnDescriptor.label = "label";
        additionalModuleColumns.push_back(std::move(columnDescriptor));
    }
    std::vector<ChecksumAlgorithmType> supportedChecksumAlgorithms;
    supportedChecksumAlgorithms.push_back(ChecksumAlgorithmType::MD5);
    supportedChecksumAlgorithms.push_back(ChecksumAlgorithmType::SHA256);

    capabilitiesEvent.body.capabilities.exceptionBreakpointFilters = std::move(exceptionBreakpointFilters);
    capabilitiesEvent.body.capabilities.completionTriggerCharacters = std::move(completionTriggerCharacters);
    capabilitiesEvent.body.capabilities.additionalModuleColumns = std::move(additionalModuleColumns);
    capabilitiesEvent.body.capabilities.supportedChecksumAlgorithms = std::move(supportedChecksumAlgorithms);
    capabilitiesEvent.body.capabilities.supportsConfigurationDoneRequest = true;
    capabilitiesEvent.body.capabilities.supportsCancelRequest = true;

    json_t json = ToEventJson(capabilitiesEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "capabilities");
    EXPECT_EQ(json["body"]["capabilities"]["exceptionBreakpointFilters"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["completionTriggerCharacters"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["additionalModuleColumns"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["supportedChecksumAlgorithms"].size(), 2);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsConfigurationDoneRequest"]);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsCancelRequest"]);
}

TEST_F(EventManagerTest, capabilitiesEventTest_EX)
{
    CapabilitiesEvent capabilitiesEvent;
    capabilitiesEvent.seq = 2;

    std::vector<ExceptionBreakpointsFilter> exceptionBreakpointFilters;
    for (int i = 0; i < 10; i++) {
        ExceptionBreakpointsFilter exceptionBreakpointsFilter;
        exceptionBreakpointsFilter.filter = "filter";
        exceptionBreakpointsFilter.label = "label";
        exceptionBreakpointFilters.push_back(std::move(exceptionBreakpointsFilter));
    }
    std::vector<std::string> completionTriggerCharacters;
    for (int i = 0; i < 10; i++) {
        completionTriggerCharacters.push_back("completionTriggerCharacter" + std::to_string(i));
    }
    std::vector<ColumnDescriptor> additionalModuleColumns;
    for (int i = 0; i < 10; i++) {
        ColumnDescriptor columnDescriptor;
        columnDescriptor.attributeName = "attributeName";
        columnDescriptor.label = "label";
        additionalModuleColumns.push_back(std::move(columnDescriptor));
    }
    std::vector<ChecksumAlgorithmType> supportedChecksumAlgorithms;
    supportedChecksumAlgorithms.push_back(ChecksumAlgorithmType::MD5);
    supportedChecksumAlgorithms.push_back(ChecksumAlgorithmType::SHA256);

    capabilitiesEvent.body.capabilities.exceptionBreakpointFilters = std::move(exceptionBreakpointFilters);
    capabilitiesEvent.body.capabilities.completionTriggerCharacters = std::move(completionTriggerCharacters);
    capabilitiesEvent.body.capabilities.additionalModuleColumns = std::move(additionalModuleColumns);
    capabilitiesEvent.body.capabilities.supportedChecksumAlgorithms = std::move(supportedChecksumAlgorithms);
    capabilitiesEvent.body.capabilities.supportsConfigurationDoneRequest = true;
    capabilitiesEvent.body.capabilities.supportsCancelRequest = true;

    json_t json = EventManager::Instance().ToJson(capabilitiesEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "capabilities");
    EXPECT_EQ(json["body"]["capabilities"]["exceptionBreakpointFilters"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["completionTriggerCharacters"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["additionalModuleColumns"].size(), 10);
    EXPECT_EQ(json["body"]["capabilities"]["supportedChecksumAlgorithms"].size(), 2);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsConfigurationDoneRequest"]);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsCancelRequest"]);
}

// OutputEvent
void SetOutputEvent(OutputEvent& event)
{
    event.seq = 1;
    event.body.category = OutputEventBodyCategory::STDOUT;
    event.body.output = "output";
    event.body.group = OutputEventBodyGroup::START;
    event.body.variablesReference = 1;
    // source
    Source source;
    source.name = "source";
    source.path = "C:\\main.cpp";
    source.sourceReference = 2;
    source.presentationHint = PresentationHint::NORMAL;
    source.origin = "./module.cpp";
    json_t json_adapter = "adapterData";
    source.adapterData = json_adapter;
    // checksums
    Checksum checksum;
    checksum.algorithm = ChecksumAlgorithmType::MD5;
    checksum.checksum = "1234567890123456";
    std::vector<Checksum> checksums = { checksum };
    source.checksums = checksums;
    // sources
    std::vector<Source> sources { source };
    source.sources = sources;
    event.body.source = source;
    event.body.line = 2;
    event.body.column = 0;
    // data
    json_t json_data = "data";
    event.body.data = json_data;
}

void ExpectOutputEvent(const json_t &json)
{
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 1);
    EXPECT_EQ(json["type"],  ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT).value());
    EXPECT_EQ(json["event"], EVENT_OUTPUT);
    EXPECT_EQ(json["body"]["category"], ENUM_TO_STR(OutputEventBodyCategory::STDOUT).value());
    EXPECT_EQ(json["body"]["group"], ENUM_TO_STR(OutputEventBodyGroup::START).value());
    EXPECT_EQ(json["body"]["variablesReference"], 1);

    EXPECT_EQ(json["body"]["source"]["name"], "source");
    EXPECT_EQ(json["body"]["source"]["path"], "C:\\main.cpp");
    EXPECT_EQ(json["body"]["source"]["sourceReference"], 2);
    EXPECT_EQ(json["body"]["source"]["presentationHint"], ENUM_TO_STR(PresentationHint::NORMAL).value());
    EXPECT_EQ(json["body"]["source"]["origin"], "./module.cpp");
    EXPECT_EQ(json["body"]["source"]["adapterData"], "adapterData");
    EXPECT_EQ(json["body"]["source"]["checksums"].size(), 1);
    EXPECT_EQ(json["body"]["source"]["checksums"][0]["algorithm"], ENUM_TO_STR(ChecksumAlgorithmType::MD5).value());
    EXPECT_EQ(json["body"]["source"]["checksums"][0]["checksum"], "1234567890123456");

    EXPECT_EQ(json["body"]["source"]["sources"].size(), 1);
    EXPECT_EQ(json["body"]["source"]["sources"][0]["name"], "source");
    EXPECT_EQ(json["body"]["source"]["sources"][0]["path"], "C:\\main.cpp");
    EXPECT_EQ(json["body"]["source"]["sources"][0]["sourceReference"], 2);
    EXPECT_EQ(json["body"]["source"]["sources"][0]["presentationHint"], ENUM_TO_STR(PresentationHint::NORMAL).value());
    EXPECT_EQ(json["body"]["source"]["sources"][0]["origin"], "./module.cpp");
    EXPECT_EQ(json["body"]["source"]["sources"][0]["adapterData"], "adapterData");
    EXPECT_EQ(json["body"]["source"]["sources"][0]["checksums"].size(), 1);
    EXPECT_EQ(json["body"]["source"]["sources"][0]["checksums"][0]["algorithm"],
              ENUM_TO_STR(ChecksumAlgorithmType::MD5).value());
    EXPECT_EQ(json["body"]["source"]["sources"][0]["checksums"][0]["checksum"], "1234567890123456");

    EXPECT_EQ(json["body"]["line"], 2);
    EXPECT_EQ(json["body"]["column"], 0);
    EXPECT_EQ(json["body"]["data"], "data");
}

TEST_F(EventManagerTest, outputEventTest)
{
    OutputEvent event;
    SetOutputEvent(event);
    json_t json = EventManager::Instance().ToJson(event).value();
    ExpectOutputEvent(json);
}

TEST_F(EventManagerTest, moduleEventTest)
{
    ModuleEvent event;
    event.seq = 1;
    event.body.reason = Reason::CHANGED;
    event.body.module.id = 2;
    event.body.module.uuid = "E15C96F9-9D08-1750-0ACD-88F55A998777";
    event.body.module.name = "name";
    event.body.module.path = "path";
    event.body.module.isOptimized = true;
    event.body.module.isUserCode = false;
    event.body.module.version = "1.0";
    event.body.module.symbolStatus = "symbolStatus";
    event.body.module.symbolFilePath = "symbolFilePath";
    event.body.module.dateTimeStamp = "dateTimeStamp";
    event.body.module.addressRange = "addressRange";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["reason"], "changed");
    EXPECT_EQ(json["body"]["module"]["id"], 2);
    EXPECT_EQ(json["body"]["module"]["uuid"], "E15C96F9-9D08-1750-0ACD-88F55A998777");
    EXPECT_EQ(json["body"]["module"]["name"], "name");
    EXPECT_EQ(json["body"]["module"]["path"], "path");
    EXPECT_TRUE(json["body"]["module"]["isOptimized"]);
    EXPECT_FALSE(json["body"]["module"]["isUserCode"]);
    EXPECT_EQ(json["body"]["module"]["version"], "1.0");
    EXPECT_EQ(json["body"]["module"]["symbolStatus"], "symbolStatus");
    EXPECT_EQ(json["body"]["module"]["symbolFilePath"], "symbolFilePath");
    EXPECT_EQ(json["body"]["module"]["dateTimeStamp"], "dateTimeStamp");
    EXPECT_EQ(json["body"]["module"]["addressRange"], "addressRange");
}

TEST_F(EventManagerTest, loadedSourceEventTest)
{
    LoadedSourceEvent event;
    event.seq = 1;
    event.body.reason = Reason::CHANGED;
    event.body.source.name = "name";
    event.body.source.path = "path";
    event.body.source.sourceReference = 0;
    event.body.source.presentationHint = PresentationHint::DEEMPHASIZE;
    event.body.source.origin = "origin";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["reason"], "changed");
    EXPECT_EQ(json["body"]["source"]["name"], "name");
    EXPECT_EQ(json["body"]["source"]["path"], "path");
    EXPECT_EQ(json["body"]["source"]["sourceReference"], 0);
    EXPECT_EQ(json["body"]["source"]["presentationHint"], "deemphasize");
    EXPECT_EQ(json["body"]["source"]["origin"], "origin");
}

TEST_F(EventManagerTest, processEventTest)
{
    ProcessEvent event;
    event.seq = 1;
    event.body.name = "name";
    event.body.systemProcessId = 1002;
    event.body.isLocalProcess = true;
    event.body.startMethod = StartMethod::ATTACH_FOR_SUSPENDED_LAUNCH;
    event.body.pointerSize = 10;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["name"], "name");
    EXPECT_EQ(json["body"]["systemProcessId"], 1002);
    EXPECT_TRUE(json["body"]["isLocalProcess"]);
    EXPECT_EQ(json["body"]["startMethod"], "attachForSuspendedLaunch");
    EXPECT_EQ(json["body"]["pointerSize"], 10);
}

TEST_F(EventManagerTest, progressStartEventTest)
{
    ProgressStartEvent event;
    event.seq = 1;
    event.body.progressId = "progressId";
    event.body.title = "title";
    event.body.requestId = 10;
    event.body.cancellable = true;
    event.body.message = "message";
    event.body.percentage = 25;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["progressId"], "progressId");
    EXPECT_EQ(json["body"]["title"], "title");
    EXPECT_EQ(json["body"]["requestId"], 10);
    EXPECT_TRUE(json["body"]["cancellable"]);
    EXPECT_EQ(json["body"]["message"], "message");
    EXPECT_EQ(json["body"]["percentage"], 25);
}

TEST_F(EventManagerTest, progressUpdateEventTest)
{
    ProgressUpdateEvent event;
    event.seq = 1;
    event.body.progressId = "progressId";
    event.body.message = "message";
    event.body.percentage = 20;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["progressId"], "progressId");
    EXPECT_EQ(json["body"]["message"], "message");
    EXPECT_EQ(json["body"]["percentage"], 20);
}

TEST_F(EventManagerTest, progressEndEventTest)
{
    ProgressEndEvent event;
    event.seq = 1;
    event.body.progressId = "progressId";
    event.body.message = "message";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["progressId"], "progressId");
    EXPECT_EQ(json["body"]["message"], "message");
}

TEST_F(EventManagerTest, invalidatedEventTest)
{
    std::vector<InvalidatedAreas> vector;
    vector.push_back(InvalidatedAreas::STACKS);
    vector.push_back(InvalidatedAreas::THREADS);
    InvalidatedEvent event;
    event.seq = 1;
    event.body.threadId = 1;
    event.body.stackFrameId = 2;
    event.body.areas = vector;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["threadId"], 1);
    EXPECT_EQ(json["body"]["stackFrameId"], 2);
    EXPECT_EQ(json["body"]["areas"].at(0), "stacks");
    EXPECT_EQ(json["body"]["areas"].at(1), "threads");
}

TEST_F(EventManagerTest, recordPointsUpdateEventTest)
{
    RecordPointsUpdateEvent recordPointsEvent;
    recordPointsEvent.seq = 2;
    recordPointsEvent.body.removedRecordPointIds = {1, 2, 3};

    std::vector<RecordPoint> vecRecordPoints;
    RecordPoint recordPoint;
    recordPoint.id = 1;
    recordPoint.dateTimeStamp = "2023-4-24 15:51";
    recordPoint.timeStamp = 1681117933196;
    recordPoint.reason = "";
    recordPoint.threadId = 1;
    recordPoint.threadName = "main";
    recordPoint.coroutineId = 2;
    vecRecordPoints.push_back(recordPoint);
    recordPointsEvent.body.newRecordPoints = vecRecordPoints;

    json_t json = ToEventJson(recordPointsEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "recordPointsUpdate");
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(0), 1);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(1), 2);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(2), 3);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["id"], 1);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["dateTimeStamp"], "2023-4-24 15:51");
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["timeStamp"], 1681117933196);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["threadName"], "main");
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["coroutineId"], 2);

    json = EventManager::Instance().ToJson(recordPointsEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], 2);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "recordPointsUpdate");
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(0), 1);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(1), 2);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(2), 3);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["id"], 1);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["dateTimeStamp"], "2023-4-24 15:51");
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["timeStamp"], 1681117933196);
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["threadName"], "main");
    EXPECT_EQ(json["body"]["newRecordPoints"][0]["coroutineId"], 2);
}

TEST_F(EventManagerTest, threadEventTest)
{
    ThreadEvent event;
    event.seq = 1;
    event.body.threadId = 3;
    event.body.reason = "exited";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["threadId"], 3);
    EXPECT_EQ(json["body"]["reason"], "exited");
}