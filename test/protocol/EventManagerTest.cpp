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

namespace {
constexpr int TEST_SEQUENCE = 2;
constexpr int TEST_PRIMARY_SEQUENCE = 1;
constexpr int TEST_CONTINUED_SEQUENCE = 5;
constexpr int TEST_THREAD_ID = 3002;
constexpr int TEST_COLLECTION_SIZE = 10;
constexpr int TEST_HIT_BREAKPOINT_COUNT = 3;
constexpr int TEST_FIRST_INDEX = 0;
constexpr int TEST_SECOND_INDEX = 1;
constexpr int TEST_THIRD_INDEX = 2;
constexpr int TEST_DEFAULT_VALUE = 0;
constexpr int TEST_VARIABLE_REFERENCE = 1;
constexpr int TEST_SOURCE_REFERENCE = 2;
constexpr int TEST_THIRD_ID = 3;
constexpr int TEST_RECORD_POINT_ID = 6;
constexpr int TEST_SYSTEM_PROCESS_ID = 1002;
constexpr int TEST_REQUEST_ID = 10;
constexpr int TEST_START_PERCENTAGE = 25;
constexpr int TEST_UPDATE_PERCENTAGE = 20;
constexpr long long TEST_TIMESTAMP = 1681117933196;
}

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
    initializedEvent.seq = TEST_SEQUENCE;
    json_t json = ToEventJson(initializedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "initialized");
}

TEST_F(EventManagerTest, initializedEventTest_EX)
{
    InitializedEvent initializedEvent;
    initializedEvent.seq = TEST_SEQUENCE;
    json_t json = EventManager::Instance().ToJson(initializedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "initialized");
}

TEST_F(EventManagerTest, stoppedEventTest)
{
    StoppedEvent stoppedEvent;
    stoppedEvent.seq = TEST_SEQUENCE;
    stoppedEvent.body.reason = StoppedReason::BREAKPOINT;
    stoppedEvent.body.allThreadsStopped = true;
    stoppedEvent.body.threadId = TEST_THREAD_ID;
    stoppedEvent.body.description = "description";
    stoppedEvent.body.text = "text";
    stoppedEvent.body.hitBreakpointIds = {TEST_VARIABLE_REFERENCE, TEST_SOURCE_REFERENCE, TEST_THIRD_ID};
    stoppedEvent.body.recordPointId = TEST_RECORD_POINT_ID;
    stoppedEvent.body.isReverseMode = true;

    json_t json = ToEventJson(stoppedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "stopped");
    EXPECT_EQ(json["body"]["reason"], ENUM_TO_STR(StoppedReason::BREAKPOINT));
    EXPECT_EQ(json["body"]["allThreadsStopped"], true);
    EXPECT_EQ(json["body"]["threadId"], TEST_THREAD_ID);
    EXPECT_EQ(json["body"]["description"], "description");
    EXPECT_EQ(json["body"]["text"], "text");
    EXPECT_EQ(json["body"]["hitBreakpointIds"].size(), TEST_HIT_BREAKPOINT_COUNT);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(TEST_FIRST_INDEX), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(TEST_SECOND_INDEX), TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["hitBreakpointIds"].at(TEST_THIRD_INDEX), TEST_THIRD_ID);
    EXPECT_EQ(json["body"]["recordPointId"], TEST_RECORD_POINT_ID);
    EXPECT_EQ(json["body"]["isReverseMode"], true);
}

TEST_F(EventManagerTest, stoppedEventTest_EX)
{
    StoppedEvent stoppedEvent;
    stoppedEvent.seq = TEST_SEQUENCE;
    stoppedEvent.body.reason = StoppedReason::BREAKPOINT;
    stoppedEvent.body.allThreadsStopped = true;
    stoppedEvent.body.threadId = TEST_THREAD_ID;
    stoppedEvent.body.description = "description";
    stoppedEvent.body.text = "text";

    json_t json = EventManager::Instance().ToJson(stoppedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "stopped");
    EXPECT_EQ(json["body"]["reason"], ENUM_TO_STR(StoppedReason::BREAKPOINT));
    EXPECT_EQ(json["body"]["allThreadsStopped"], true);
    EXPECT_EQ(json["body"]["threadId"], TEST_THREAD_ID);
    EXPECT_EQ(json["body"]["description"], "description");
    EXPECT_EQ(json["body"]["text"], "text");
}

