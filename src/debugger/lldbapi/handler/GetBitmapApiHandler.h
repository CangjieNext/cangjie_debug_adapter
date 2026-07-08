// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_GETBITMAPAPIHANDLER_H
#define DAP_SERVER_GETBITMAPAPIHANDLER_H

#include "Protocol.h"
#include "RequestApiHandlerBase.h"

namespace Dap {
enum class ColorFormat : int {
    COLOR_FORMAT_DEFAULT = -1,
    COLOR_FORMAT_UNKNOWN,
    COLOR_FORMAT_ALPHA_8,
    COLOR_FORMAT_RGB_565,
    COLOR_FORMAT_ARGB_4444,
    COLOR_FORMAT_RGBA_8888,
    COLOR_FORMAT_BGRA_8888
};
enum class AlphaFormat : int {
    ALPHA_FORMAT_DEFAULT = -1,
    ALPHA_FORMAT_UNKNOWN,
    ALPHA_FORMAT_OPAQUE,
    ALPHA_FORMAT_PREMUL,
    ALPHA_FORMAT_UNPREMUL
};
class GetBitmapApiHandler : public RequestApiHandlerBase {
public:
    explicit GetBitmapApiHandler(std::shared_ptr<LLDBApiDebugger> apiDebugger) : RequestApiHandlerBase(apiDebugger) {};
    ~GetBitmapApiHandler() override = default;
    UniqueResponsePtr HandleRequest(UniqueRequestPtr request) override;
private:
    bool GetAttr(lldb::SBFrame &frame, const std::string &varName);
    bool GetFormatByImageInfo(lldb::SBFrame &frame, const std::string &varName);
    bool GetData(lldb::SBFrame &frame, const std::string &varName, uint64_t size);
    uint64_t GetDataSize();
    void RecordResponse(Dap::Protocol::GetBitmapResponse &response);
    ColorFormat ToClrFmt(int val);
    AlphaFormat ToAlphaFmt(int val);
private:
    int width = -1;
    int height = -1;
    ColorFormat colorFormat = ColorFormat::COLOR_FORMAT_DEFAULT;
    AlphaFormat alphaFormat = AlphaFormat::ALPHA_FORMAT_DEFAULT;
    uint64_t dataSize = 0;
    std::string data;
    std::string error;
};
};

#endif // DAP_SERVER_GETBITMAPAPIHANDLER_H
