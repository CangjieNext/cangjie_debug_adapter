// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "ProtocolSuite.h"
#include "EnumUtils.h"

using namespace Dap::Protocol;

class EnumUtilsTest : public ProtocolSuite {
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EnumUtilsTest, NotFoundEnumTest)
{
    ASSERT_TRUE(ENUM_TO_STR("STOP") == std::nullopt);
}

TEST_F(EnumUtilsTest, ProtocolMessageTypeTest)
{
    ASSERT_EQ(std::nullopt, STR_TO_ENUM<ProtocolMessage::Type>("NONE"));
    ASSERT_EQ(ProtocolMessage::Type::REQUEST, STR_TO_ENUM<ProtocolMessage::Type>(
        ENUM_TO_STR(ProtocolMessage::Type::REQUEST).value()));
    ASSERT_EQ(ProtocolMessage::Type::RESPONSE, STR_TO_ENUM<ProtocolMessage::Type>(
        ENUM_TO_STR(ProtocolMessage::Type::RESPONSE).value()));
    ASSERT_EQ(ProtocolMessage::Type::EVENT, STR_TO_ENUM<ProtocolMessage::Type>(
        ENUM_TO_STR(ProtocolMessage::Type::EVENT).value()));
    ASSERT_EQ(ProtocolMessage::Type::NONE, STR_TO_ENUM<ProtocolMessage::Type>(
            ENUM_TO_STR(ProtocolMessage::Type::NONE).value()));
}

TEST_F(EnumUtilsTest, StoppedReasonTest)
{
    ASSERT_EQ(StoppedReason::STEP, STR_TO_ENUM<StoppedReason>(ENUM_TO_STR(StoppedReason::STEP).value()));
    ASSERT_EQ(StoppedReason::BREAKPOINT, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::BREAKPOINT).value()));
    ASSERT_EQ(StoppedReason::EXCEPTION, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::EXCEPTION).value()));
    ASSERT_EQ(StoppedReason::EXCEPTION, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::EXCEPTION).value()));
    ASSERT_EQ(StoppedReason::PAUSE, STR_TO_ENUM<StoppedReason>(ENUM_TO_STR(StoppedReason::PAUSE).value()));
    ASSERT_EQ(StoppedReason::ENTRY, STR_TO_ENUM<StoppedReason>(ENUM_TO_STR(StoppedReason::ENTRY).value()));
    ASSERT_EQ(StoppedReason::GOTO, STR_TO_ENUM<StoppedReason>(ENUM_TO_STR(StoppedReason::GOTO).value()));
    ASSERT_EQ(StoppedReason::FUNCTION_BREAKPOINT, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::FUNCTION_BREAKPOINT).value()));
    ASSERT_EQ(StoppedReason::DATA_BREAKPOINT, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::DATA_BREAKPOINT).value()));
    ASSERT_EQ(StoppedReason::INSTRUCTION_BREAKPOINT, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::INSTRUCTION_BREAKPOINT).value()));
    ASSERT_EQ(StoppedReason::SET_EXECUTIONPOINT, STR_TO_ENUM<StoppedReason>(
        ENUM_TO_STR(StoppedReason::SET_EXECUTIONPOINT).value()));
    ASSERT_EQ(StoppedReason::UNKNOWN, STR_TO_ENUM<StoppedReason>(
            ENUM_TO_STR(StoppedReason::UNKNOWN).value()));
}

TEST_F(EnumUtilsTest, ColumnDescriptorTypeTest)
{
    ASSERT_EQ(ColumnDescriptorType::STRING, STR_TO_ENUM<ColumnDescriptorType>(
        ENUM_TO_STR(ColumnDescriptorType::STRING).value()));
    ASSERT_EQ(ColumnDescriptorType::NUMBER, STR_TO_ENUM<ColumnDescriptorType>(
        ENUM_TO_STR(ColumnDescriptorType::NUMBER).value()));
    ASSERT_EQ(ColumnDescriptorType::BOOLEAN, STR_TO_ENUM<ColumnDescriptorType>(
        ENUM_TO_STR(ColumnDescriptorType::BOOLEAN).value()));
    ASSERT_EQ(ColumnDescriptorType::UNIX_TIMESTAMP_UTC, STR_TO_ENUM<ColumnDescriptorType>(
        ENUM_TO_STR(ColumnDescriptorType::UNIX_TIMESTAMP_UTC).value()));
}