TEST_F(EventManagerTest, continuedEventTest)
{
    ContinuedEvent continuedEvent;
    continuedEvent.seq = TEST_CONTINUED_SEQUENCE;
    continuedEvent.body.threadId = TEST_THREAD_ID;
    continuedEvent.body.allThreadsContinued = true;
    json_t json = ToEventJson(continuedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "continued");
    EXPECT_EQ(json["body"]["threadId"], TEST_THREAD_ID);
    EXPECT_EQ(json["body"]["allThreadsContinued"], true);
}

TEST_F(EventManagerTest, continuedEventTest_EX)
{
    ContinuedEvent continuedEvent;
    continuedEvent.seq = TEST_CONTINUED_SEQUENCE;
    continuedEvent.body.threadId = TEST_THREAD_ID;
    continuedEvent.body.allThreadsContinued = true;
    json_t json = EventManager::Instance().ToJson(continuedEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "continued");
    EXPECT_EQ(json["body"]["threadId"], TEST_THREAD_ID);
    EXPECT_EQ(json["body"]["allThreadsContinued"], true);
}

TEST_F(EventManagerTest, exitedEventTest)
{
    ExitedEvent exitedEvent;
    exitedEvent.seq = TEST_SEQUENCE;
    exitedEvent.body.exitCode = TEST_DEFAULT_VALUE;
    json_t json = ToEventJson(exitedEvent).value();
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "exited");
    EXPECT_EQ(json["body"]["exitCode"], TEST_DEFAULT_VALUE);
}

TEST_F(EventManagerTest, exitedEventTest_EX)
{
    ExitedEvent exitedEvent;
    exitedEvent.seq = TEST_SEQUENCE;
    exitedEvent.body.exitCode = TEST_DEFAULT_VALUE;
    json_t json = EventManager::Instance().ToJson(exitedEvent).value();
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "exited");
    EXPECT_EQ(json["body"]["exitCode"], TEST_DEFAULT_VALUE);
}

TEST_F(EventManagerTest, terminatedEventTest)
{
    TerminatedEvent terminatedEvent;
    terminatedEvent.seq = TEST_SEQUENCE;
    json_t json = ToEventJson(terminatedEvent).value();
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "terminated");
}

TEST_F(EventManagerTest, terminatedEventTest_EX)
{
    TerminatedEvent terminatedEvent;
    terminatedEvent.seq = TEST_SEQUENCE;
    json_t json = EventManager::Instance().ToJson(terminatedEvent).value();
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "terminated");
}

TEST_F(EventManagerTest, breakpointEventTest)
{
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = TEST_SEQUENCE;
    breakpointEvent.body.reason = Reason::CHANGED;
    Breakpoint breakpoint;
    breakpoint.verified = false;
    breakpointEvent.body.breakpoint = breakpoint;
    json_t json = ToEventJson(breakpointEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "breakpoint");
    EXPECT_EQ(json["body"]["breakpoint"]["verified"], false);
    EXPECT_EQ(json["body"]["reason"], "changed");
}

TEST_F(EventManagerTest, breakpointEventTest_EX)
{
    BreakpointEvent breakpointEvent;
    breakpointEvent.seq = TEST_SEQUENCE;
    breakpointEvent.body.reason = Reason::CHANGED;
    Breakpoint breakpoint;
    breakpoint.verified = false;
    breakpointEvent.body.breakpoint = breakpoint;
    json_t json = EventManager::Instance().ToJson(breakpointEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "breakpoint");
    EXPECT_EQ(json["body"]["breakpoint"]["verified"], false);
    EXPECT_EQ(json["body"]["reason"], "changed");
}

