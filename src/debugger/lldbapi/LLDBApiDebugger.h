// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_LLDBAPI_LLDB_API_DEBUGGER_H
#define DAP_LLDBAPI_LLDB_API_DEBUGGER_H

#include <variant>
#include <string>
#include <memory>
#include <shared_mutex>

#include "Debugger.h"
#include "Protocol.h"
#include "DapLog.h"
#include "LLDB.h"
#include "BreakPointMap.h"
#include "FunctionBreakPointMap.h"
#include "DataBreakpointSet.h"
#include "InstructionBreakPointMap.h"
#include "ExceptionBreakpointSet.h"
#include "VariablesReferenceCreator.h"
#include "ContainerManager.h"
#include "DisassemblyManager.h"

namespace Dap {
using UniqueResponsePtr = std::unique_ptr<Dap::Protocol::Response>;
using UniqueRequestPtr = std::unique_ptr<Dap::Protocol::Request>;

class LLDBApiEventHandler;

enum class ShowDisassembly {
    NEVER,
    ALWAYS,
    AUTO
};

struct LaunchInfo {
    std::string workingDirectory;
    std::vector<std::string> argv;
    char const **envp;
    uint32_t launchFlags;
    bool stopAtEntry;
};

struct LLDBApiDebuggerOption : public DebuggerOption {
    std::string remotePlatform;
    ShowDisassembly disassemblyOption = ShowDisassembly::AUTO;
};

const std::string LLDB_API_EXCEPTION_CATCH = "catch";
const std::string LLDB_API_EXCEPTION_THROW = "throw";
const std::vector<std::string> LLDB_API_EXCEPTION_FILTERS = { LLDB_API_EXCEPTION_CATCH, LLDB_API_EXCEPTION_THROW };

class LLDBApiDebugger : public Debugger, public std::enable_shared_from_this<LLDBApiDebugger> {
public:
    explicit LLDBApiDebugger(const LLDBApiDebuggerOption &debuggerOption);
    virtual ~LLDBApiDebugger() = default;

    int OnDapRequest(UniqueRequestPtr request) override;
    int PreHandleRequest(Protocol::Request *request);
    void OnDapResponse(UniqueResponsePtr response) override;
    void SendStoppedEventInReverseDebug(bool isReverseMode = true);
    void Start() override;
    void Stop() override;
    // execute the lldb native command. return whether the command has been executed
    bool ExecCommand(const std::string &command);
    // save the result to @param: result.
    bool ExecCommand(const std::string &command, std::string &result);
    // save the result to @param: result and success of command to @param: success.
    bool ExecCommand(const std::string &command, std::string &result, bool &success, bool isLog = true);

    const LLDBApiDebuggerOption &GetOption() const
    {
        return option;
    }

    void SetOption(const LLDBApiDebuggerOption &debuggerOption)
    {
        option = debuggerOption;
    }

    std::map<int, int> &GetThreadMap()
    {
        return threadMap;
    }

    void SetCjThreadIdAndIndex(int key, int value)
    {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        threadMap[key] = value;
    }

