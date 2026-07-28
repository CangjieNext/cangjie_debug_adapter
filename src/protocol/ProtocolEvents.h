// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_PROTOCOLEVENTS_H
#define DAP_PROTOCOLEVENTS_H

#include "defs.h"
#include "ProtocolBase.h"
#include "ProtocolDefs.h"
#include "ProtocolTypes.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

// Initialized
struct InitializedEvent : public Event {
    InitializedEvent() : Event(EVENT_INITIALIZED) {}
};

// Stopped
struct StoppedEventBody {
    StoppedReason reason = StoppedReason::UNKNOWN;
    std::optional<std::string> description;
    std::optional<int64_t> threadId;
    std::optional<bool> preserveFocusHint;
    std::optional<std::string> text;
    std::optional<bool> allThreadsStopped;
    std::optional<std::vector<int64_t>> hitBreakpointIds;
    // for reverse debug
    std::optional<int64_t> recordPointId;
    std::optional<bool> isReverseMode;
};

struct StoppedEvent : public Event {
    StoppedEvent() : Event(EVENT_STOPPED) {}

    StoppedEventBody body;
};

// Continued
struct ContinuedEventBody {
    int64_t threadId = -1;
    std::optional<bool> allThreadsContinued;
};

struct ContinuedEvent : public Event {
    ContinuedEvent() : Event(EVENT_CONTINUED) {}

    ContinuedEventBody body;
};

// Exited
struct ExitedEventBody {
    int64_t exitCode = -1;
};

struct ExitedEvent : public Event {
    ExitedEvent() : Event(EVENT_EXITED) {}

    ExitedEventBody body;
};

// Terminated
struct TerminatedEventBody {
    std::optional<any> restart;
};

struct TerminatedEvent : public Event {
    TerminatedEvent() : Event(EVENT_TERMINATED) {}

    std::optional<TerminatedEventBody> body;
};

// Capabilities
struct CapabilitiesEventBody {
    Capabilities capabilities;
};

struct CapabilitiesEvent : public Event {
    CapabilitiesEvent() : Event(EVENT_CAPABILITIES) {}

    CapabilitiesEventBody body;
};

// Output
struct OutputEventBody {
    std::string output;
    std::optional<OutputEventBodyCategory> category;
    std::optional<OutputEventBodyGroup> group;
    std::optional<int64_t> variablesReference;
    std::optional<Source> source;
    std::optional<int64_t> line;
    std::optional<int64_t> column;
    std::optional<any> data;
};

struct OutputEvent : public Event {
    OutputEvent() : Event(EVENT_OUTPUT) {}

    OutputEventBody body;
};

// Breakpoint
struct BreakpointEventBody {
    Reason reason = Reason::NEW;
    Breakpoint breakpoint;
    // huawei
    std::optional<DataBreakpoint> dataBreakpoint;
    std::optional<uint32_t> hitCount;
    std::optional<uint32_t> ignoreCount;
    std::optional<std::string> description;
};

struct BreakpointEvent : public Event {
    BreakpointEvent() : Event(EVENT_BREAKPOINT) {}

    BreakpointEventBody body;
};

// Thread
struct ThreadEventBody {
    std::string reason;
    int64_t threadId = -1;
};

struct ThreadEvent : public Event {
    ThreadEvent() : Event(EVENT_THREAD) {}

    ThreadEventBody body;
};

// Module
struct ModuleEventBody {
    Reason reason = Reason::NEW;
    Module module;
};

struct ModuleEvent : public Event {
    ModuleEvent() : Event(EVENT_MODULE) {}

    ModuleEventBody body;
};

struct ModulesEventBody {
    Reason reason = Reason::NEW;
    std::vector<Module> moduleList;
};

struct ModulesEvent : public Event {
    ModulesEvent() : Event(EVENT_MODULES) {}

    ModulesEventBody body;
};

// LoadedSource
struct LoadedSourceBody {
    Reason reason = Reason::NEW;
    Source source;
};

struct LoadedSourceEvent : public Event {
    LoadedSourceEvent() : Event(EVENT_LOADED_SOURCE) {}

    LoadedSourceBody body;
};

// Process
struct ProcessEventBody {
    std::string name;
    std::optional<int64_t> systemProcessId;
    std::optional<bool> isLocalProcess;
    std::optional<StartMethod> startMethod;
    std::optional<int64_t> pointerSize;
};

struct ProcessEvent : public Event {
    ProcessEvent() : Event(EVENT_PROCESS) {}

    ProcessEventBody body;
};

// ProgressStart
struct ProgressStartEventBody {
    std::string progressId;
    std::string title;
    std::optional<int64_t> requestId;
    std::optional<bool> cancellable;
    std::optional<std::string> message;
    std::optional<int64_t> percentage;
};

struct ProgressStartEvent : public Event {
    ProgressStartEvent() : Event(EVENT_PROGRESS_START) {}

    ProgressStartEventBody body;
};

// ProgressUpdate
struct ProgressUpdateEventBody {
    std::string progressId;
    std::optional<std::string> message;
    std::optional<int64_t> percentage;
};

struct ProgressUpdateEvent : public Event {
    ProgressUpdateEvent() : Event(EVENT_PROGRESS_UPDATE) {}

    ProgressUpdateEventBody body;
};

// ProgressEnd
struct ProgressEndEventBody {
    std::string progressId;
    std::optional<std::string> message;
};

struct ProgressEndEvent : public Event {
    ProgressEndEvent() : Event(EVENT_PROGRESS_END) {}

    ProgressEndEventBody body;
};

// Invalidated
struct InvalidatedEventBody {
    std::optional<std::vector<InvalidatedAreas>> areas;
    std::optional<int64_t> threadId;
    std::optional<int64_t> stackFrameId;
};

struct InvalidatedEvent : public Event {
    InvalidatedEvent() : Event(EVENT_INVALIDATED) {}

    InvalidatedEventBody body;
};

// RecordPointEvent
// When the reverse debugging function is enabled and the forward debugging is performed, each time the stop event is
// reported, the record point increase or decrease information is reported to the client through this event.
// new indicates the record point added in this process, and re indicates the ID of the record point deleted because
// the cache reaches the maximum value.
struct RecordPointsUpdateEventBody {
    std::optional<std::vector<int64_t>> removedRecordPointIds;
    std::optional<std::vector<RecordPoint>> newRecordPoints;
};

struct RecordPointsUpdateEvent : public Event {
    RecordPointsUpdateEvent() : Event(EVENT_RECORD_UPDATE_POINTS) {}
    RecordPointsUpdateEventBody body;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_PROTOCOLEVENTS_H
