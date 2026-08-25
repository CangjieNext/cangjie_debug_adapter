// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "EventManager.h"
#include "TypeUtils.h"
#include "DapLog.h"

using namespace Dap::Protocol;

void EventManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (isRegister) {
        return;
    }
    eventFactory.emplace(EVENT_INITIALIZED, ToInitializeJson);
    eventFactory.emplace(EVENT_STOPPED, ToStoppedJson);
    eventFactory.emplace(EVENT_CONTINUED, ToContinuedJson);
    eventFactory.emplace(EVENT_EXITED, ToExitedJson);
    eventFactory.emplace(EVENT_TERMINATED, ToTerminatedJson);
    eventFactory.emplace(EVENT_BREAKPOINT, ToBreakpointJson);
    eventFactory.emplace(EVENT_CAPABILITIES, ToCapabilitiesJson);
    eventFactory.emplace(EVENT_OUTPUT, ToOutputJson);
    eventFactory.emplace(EVENT_MODULE, ToModuleEventJson);
    eventFactory.emplace(EVENT_LOADED_SOURCE, ToLoadSourceEventJson);
    eventFactory.emplace(EVENT_PROCESS, ToProcessEventJson);
    eventFactory.emplace(EVENT_PROGRESS_START, ToProgressStartEventJson);
    eventFactory.emplace(EVENT_PROGRESS_UPDATE, ToProgressUpdateEventJson);
    eventFactory.emplace(EVENT_PROGRESS_END, ToProgressEndEventJson);
    eventFactory.emplace(EVENT_INVALIDATED, ToInvalidatedEventJson);
    eventFactory.emplace(EVENT_RECORD_UPDATE_POINTS, ToRecordPointsUpdateEventJson);
    eventFactory.emplace(EVENT_THREAD, ToThreadEventJson);
    eventFactory.emplace(EVENT_MODULES, ToModulesEventJson);
    isRegister = true;
}

void EventManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!isRegister) {
        return;
    }
    eventFactory.clear();
    isRegister = false;
}

bool EventManager::IsRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    return isRegister;
}

std::optional<EventManager::EventToJsonFunc> EventManager::GetEventJsonFunc(const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (eventFactory.count(event) == 0) {
        return std::nullopt;
    }
    return eventFactory[event];
}

std::optional<json_t> EventManager::ToJson(const Event &event)
{
    std::string eventString = event.event;
    std::optional<EventManager::EventToJsonFunc> func = GetEventJsonFunc(eventString);
    if (!func.has_value()) {
        LOG_ERROR("not find target function, event is:", event.event);
        return std::nullopt;
    }
    return func.value()(event);
}

template <typename EVENT> std::optional<json_t> ToEventJson(const EVENT &e)
{
    (void)e;
    return std::nullopt;
}