TEST_F(EventManagerTest, capabilitiesEventTest)
{
    CapabilitiesEvent capabilitiesEvent;
    capabilitiesEvent.seq = TEST_SEQUENCE;

    std::vector<ExceptionBreakpointsFilter> exceptionBreakpointFilters;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
        ExceptionBreakpointsFilter exceptionBreakpointsFilter;
        exceptionBreakpointsFilter.filter = "filter";
        exceptionBreakpointsFilter.label = "label";
        exceptionBreakpointFilters.push_back(std::move(exceptionBreakpointsFilter));
    }
    std::vector<std::string> completionTriggerCharacters;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
        completionTriggerCharacters.push_back("completionTriggerCharacter" + std::to_string(i));
    }
    std::vector<ColumnDescriptor> additionalModuleColumns;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
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
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "capabilities");
    EXPECT_EQ(json["body"]["capabilities"]["exceptionBreakpointFilters"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["completionTriggerCharacters"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["additionalModuleColumns"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["supportedChecksumAlgorithms"].size(), TEST_SOURCE_REFERENCE);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsConfigurationDoneRequest"]);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsCancelRequest"]);
}

TEST_F(EventManagerTest, capabilitiesEventTest_EX)
{
    CapabilitiesEvent capabilitiesEvent;
    capabilitiesEvent.seq = TEST_SEQUENCE;

    std::vector<ExceptionBreakpointsFilter> exceptionBreakpointFilters;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
        ExceptionBreakpointsFilter exceptionBreakpointsFilter;
        exceptionBreakpointsFilter.filter = "filter";
        exceptionBreakpointsFilter.label = "label";
        exceptionBreakpointFilters.push_back(std::move(exceptionBreakpointsFilter));
    }
    std::vector<std::string> completionTriggerCharacters;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
        completionTriggerCharacters.push_back("completionTriggerCharacter" + std::to_string(i));
    }
    std::vector<ColumnDescriptor> additionalModuleColumns;
    for (int i = 0; i < TEST_COLLECTION_SIZE; ++i) {
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
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "capabilities");
    EXPECT_EQ(json["body"]["capabilities"]["exceptionBreakpointFilters"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["completionTriggerCharacters"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["additionalModuleColumns"].size(), TEST_COLLECTION_SIZE);
    EXPECT_EQ(json["body"]["capabilities"]["supportedChecksumAlgorithms"].size(), TEST_SOURCE_REFERENCE);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsConfigurationDoneRequest"]);
    EXPECT_TRUE(json["body"]["capabilities"]["supportsCancelRequest"]);
}

// OutputEvent
void SetOutputEvent(OutputEvent& event)
{
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.category = OutputEventBodyCategory::STDOUT;
    event.body.output = "output";
    event.body.group = OutputEventBodyGroup::START;
    event.body.variablesReference = TEST_VARIABLE_REFERENCE;
    // source
    Source source;
    source.name = "source";
    source.path = "C:\\main.cpp";
    source.sourceReference = TEST_SOURCE_REFERENCE;
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
    event.body.line = TEST_SOURCE_REFERENCE;
    event.body.column = TEST_DEFAULT_VALUE;
    // data
    json_t json_data = "data";
    event.body.data = json_data;
}

void ExpectOutputEvent(const json_t &json)
{
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_PRIMARY_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT).value());
    EXPECT_EQ(json["event"], EVENT_OUTPUT);
    EXPECT_EQ(json["body"]["category"], ENUM_TO_STR(OutputEventBodyCategory::STDOUT).value());
    EXPECT_EQ(json["body"]["group"], ENUM_TO_STR(OutputEventBodyGroup::START).value());
    EXPECT_EQ(json["body"]["variablesReference"], TEST_VARIABLE_REFERENCE);

    EXPECT_EQ(json["body"]["source"]["name"], "source");
    EXPECT_EQ(json["body"]["source"]["path"], "C:\\main.cpp");
    EXPECT_EQ(json["body"]["source"]["sourceReference"], TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["source"]["presentationHint"], ENUM_TO_STR(PresentationHint::NORMAL).value());
    EXPECT_EQ(json["body"]["source"]["origin"], "./module.cpp");
    EXPECT_EQ(json["body"]["source"]["adapterData"], "adapterData");
    EXPECT_EQ(json["body"]["source"]["checksums"].size(), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["source"]["checksums"][TEST_FIRST_INDEX]["algorithm"],
              ENUM_TO_STR(ChecksumAlgorithmType::MD5).value());
    EXPECT_EQ(json["body"]["source"]["checksums"][TEST_FIRST_INDEX]["checksum"], "1234567890123456");

    EXPECT_EQ(json["body"]["source"]["sources"].size(), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["name"], "source");
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["path"], "C:\\main.cpp");
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["sourceReference"], TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["presentationHint"],
              ENUM_TO_STR(PresentationHint::NORMAL).value());
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["origin"], "./module.cpp");
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["adapterData"], "adapterData");
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["checksums"].size(), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["checksums"][TEST_FIRST_INDEX]["algorithm"],
              ENUM_TO_STR(ChecksumAlgorithmType::MD5).value());
    EXPECT_EQ(json["body"]["source"]["sources"][TEST_FIRST_INDEX]["checksums"][TEST_FIRST_INDEX]["checksum"],
              "1234567890123456");

    EXPECT_EQ(json["body"]["line"], TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["column"], TEST_DEFAULT_VALUE);
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
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.reason = Reason::CHANGED;
    event.body.module.id = TEST_SOURCE_REFERENCE;
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
    EXPECT_EQ(json["body"]["module"]["id"], TEST_SOURCE_REFERENCE);
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
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.reason = Reason::CHANGED;
    event.body.source.name = "name";
    event.body.source.path = "path";
    event.body.source.sourceReference = TEST_DEFAULT_VALUE;
    event.body.source.presentationHint = PresentationHint::DEEMPHASIZE;
    event.body.source.origin = "origin";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["reason"], "changed");
    EXPECT_EQ(json["body"]["source"]["name"], "name");
    EXPECT_EQ(json["body"]["source"]["path"], "path");
    EXPECT_EQ(json["body"]["source"]["sourceReference"], TEST_DEFAULT_VALUE);
    EXPECT_EQ(json["body"]["source"]["presentationHint"], "deemphasize");
    EXPECT_EQ(json["body"]["source"]["origin"], "origin");
}