TEST_F(EnumUtilsTest, ChecksumAlgorithmTypeTest)
{
    ASSERT_EQ(ChecksumAlgorithmType::MD5, STR_TO_ENUM<ChecksumAlgorithmType>(
        ENUM_TO_STR(ChecksumAlgorithmType::MD5).value()));
    ASSERT_EQ(ChecksumAlgorithmType::SHA1, STR_TO_ENUM<ChecksumAlgorithmType>(
        ENUM_TO_STR(ChecksumAlgorithmType::SHA1).value()));
    ASSERT_EQ(ChecksumAlgorithmType::SHA256, STR_TO_ENUM<ChecksumAlgorithmType>(
        ENUM_TO_STR(ChecksumAlgorithmType::SHA256).value()));
    ASSERT_EQ(ChecksumAlgorithmType::TIMESTAMP, STR_TO_ENUM<ChecksumAlgorithmType>(
        ENUM_TO_STR(ChecksumAlgorithmType::TIMESTAMP).value()));
}

TEST_F(EnumUtilsTest, PathFormatTest)
{
    ASSERT_EQ(PathFormat::PATH, STR_TO_ENUM<PathFormat>(ENUM_TO_STR(PathFormat::PATH).value()));
    ASSERT_EQ(PathFormat::URL, STR_TO_ENUM<PathFormat>(ENUM_TO_STR(PathFormat::URL).value()));
}

TEST_F(EnumUtilsTest, PresentationHintTest)
{
    ASSERT_EQ(PresentationHint::NORMAL, STR_TO_ENUM<PresentationHint>(
        ENUM_TO_STR(PresentationHint::NORMAL).value()));
    ASSERT_EQ(PresentationHint::EMPHASIZE, STR_TO_ENUM<PresentationHint>(
        ENUM_TO_STR(PresentationHint::EMPHASIZE).value()));
    ASSERT_EQ(PresentationHint::DEEMPHASIZE, STR_TO_ENUM<PresentationHint>(
        ENUM_TO_STR(PresentationHint::DEEMPHASIZE).value()));
}

TEST_F(EnumUtilsTest, ReasonTest)
{
    ASSERT_EQ(Reason::CHANGED, STR_TO_ENUM<Reason>(ENUM_TO_STR(Reason::CHANGED).value()));
    ASSERT_EQ(Reason::NEW, STR_TO_ENUM<Reason>(ENUM_TO_STR(Reason::NEW).value()));
    ASSERT_EQ(Reason::REMOVED, STR_TO_ENUM<Reason>(ENUM_TO_STR(Reason::REMOVED).value()));
}

TEST_F(EnumUtilsTest, SteppingGranularityTest)
{
    ASSERT_EQ(SteppingGranularity::STATEMENT, STR_TO_ENUM<SteppingGranularity>(
        ENUM_TO_STR(SteppingGranularity::STATEMENT).value()));
    ASSERT_EQ(SteppingGranularity::LINE, STR_TO_ENUM<SteppingGranularity>(
        ENUM_TO_STR(SteppingGranularity::LINE).value()));
    ASSERT_EQ(SteppingGranularity::INSTRUCTION, STR_TO_ENUM<SteppingGranularity>(
        ENUM_TO_STR(SteppingGranularity::INSTRUCTION).value()));
}

TEST_F(EnumUtilsTest, FilterTest)
{
    ASSERT_EQ(Filter::INDEXED, STR_TO_ENUM<Filter>(ENUM_TO_STR(Filter::INDEXED).value()));
    ASSERT_EQ(Filter::NAMED, STR_TO_ENUM<Filter>(ENUM_TO_STR(Filter::NAMED).value()));
}