static void SetEventJsonBaseInfo(const Event &event, json_t &json)
{
    json["seq"] = event.seq;
    json["type"] = ENUM_TO_STR(event.type).value();
    json["event"] = event.event;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<InitializedEvent>(const InitializedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<StoppedEvent>(const StoppedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json_t body;
    StoppedEventBody stoppedEventBody = event.body;
    body["reason"] = ENUM_TO_STR(stoppedEventBody.reason).value();
    SetJsonOptional(body, stoppedEventBody.description, "description");
    SetJsonOptional(body, stoppedEventBody.threadId, "threadId");
    SetJsonOptional(body, stoppedEventBody.preserveFocusHint, "preserveFocusHint");
    SetJsonOptional(body, stoppedEventBody.text, "text");
    SetJsonOptional(body, stoppedEventBody.allThreadsStopped, "allThreadsStopped");
    SetJsonOptional(body, stoppedEventBody.isReverseMode, "isReverseMode");
    SetJsonOptional(body, stoppedEventBody.recordPointId, "recordPointId");
    if (stoppedEventBody.hitBreakpointIds.has_value()) {
        json_t hitBreakpointIds = json_t::array();
        for (auto hitBreakpointId : stoppedEventBody.hitBreakpointIds.value()) {
            hitBreakpointIds.emplace_back(hitBreakpointId);
        }
        body["hitBreakpointIds"] = hitBreakpointIds;
    }
    json["body"] = body;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ContinuedEvent>(const ContinuedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json_t body;
    ContinuedEventBody continuedEventBody = event.body;
    body["threadId"] = continuedEventBody.threadId;
    if (continuedEventBody.allThreadsContinued.has_value()) {
        body["allThreadsContinued"] = continuedEventBody.allThreadsContinued.value();
    }
    json["body"] = body;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ExitedEvent>(const ExitedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["exitCode"] = event.body.exitCode;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<TerminatedEvent>(const TerminatedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    if (event.body.has_value()) {
        /**
         * A debug adapter may set 'restart' to true (or to an arbitrary object) to
         * request that the front end restarts the session.
         * The value is not interpreted by the client and passed unmodified as an
         * attribute '__restart' to the 'launch' and 'attach' requests.
        */
        if (event.body.value().restart.has_value() && event.body.value().restart.value().type() == typeid(bool)) {
            json["body"]["restart"] = std::any_cast<bool>(event.body.value().restart.value());
        }
    }
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<BreakpointEvent>(const BreakpointEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["reason"] = ENUM_TO_STR(event.body.reason).value();
    json["body"]["breakpoint"]["verified"] = event.body.breakpoint.verified;
    SetJsonOptional(json, event.body.breakpoint.id, "body", "breakpoint", "id");
    SetJsonOptional(json, event.body.breakpoint.message, "body", "breakpoint", "message");
    if (event.body.breakpoint.source.has_value()) {
        json["body"]["breakpoint"]["source"] = GetTypeJson(event.body.breakpoint.source.value()).value();
    }
    SetJsonOptional(json, event.body.breakpoint.line, "body", "breakpoint", "line");
    SetJsonOptional(json, event.body.breakpoint.column, "body", "breakpoint", "column");
    SetJsonOptional(json, event.body.breakpoint.endLine, "body", "breakpoint", "endLine");
    SetJsonOptional(json, event.body.breakpoint.endColumn, "body", "breakpoint", "endColumn");
    SetJsonOptional(json, event.body.breakpoint.instructionReference,
        "body", "breakpoint", "instructionReference");
    SetJsonOptional(json, event.body.breakpoint.offset, "body", "breakpoint", "offset");
    SetJsonOptional(json, event.body.breakpoint.condition, "body", "breakpoint", "condition");
    if (event.body.dataBreakpoint.has_value()) {
        json["body"]["dataBreakpoint"]["dataId"] = event.body.dataBreakpoint.value().dataId;
        SetJsonOptional(json, event.body.dataBreakpoint.value().condition,
            "body", "dataBreakpoint", "condition");
        if (event.body.dataBreakpoint.value().accessType.has_value()) {
            SetJsonOptional(json,
                ENUM_TO_STR<DataBreakpointAccessType>(event.body.dataBreakpoint.value().accessType.value()),
                "body", "dataBreakpoint", "accessType");
        }
    }
    SetJsonOptional(json, event.body.hitCount, "body", "hitCount");
    SetJsonOptional(json, event.body.ignoreCount, "body", "ignoreCount");
    SetJsonOptional(json, event.body.description, "body", "description");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<CapabilitiesEvent>(const CapabilitiesEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["capabilities"] = GetTypeJson<Capabilities>(event.body.capabilities).value();
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<OutputEvent>(const OutputEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["output"] = event.body.output;
    SetJsonOptional(json, event.body.variablesReference, "body", "variablesReference");
    SetJsonOptional(json, event.body.line, "body", "line");
    SetJsonOptional(json, event.body.column, "body", "column");
    if (event.body.category.has_value()) {
        SetJsonOptional(json, ENUM_TO_STR(event.body.category.value()), "body", "category");
    }
    if (event.body.group.has_value()) {
        SetJsonOptional(json, ENUM_TO_STR(event.body.group.value()), "body", "group");
    }
    if (event.body.source.has_value()) {
        SetJsonOptional(json, GetTypeJson(event.body.source.value()), "body", "source");
    }
    // json format
    if (event.body.data.has_value()) {
        json["body"]["data"] = std::any_cast<json_t>(event.body.data.value());
    }
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ModuleEvent>(const ModuleEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    SetJsonOptional(json, ENUM_TO_STR(event.body.reason), "body", "reason");
    SetJsonOptional(json, GetTypeJson<Module>(event.body.module), "body", "module");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<LoadedSourceEvent>(const LoadedSourceEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    SetJsonOptional(json, ENUM_TO_STR(event.body.reason), "body", "reason");
    SetJsonOptional(json, GetTypeJson<Source>(event.body.source), "body", "source");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ProcessEvent>(const ProcessEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["name"] = event.body.name;
    SetJsonOptional(json, event.body.systemProcessId, "body", "systemProcessId");
    SetJsonOptional(json, event.body.isLocalProcess, "body", "isLocalProcess");
    if (event.body.startMethod.has_value()) {
        SetJsonOptional(json, ENUM_TO_STR(event.body.startMethod.value()), "body", "startMethod");
    }
    SetJsonOptional(json, event.body.pointerSize, "body", "pointerSize");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ProgressStartEvent>(const ProgressStartEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["progressId"] = event.body.progressId;
    json["body"]["title"] = event.body.title;
    SetJsonOptional(json, event.body.requestId, "body", "requestId");
    SetJsonOptional(json, event.body.cancellable, "body", "cancellable");
    SetJsonOptional(json, event.body.message, "body", "message");
    SetJsonOptional(json, event.body.percentage, "body", "percentage");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ProgressUpdateEvent>(const ProgressUpdateEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["progressId"] = event.body.progressId;
    SetJsonOptional(json, event.body.message, "body", "message");
    SetJsonOptional(json, event.body.percentage, "body", "percentage");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ProgressEndEvent>(const ProgressEndEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["progressId"] = event.body.progressId;
    SetJsonOptional(json, event.body.message, "body", "message");
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<InvalidatedEvent>(const InvalidatedEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    SetJsonOptional(json, event.body.threadId, "body", "threadId");
    SetJsonOptional(json, event.body.stackFrameId, "body", "stackFrameId");
    if (event.body.areas.has_value()) {
        json["body"]["areas"] = json_t::array();
        for (auto area : event.body.areas.value()) {
            json["body"]["areas"].push_back(ENUM_TO_STR(area).value());
        }
    }
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<RecordPointsUpdateEvent>(
    const RecordPointsUpdateEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json_t body;
    body["removedRecordPointIds"] = json_t::array();
    body["newRecordPoints"] = json_t::array();
    RecordPointsUpdateEventBody eventBody = event.body;
    if (eventBody.removedRecordPointIds.has_value()) {
        for (auto recordPointId : eventBody.removedRecordPointIds.value()) {
            body["removedRecordPointIds"].emplace_back(recordPointId);
        }
    }
    if (eventBody.newRecordPoints.has_value()) {
        for (const auto &recordPoint : eventBody.newRecordPoints.value()) {
            json_t point;
            point["id"] = recordPoint.id;
            point["threadId"] = recordPoint.threadId;
            point["threadName"] = recordPoint.threadName;
            point["dateTimeStamp"] = recordPoint.dateTimeStamp;
            point["timeStamp"] = recordPoint.timeStamp;
            point["type"] = recordPoint.reason;
            SetJsonOptional(point, recordPoint.coroutineId, "coroutineId");
            body["newRecordPoints"].emplace_back(point);
        }
    }
    json["body"] = body;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ThreadEvent>(const ThreadEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json["body"]["reason"] = event.body.reason;
    json["body"]["threadId"] = event.body.threadId;
    return json;
}

template <> std::optional<json_t> Dap::Protocol::ToEventJson<ModulesEvent>(const ModulesEvent &event)
{
    json_t json;
    SetEventJsonBaseInfo(event, json);
    json_t body;
    body["reason"] = event.body.reason;
    body["modules"] = json_t::array();
    for (const auto &module : event.body.moduleList) {
        body["modules"].emplace_back(GetTypeJson<Module>(module).value());
    }
    json["body"] = body;
    return json;
}

// EventManager: event to json
std::optional<json_t> EventManager::ToInitializeJson(const Event &event)
{
    return ToEventJson<InitializedEvent>((InitializedEvent &)event);
}

std::optional<json_t> EventManager::ToStoppedJson(const Event &event)
{
    return ToEventJson<StoppedEvent>((StoppedEvent &)event);
}

std::optional<json_t> EventManager::ToContinuedJson(const Event &event)
{
    return ToEventJson<ContinuedEvent>((ContinuedEvent &)event);
}

std::optional<json_t> EventManager::ToExitedJson(const Event &event)
{
    return ToEventJson<ExitedEvent>((ExitedEvent &)event);
}

std::optional<json_t> EventManager::ToTerminatedJson(const Event &event)
{
    return ToEventJson<TerminatedEvent>((TerminatedEvent &)event);
}

std::optional<json_t> EventManager::ToBreakpointJson(const Event &event)
{
    return ToEventJson<BreakpointEvent>((BreakpointEvent &)event);
}

std::optional<json_t> EventManager::ToCapabilitiesJson(const Event &event)
{
    return ToEventJson<CapabilitiesEvent>((CapabilitiesEvent &)event);
}

std::optional<json_t> EventManager::ToOutputJson(const Event &event)
{
    return ToEventJson<OutputEvent>((OutputEvent &)event);
}

std::optional<json_t> EventManager::ToModuleEventJson(const Event &event)
{
    return ToEventJson<ModuleEvent>((ModuleEvent &)event);
}

std::optional<json_t> EventManager::ToLoadSourceEventJson(const Event &event)
{
    return ToEventJson<LoadedSourceEvent>((LoadedSourceEvent &)event);
}

std::optional<json_t> EventManager::ToProcessEventJson(const Event &event)
{
    return ToEventJson<ProcessEvent>((ProcessEvent &)event);
}

std::optional<json_t> EventManager::ToProgressStartEventJson(const Event &event)
{
    return ToEventJson<ProgressStartEvent>((ProgressStartEvent &)event);
}

std::optional<json_t> EventManager::ToProgressUpdateEventJson(const Event &event)
{
    return ToEventJson<ProgressUpdateEvent>((ProgressUpdateEvent &)event);
}

std::optional<json_t> EventManager::ToProgressEndEventJson(const Event &event)
{
    return ToEventJson<ProgressEndEvent>((ProgressEndEvent &)event);
}

std::optional<json_t> EventManager::ToInvalidatedEventJson(const Event &event)
{
    return ToEventJson<InvalidatedEvent>((InvalidatedEvent &)event);
}

std::optional<json_t> EventManager::ToRecordPointsUpdateEventJson(const Event &event)
{
    return ToEventJson<RecordPointsUpdateEvent>((RecordPointsUpdateEvent &)event);
}

std::optional<json_t> EventManager::ToThreadEventJson(const Event &event)
{
    return ToEventJson<ThreadEvent>((ThreadEvent &)event);
}

std::optional<json_t> EventManager::ToModulesEventJson(const Event &event)
{
    return ToEventJson<ModulesEvent>((ModulesEvent &)event);
}