TEST_F(EventManagerTest, processEventTest)
{
    ProcessEvent event;
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.name = "name";
    event.body.systemProcessId = TEST_SYSTEM_PROCESS_ID;
    event.body.isLocalProcess = true;
    event.body.startMethod = StartMethod::ATTACH_FOR_SUSPENDED_LAUNCH;
    event.body.pointerSize = TEST_COLLECTION_SIZE;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["name"], "name");
    EXPECT_EQ(json["body"]["systemProcessId"], TEST_SYSTEM_PROCESS_ID);
    EXPECT_TRUE(json["body"]["isLocalProcess"]);
    EXPECT_EQ(json["body"]["startMethod"], "attachForSuspendedLaunch");
    EXPECT_EQ(json["body"]["pointerSize"], TEST_COLLECTION_SIZE);
}

TEST_F(EventManagerTest, progressStartEventTest)
{
    ProgressStartEvent event;
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.progressId = "progressId";
    event.body.title = "title";
    event.body.requestId = TEST_REQUEST_ID;
    event.body.cancellable = true;
    event.body.message = "message";
    event.body.percentage = TEST_START_PERCENTAGE;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["progressId"], "progressId");
    EXPECT_EQ(json["body"]["title"], "title");
    EXPECT_EQ(json["body"]["requestId"], TEST_REQUEST_ID);
    EXPECT_TRUE(json["body"]["cancellable"]);
    EXPECT_EQ(json["body"]["message"], "message");
    EXPECT_EQ(json["body"]["percentage"], TEST_START_PERCENTAGE);
}