TEST_F(EnumUtilsTest, KindTest)
{
    ASSERT_EQ(Kind::PROPERTY, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::PROPERTY).value()));
    ASSERT_EQ(Kind::METHOD, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::METHOD).value()));
    ASSERT_EQ(Kind::CLASS, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::CLASS).value()));
    ASSERT_EQ(Kind::DATA, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::DATA).value()));
    ASSERT_EQ(Kind::EVENT, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::EVENT).value()));
    ASSERT_EQ(Kind::BASE_CLASS, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::BASE_CLASS).value()));
    ASSERT_EQ(Kind::INNER_CLASS, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::INNER_CLASS).value()));
    ASSERT_EQ(Kind::INTERFACE, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::INTERFACE).value()));
    ASSERT_EQ(Kind::MOST_DERIVED_CLASS, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::MOST_DERIVED_CLASS).value()));
    ASSERT_EQ(Kind::VIRTUAL, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::VIRTUAL).value()));
    ASSERT_EQ(Kind::DATA_BREAKPOINT, STR_TO_ENUM<Kind>(ENUM_TO_STR(Kind::DATA_BREAKPOINT).value()));
}

TEST_F(EnumUtilsTest, VisibilityTest)
{
    ASSERT_EQ(Visibility::PUBLIC, STR_TO_ENUM<Visibility>(ENUM_TO_STR(Visibility::PUBLIC).value()));
    ASSERT_EQ(Visibility::PRIVATE, STR_TO_ENUM<Visibility>(ENUM_TO_STR(Visibility::PRIVATE).value()));
    ASSERT_EQ(Visibility::PROTECTED, STR_TO_ENUM<Visibility>(ENUM_TO_STR(Visibility::PROTECTED).value()));
    ASSERT_EQ(Visibility::INTERNAL, STR_TO_ENUM<Visibility>(ENUM_TO_STR(Visibility::INTERNAL).value()));
    ASSERT_EQ(Visibility::FINAL, STR_TO_ENUM<Visibility>(ENUM_TO_STR(Visibility::FINAL).value()));
}

TEST_F(EnumUtilsTest, AttributesTest)
{
    ASSERT_EQ(Attributes::STATIC, STR_TO_ENUM<Attributes>(ENUM_TO_STR(Attributes::STATIC).value()));
    ASSERT_EQ(Attributes::CONSTANT, STR_TO_ENUM<Attributes>(ENUM_TO_STR(Attributes::CONSTANT).value()));
    ASSERT_EQ(Attributes::READ_ONLY, STR_TO_ENUM<Attributes>(ENUM_TO_STR(Attributes::READ_ONLY).value()));
    ASSERT_EQ(Attributes::RAW_STRING, STR_TO_ENUM<Attributes>(ENUM_TO_STR(Attributes::RAW_STRING).value()));
    ASSERT_EQ(Attributes::HAS_OBJECT_ID, STR_TO_ENUM<Attributes>(ENUM_TO_STR(Attributes::HAS_OBJECT_ID).value()));
    ASSERT_EQ(Attributes::CAN_HAVE_OBJECT_ID, STR_TO_ENUM<Attributes>(
        ENUM_TO_STR(Attributes::CAN_HAVE_OBJECT_ID).value()));
    ASSERT_EQ(Attributes::HAS_SIDE_EFFECTS, STR_TO_ENUM<Attributes>(
        ENUM_TO_STR(Attributes::HAS_SIDE_EFFECTS).value()));
}

TEST_F(EnumUtilsTest, OutputEventBodyCategoryTest)
{
    ASSERT_EQ(OutputEventBodyCategory::CONSOLE, STR_TO_ENUM<OutputEventBodyCategory>(
        ENUM_TO_STR(OutputEventBodyCategory::CONSOLE).value()));
    ASSERT_EQ(OutputEventBodyCategory::STDOUT, STR_TO_ENUM<OutputEventBodyCategory>(
        ENUM_TO_STR(OutputEventBodyCategory::STDOUT).value()));
    ASSERT_EQ(OutputEventBodyCategory::STDERR, STR_TO_ENUM<OutputEventBodyCategory>(
        ENUM_TO_STR(OutputEventBodyCategory::STDERR).value()));
    ASSERT_EQ(OutputEventBodyCategory::TELEMETRY, STR_TO_ENUM<OutputEventBodyCategory>(
        ENUM_TO_STR(OutputEventBodyCategory::TELEMETRY).value()));
}