    std::optional<int> GetCjThreadIndexById(int key) const
    {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        auto it = threadMap.find(key);
        if (it != threadMap.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    lldb::SBDebugger &GetSbDebugger()
    {
        return sbDebugger;
    }

    void SetSbDebugger(const lldb::SBDebugger &debugger)
    {
        sbDebugger = debugger;
    }

    lldb::SBTarget &GetSbTarget()
    {
        return sbTarget;
    }

    void SetSbTarget(const lldb::SBTarget &target)
    {
        sbTarget = target;
    }

    const lldb::SBListener &GetSBListener() const
    {
        return sbListener;
    }

    void SetSBListener(const lldb::SBListener &listener)
    {
        sbListener = listener;
    }

    std::shared_ptr<LLDBApiEventHandler> GetEventHandler() const
    {
        return eventHandler;
    }

    void SetEventHandler(std::shared_ptr<LLDBApiEventHandler> handler)
    {
        eventHandler = std::move(handler);
    }

    ContainerManager& GetContainerManager()
    {
        return containerManager;
    }

    DisassemblyManager &GetDisassemblyManager()
    {
        return disassemblyManager;
    }

    void SetDisassemblyManager(const DisassemblyManager &manager)
    {
        disassemblyManager = manager;
    }

    bool IsInDisassembly(const lldb::SBFrame sbFrame)
    {
        if (option.disassemblyOption == ShowDisassembly::NEVER) {
            return false;
        } else if (option.disassemblyOption == ShowDisassembly::ALWAYS) {
            return true;
        } else {
            return !sbFrame.GetLineEntry().IsValid();
        }
    }

    /* *
     * fill the response when handle request failed
     * @param responsePointer
     * @param error
     * @param requestSeq
     */
    UniqueResponsePtr CompleteFailureResponse(UniqueResponsePtr responsePointer, const std::string &error,
        unsigned int requestSeq)
    {
        if (responsePointer == nullptr) {
            LOG_ERROR("Response pointer is nullptr!");
            return nullptr;
        }
        LOG_ERROR(error);
        responsePointer->request_seq = requestSeq;
        responsePointer->seq = GetAndIncCurrentMessageSeq();
        responsePointer->message = error;
        responsePointer->success = false;
        return responsePointer;
    }

    /* *
     * fill the common response when handle request success
     * @param responsePointer
     * @param requestSeq
     */
    UniqueResponsePtr CompleteSuccessResponse(UniqueResponsePtr responsePointer, unsigned int requestSeq)
    {
        if (responsePointer == nullptr) {
            LOG_ERROR("Response pointer is nullptr!");
            return nullptr;
        }
        responsePointer->request_seq = requestSeq;
        responsePointer->seq = GetAndIncCurrentMessageSeq();
        responsePointer->success = true;
        return responsePointer;
    }

    /* *
     * init the common response
     * @param response
     * @param requestSeq
     * @param success
     */
    void InitResponse(Dap::Protocol::Response& response, unsigned int requestSeq, bool success)
    {
        response.request_seq = requestSeq;
        response.seq = GetAndIncCurrentMessageSeq();
        response.success = success;
    }

    lldb::LanguageType GetLanguageType()
    {
        return languageType;
    }

    void UpdateCategory()
    {
        lldb::SBTypeCategory typeCategory = sbDebugger.GetCategory(languageType);
        typeCategory.SetEnabled(!option.libraryRenderer);
    }

    inline void UpdateCategory(bool libraryRenderer)
    {
        lldb::SBTypeCategory typeCategory = sbDebugger.GetCategory(languageType);
        typeCategory.SetEnabled(!libraryRenderer);
    }

    void SendUndoEvent(const std::string& command);

    UniqueResponsePtr OnDapRequestInUndo(UniqueRequestPtr request,
                                         const std::string &command,
                                         unsigned int requestSeq);

    UniqueResponsePtr OnDapRequestInNormal(UniqueRequestPtr request,
                                           const std::string &command,
                                           unsigned int requestSeq);

    void SetMaxStringSummaryLength(const std::string &maxLength);
    std::string GetMaxStringSummaryLength();
    void GetModuleInfo(lldb::SBModule &sbModule, Dap::Protocol::Module &module);

    void GetModuleVersion(lldb::SBModule &sbModule, Dap::Protocol::Module &module);

    lldb::SBValue FindExprValue(lldb::SBFrame &frame, const std::string &expression);

    bool JustExpr(lldb::SBFrame &frame, const std::string &expression, bool isUpdateCategory);

    std::string GetExprValueResult(lldb::SBFrame &frame, const std::string &expression);

    std::string GetMemoryData(const std::string &hexAddr, uint64_t count, int64_t offset);

    std::optional<lldb::SBModule> GetModuleByUUID(const std::string &uuidStr);

    int GetIndexByUUID(const std::string &uuidStr);

    bool GetSourceMap(std::string& sourceMapStr, std::string& error);

    static std::unordered_map<std::string, std::string> ParseSourceMap(const std::string& sourceMapStr);

    bool ExecuteSourceMapCommand(std::string& command, std::string& error);

    lldb::SBFrame GetLiveFrame(int64_t containerId);

protected:
    virtual void Configure();
    virtual UniqueResponsePtr Initialize(UniqueRequestPtr request);
    virtual UniqueResponsePtr Launch(UniqueRequestPtr request);
    virtual UniqueResponsePtr Attach(UniqueRequestPtr request);
    virtual UniqueResponsePtr Continue(UniqueRequestPtr request);
    virtual UniqueResponsePtr Pause(UniqueRequestPtr request);
    virtual UniqueResponsePtr Disconnect(UniqueRequestPtr request);
    virtual UniqueResponsePtr ConfigurationDone(UniqueRequestPtr request);
    virtual UniqueResponsePtr Next(UniqueRequestPtr request);
    virtual UniqueResponsePtr StepIn(UniqueRequestPtr request);
    virtual UniqueResponsePtr StepOut(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetBreakpoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetFunctionBreakpoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetExceptionBreakpoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr DataBreakpointInfo(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetDataBreakpoints(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetInstructionBreakpoints(UniqueRequestPtr request);
    virtual UniqueResponsePtr DebugInConsole(UniqueRequestPtr request);
    virtual UniqueResponsePtr Threads(UniqueRequestPtr request);
    virtual UniqueResponsePtr StackTrace(UniqueRequestPtr request);
    virtual UniqueResponsePtr Scopes(UniqueRequestPtr request);
    virtual UniqueResponsePtr Variables(UniqueRequestPtr request);
    virtual UniqueResponsePtr Evaluate(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetVariable(UniqueRequestPtr request);
    virtual UniqueResponsePtr Source(UniqueRequestPtr request);
    virtual UniqueResponsePtr ReadMemory(UniqueRequestPtr request);
    virtual UniqueResponsePtr WriteMemory(UniqueRequestPtr request);
    virtual UniqueResponsePtr Disassemble(UniqueRequestPtr request);
    virtual UniqueResponsePtr SetExecutionPoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr ReverseStep(UniqueRequestPtr request);
    virtual UniqueResponsePtr Undo(UniqueRequestPtr request);
    virtual UniqueResponsePtr ReverseContinue(UniqueRequestPtr request);
    virtual UniqueResponsePtr GotoRecordPoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr RecordPoints(UniqueRequestPtr request);
    virtual UniqueResponsePtr CacheDataConfig(UniqueRequestPtr request);
    virtual UniqueResponsePtr ContinueInReverse(UniqueRequestPtr request);
    virtual UniqueResponsePtr StepInReverse(UniqueRequestPtr request);
    virtual UniqueResponsePtr RecordMessage(UniqueRequestPtr request);
    virtual UniqueResponsePtr GetBitmap(UniqueRequestPtr request);
    virtual UniqueResponsePtr LoadModules(UniqueRequestPtr request);
    virtual UniqueResponsePtr AddSourceMapping(UniqueRequestPtr request);
    virtual UniqueResponsePtr ResetSourceMapping(UniqueRequestPtr request);
    virtual UniqueResponsePtr BreakpointLocations(UniqueRequestPtr request);
    virtual UniqueResponsePtr AddSourceBreakpoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr DeleteBreakpoint(UniqueRequestPtr request);
    virtual UniqueResponsePtr VariableHexView(UniqueRequestPtr request);
    virtual UniqueResponsePtr CommandCompletion(Dap::UniqueRequestPtr request);

    void PreResume()
    {
        containerManager.Reset();
    }

protected:
    using RequestLLDBApiHandler =
        std::function<std::unique_ptr<Dap::Protocol::Response>(std::unique_ptr<Dap::Protocol::Request> request)>;
    std::map<std::string, RequestLLDBApiHandler> requestHandlerMap;
    LLDBApiDebuggerOption option;
    lldb::SBDebugger sbDebugger;
    lldb::SBTarget sbTarget;
    lldb::SBListener sbListener;
    ContainerManager containerManager;
    DisassemblyManager disassemblyManager;
    std::map<int, int> threadMap;
    mutable std::shared_mutex m_mtx;

private:
    bool AddInternalTypeSummaries();
    bool LoadInternalSummaries(lldb::SBTypeCategory category);
    std::string GetCodeRange(lldb::SBModule &sbModule);
    std::string GetAddressRange(lldb::SBModule &sbModule);
    std::string ExtractModuleFilePath(const std::string &commandResult);
    Dap::Protocol::Module GetRefreshedModule(std::string addSymbolFileResult);
    void FillNativeThreads(lldb::SBProcess& process, Dap::Protocol::ThreadsResponse& response);
    void FillCJThreads(lldb::SBProcess& process, Dap::Protocol::ThreadsResponse& response);
private:
    const static int THREAD_NAME_LENGTH_MAX = 32;

    std::shared_ptr<LLDBApiEventHandler> eventHandler = nullptr;
    lldb::LanguageType languageType = lldb::eLanguageTypeC_plus_plus_14;
};
};

#endif // DAP_LLDBAPI_LLDB_API_DEBUGGER_H