// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_EVENT_MANAGER_H
#define DAP_EVENT_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <utility>
#include <mutex>
#include <optional>

#include "defs.h"
#include "Protocol.h"

NS_DAP_BEGIN
NS_PROTOCOL_BEGIN

template<typename EVENT> std::optional<json_t> ToEventJson(const EVENT &e);
template<> std::optional<json_t> ToEventJson<InitializedEvent>(const InitializedEvent &event);
template<> std::optional<json_t> ToEventJson<StoppedEvent>(const StoppedEvent &event);
template<> std::optional<json_t> ToEventJson<ContinuedEvent>(const ContinuedEvent &event);
template<> std::optional<json_t> ToEventJson<ExitedEvent>(const ExitedEvent &event);
template<> std::optional<json_t> ToEventJson<TerminatedEvent>(const TerminatedEvent &event);
template<> std::optional<json_t> ToEventJson<BreakpointEvent>(const BreakpointEvent &event);
template<> std::optional<json_t> ToEventJson<CapabilitiesEvent>(const CapabilitiesEvent &event);
template<> std::optional<json_t> ToEventJson<OutputEvent>(const OutputEvent &event);
template<> std::optional<json_t> ToEventJson<ModuleEvent>(const ModuleEvent &event);
template<> std::optional<json_t> ToEventJson<LoadedSourceEvent>(const LoadedSourceEvent &event);
template<> std::optional<json_t> ToEventJson<ProcessEvent>(const ProcessEvent &event);
template<> std::optional<json_t> ToEventJson<ProgressStartEvent>(const ProgressStartEvent &event);
template<> std::optional<json_t> ToEventJson<ProgressUpdateEvent>(const ProgressUpdateEvent &event);
template<> std::optional<json_t> ToEventJson<ProgressEndEvent>(const ProgressEndEvent &event);
template<> std::optional<json_t> ToEventJson<InvalidatedEvent>(const InvalidatedEvent &event);
template<> std::optional<json_t> ToEventJson<RecordPointsUpdateEvent>(const RecordPointsUpdateEvent &event);
template<> std::optional<json_t> ToEventJson<ThreadEvent>(const ThreadEvent &event);
template<> std::optional<json_t> ToEventJson<ModulesEvent>(const ModulesEvent &event);

class EventManager {
public:
    EventManager(const EventManager&) = delete;
    EventManager &operator=(const EventManager&) = delete;
    EventManager(const EventManager &&) = delete;
    const EventManager &operator=(const EventManager &&) = delete;
    static EventManager &Instance()
    {
        static EventManager instance;
        return instance;
    }
    using EventToJsonFunc = std::function<std::optional<json_t>(const Event &)>;
    void Register();
    void UnRegister();
    bool IsRegister();
    std::optional<EventManager::EventToJsonFunc> GetEventJsonFunc(const std::string &event);
    std::optional<json_t> ToJson(const Event& event);
private:
    EventManager() = default;
    ~EventManager() = default;
    static std::optional<json_t> ToInitializeJson(const Event &event);
    static std::optional<json_t> ToStoppedJson(const Event &event);
    static std::optional<json_t> ToContinuedJson(const Event &event);
    static std::optional<json_t> ToExitedJson(const Event &event);
    static std::optional<json_t> ToTerminatedJson(const Event &event);
    static std::optional<json_t> ToBreakpointJson(const Event &event);
    static std::optional<json_t> ToCapabilitiesJson(const Event &event);
    static std::optional<json_t> ToOutputJson(const Event &event);
    static std::optional<json_t> ToModuleEventJson(const Event &event);
    static std::optional<json_t> ToLoadSourceEventJson(const Event &event);
    static std::optional<json_t> ToProcessEventJson(const Event &event);
    static std::optional<json_t> ToProgressStartEventJson(const Event &event);
    static std::optional<json_t> ToProgressUpdateEventJson(const Event &event);
    static std::optional<json_t> ToProgressEndEventJson(const Event &event);
    static std::optional<json_t> ToInvalidatedEventJson(const Event &event);
    static std::optional<json_t> ToRecordPointsUpdateEventJson(const Event &event);
    static std::optional<json_t> ToThreadEventJson(const Event &event);
    static std::optional<json_t> ToModulesEventJson(const Event &event);

private:
    std::mutex mutex;
    bool isRegister = false;
    std::map<std::string, EventToJsonFunc> eventFactory;
};

NS_PROTOCOL_END
NS_DAP_END

#endif // DAP_EVENT_MANAGER_H