TEST_F(EventManagerTest, progressUpdateEventTest)
{
    ProgressUpdateEvent event;
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.progressId = "progressId";
    event.body.message = "message";
    event.body.percentage = TEST_UPDATE_PERCENTAGE;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["progressId"], "progressId");
    EXPECT_EQ(json["body"]["message"], "message");
    EXPECT_EQ(json["body"]["percentage"], TEST_UPDATE_PERCENTAGE);
}

TEST_F(EventManagerTest, progressEndEventTest)
{
    ProgressEndEvent event;
    event.seq = TEST_PRIMARY_SEQUENCE;
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
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.threadId = TEST_VARIABLE_REFERENCE;
    event.body.stackFrameId = TEST_SOURCE_REFERENCE;
    event.body.areas = vector;
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["threadId"], TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["stackFrameId"], TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["areas"].at(TEST_FIRST_INDEX), "stacks");
    EXPECT_EQ(json["body"]["areas"].at(TEST_SECOND_INDEX), "threads");
}

TEST_F(EventManagerTest, recordPointsUpdateEventTest)
{
    RecordPointsUpdateEvent recordPointsEvent;
    recordPointsEvent.seq = TEST_SEQUENCE;
    recordPointsEvent.body.removedRecordPointIds = {
        TEST_VARIABLE_REFERENCE, TEST_SOURCE_REFERENCE, TEST_THIRD_ID};

    std::vector<RecordPoint> vecRecordPoints;
    RecordPoint recordPoint;
    recordPoint.id = TEST_VARIABLE_REFERENCE;
    recordPoint.dateTimeStamp = "2023-4-24 15:51";
    recordPoint.timeStamp = TEST_TIMESTAMP;
    recordPoint.reason = "";
    recordPoint.threadId = TEST_VARIABLE_REFERENCE;
    recordPoint.threadName = "main";
    recordPoint.coroutineId = TEST_SOURCE_REFERENCE;
    vecRecordPoints.push_back(recordPoint);
    recordPointsEvent.body.newRecordPoints = vecRecordPoints;

    json_t json = ToEventJson(recordPointsEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "recordPointsUpdate");
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_FIRST_INDEX), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_SECOND_INDEX), TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_THIRD_INDEX), TEST_THIRD_ID);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["id"], TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["dateTimeStamp"], "2023-4-24 15:51");
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["timeStamp"], TEST_TIMESTAMP);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["threadName"], "main");
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["coroutineId"], TEST_SOURCE_REFERENCE);

    json = EventManager::Instance().ToJson(recordPointsEvent).value();
    EXPECT_FALSE(json.is_null());
    EXPECT_EQ(json["seq"], TEST_SEQUENCE);
    EXPECT_EQ(json["type"], ENUM_TO_STR(Dap::Protocol::ProtocolMessage::Type::EVENT));
    EXPECT_EQ(json["event"], "recordPointsUpdate");
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_FIRST_INDEX), TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_SECOND_INDEX), TEST_SOURCE_REFERENCE);
    EXPECT_EQ(json["body"]["removedRecordPointIds"].at(TEST_THIRD_INDEX), TEST_THIRD_ID);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["id"], TEST_VARIABLE_REFERENCE);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["dateTimeStamp"], "2023-4-24 15:51");
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["timeStamp"], TEST_TIMESTAMP);
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["threadName"], "main");
    EXPECT_EQ(json["body"]["newRecordPoints"][TEST_FIRST_INDEX]["coroutineId"], TEST_SOURCE_REFERENCE);
}

TEST_F(EventManagerTest, threadEventTest)
{
    ThreadEvent event;
    event.seq = TEST_PRIMARY_SEQUENCE;
    event.body.threadId = TEST_THIRD_ID;
    event.body.reason = "exited";
    json_t json = EventManager::Instance().ToJson(event).value();
    EXPECT_EQ(json["body"]["threadId"], TEST_THIRD_ID);
    EXPECT_EQ(json["body"]["reason"], "exited");
}