TEST_F(EnumUtilsTest, OutputEventBodyGroupTest)
{
    ASSERT_EQ(OutputEventBodyGroup::START, STR_TO_ENUM<OutputEventBodyGroup>(
        ENUM_TO_STR(OutputEventBodyGroup::START).value()));
    ASSERT_EQ(OutputEventBodyGroup::START_COLLAPSED, STR_TO_ENUM<OutputEventBodyGroup>(
        ENUM_TO_STR(OutputEventBodyGroup::START_COLLAPSED).value()));
    ASSERT_EQ(OutputEventBodyGroup::END, STR_TO_ENUM<OutputEventBodyGroup>(
        ENUM_TO_STR(OutputEventBodyGroup::END).value()));
}

TEST_F(EnumUtilsTest, StartMethodTest)
{
    ASSERT_EQ(StartMethod::LAUNCH, STR_TO_ENUM<StartMethod>(ENUM_TO_STR(StartMethod::LAUNCH).value()));
    ASSERT_EQ(StartMethod::ATTACH, STR_TO_ENUM<StartMethod>(ENUM_TO_STR(StartMethod::ATTACH).value()));
    ASSERT_EQ(StartMethod::ATTACH_FOR_SUSPENDED_LAUNCH, STR_TO_ENUM<StartMethod>(
        ENUM_TO_STR(StartMethod::ATTACH_FOR_SUSPENDED_LAUNCH).value()));
}

TEST_F(EnumUtilsTest, InvalidatedAreasTest)
{
    ASSERT_EQ(InvalidatedAreas::ALL, STR_TO_ENUM<InvalidatedAreas>(
        ENUM_TO_STR(InvalidatedAreas::ALL).value()));
    ASSERT_EQ(InvalidatedAreas::STACKS, STR_TO_ENUM<InvalidatedAreas>(
        ENUM_TO_STR(InvalidatedAreas::STACKS).value()));
    ASSERT_EQ(InvalidatedAreas::THREADS, STR_TO_ENUM<InvalidatedAreas>(
        ENUM_TO_STR(InvalidatedAreas::THREADS).value()));
    ASSERT_EQ(InvalidatedAreas::VARIABLES, STR_TO_ENUM<InvalidatedAreas>(
        ENUM_TO_STR(InvalidatedAreas::VARIABLES).value()));
}

TEST_F(EnumUtilsTest, ContextTest)
{
    ASSERT_EQ(Context::WATCH, STR_TO_ENUM<Context>(ENUM_TO_STR(Context::WATCH).value()));
    ASSERT_EQ(Context::REPL, STR_TO_ENUM<Context>(ENUM_TO_STR(Context::REPL).value()));
    ASSERT_EQ(Context::HOVER, STR_TO_ENUM<Context>(ENUM_TO_STR(Context::HOVER).value()));
    ASSERT_EQ(Context::CLIPBOARD, STR_TO_ENUM<Context>(ENUM_TO_STR(Context::CLIPBOARD).value()));
}

TEST_F(EnumUtilsTest, StackFramePresentationHintTest)
{
    ASSERT_EQ(StackFramePresentationHint::NORMAL, STR_TO_ENUM<StackFramePresentationHint>(
        ENUM_TO_STR(StackFramePresentationHint::NORMAL).value()));
    ASSERT_EQ(StackFramePresentationHint::LABEL, STR_TO_ENUM<StackFramePresentationHint>(
        ENUM_TO_STR(StackFramePresentationHint::LABEL).value()));
    ASSERT_EQ(StackFramePresentationHint::SUBTLE, STR_TO_ENUM<StackFramePresentationHint>(
        ENUM_TO_STR(StackFramePresentationHint::SUBTLE).value()));
}