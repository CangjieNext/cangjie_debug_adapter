// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "GetBitmapApiHandler.h"
#include "ValueParser.h"
#include "ResponseManager.h"
#include "UniqueIdCreator.h"

using namespace std;
using namespace Dap;
using namespace Dap::Protocol;
using namespace lldb;

UniqueResponsePtr GetBitmapApiHandler::HandleRequest(Dap::UniqueRequestPtr request)
{
    currentRequest = std::move(request);
    auto reqPtr = dynamic_cast<GetBitmapRequest*>(currentRequest.get());
    if (reqPtr == nullptr) {
        error = "ERROR: GetBitmapRequest pointer conversion failed.";
        return debugger->CompleteFailureResponse(std::make_unique<GetBitmapResponse>(), error, currentRequest->seq);
    }
    int64_t frameId = reqPtr->arguments.frameId.has_value() ? reqPtr->arguments.frameId.value() : -1;
    SBFrame frame;
    if (frameId == -1) {
        SBProcess sbProcess = debugger->GetSbTarget().GetProcess();
        SBThread thread = sbProcess.GetSelectedThread();
        frame = thread.GetSelectedFrame();
    } else {
        ContainerInfo containerInfo = debugger->GetContainerManager().GetContainerInfo(frameId);
        auto *framePtr = std::get_if<SBFrame>(&containerInfo.container);
        if (framePtr != nullptr) {
            frame = *framePtr;
        }
    }
    std::string varName = reqPtr->arguments.varName;
    if (!GetAttr(frame, varName)) {
        return debugger->CompleteFailureResponse(std::make_unique<GetBitmapResponse>(), error, currentRequest->seq);
    }
    bool hasData = reqPtr->arguments.hasData.has_value() && reqPtr->arguments.hasData.value();
    if (hasData) {
        dataSize = GetDataSize();
        if (!GetData(frame, varName, dataSize)) {
            return debugger->CompleteFailureResponse(std::make_unique<GetBitmapResponse>(), error, currentRequest->seq);
        }
    }
    std::unique_ptr<GetBitmapResponse> responsePtr = std::make_unique<GetBitmapResponse>();
    debugger->InitResponse(*responsePtr,  currentRequest->seq, true);
    responsePtr->bitmap.width = width;
    responsePtr->bitmap.height = height;
    responsePtr->bitmap.format.colorFormat = static_cast<int>(colorFormat);
    responsePtr->bitmap.format.alphaFormat = static_cast<int>(alphaFormat);
    if (hasData) {
        responsePtr->bitmap.data = data;
        responsePtr->bitmap.size = dataSize;
    }
    RecordResponse(*responsePtr);
    return responsePtr;
}

bool GetBitmapApiHandler::GetAttr(SBFrame &frame, const std::string &varName)
{
    if (!frame.IsValid()) {
        error = "ERROR: Invalid frame id.";
        return false;
    }
    // get bitmap width
    std::string exprGetWidth = "(int)OH_Drawing_BitmapGetWidth(" + varName + ")";
    std::string widthStr = debugger->GetExprValueResult(frame, exprGetWidth);
    if (widthStr.empty() || !CommonUtils::StringToIntExt(widthStr, width)) {
        error = "ERROR: Failed to get image width.";
        return false;
    }
    // get bitmap height
    std::string exprGetHeight = "(int)OH_Drawing_BitmapGetHeight(" + varName + ")";
    std::string heightStr = debugger->GetExprValueResult(frame, exprGetHeight);
    if (heightStr.empty() || !CommonUtils::StringToIntExt(heightStr, height)) {
        error = "ERROR: Failed to get image height.";
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = "ERROR: Either width or height must be greater than zero.";
        return false;
    }
    // API12 & API12+ support OH_Drawing_BitmapGetColorFormat and OH_Drawing_BitmapGetAlphaFormat
    std::string exprGetClrFmt = "(int)OH_Drawing_BitmapGetColorFormat(" + varName + ")";
    std::string clrFmtStr = debugger->GetExprValueResult(frame, exprGetClrFmt);
    int clrFmtVal = -1;
    if (clrFmtStr.empty() || !CommonUtils::StringToIntExt(clrFmtStr, clrFmtVal)) {
        LOG_WARN("WARN: Failed to get image color type.");
        // API12- support OH_Drawing_BitmapGetImageInfo
        if (!GetFormatByImageInfo(frame, varName)) {
            return false;
        }
        return true;
    }
    colorFormat = ToClrFmt(clrFmtVal);
    std::string exprGetAlphaFmt = "(int)OH_Drawing_BitmapGetAlphaFormat(" + varName + ")";
    std::string alphaFmtStr = debugger->GetExprValueResult(frame, exprGetAlphaFmt);
    int alphaFmtVal = -1;
    if (alphaFmtStr.empty() || !CommonUtils::StringToIntExt(alphaFmtStr, alphaFmtVal)) {
        error = "ERROR: Failed to get image alpha type.";
        return false;
    }
    alphaFormat = ToAlphaFmt(alphaFmtVal);
    return true;
}

bool GetBitmapApiHandler::GetFormatByImageInfo(lldb::SBFrame &frame, const std::string &varName)
{
    // new OH_Drawing_Image_Info variable
    std::string imgInfoVarName = UniqueIdCreator::IdCreator().ToString("$lldb_inner_oh_drawing_img_info_xyz123_", true);
    std::string exprNewImageInfo = "int " + imgInfoVarName + "[4]";
    if (!debugger->JustExpr(frame, exprNewImageInfo, true)) {
        error = "ERROR: Failed to new OH_Drawing_Image_Info variable.";
        return false;
    }
    // get image info
    std::string exprGetImageInfo = "(void)OH_Drawing_BitmapGetImageInfo(" + varName + ", &" + imgInfoVarName + ")";
    if (!debugger->JustExpr(frame, exprGetImageInfo, true)) {
        error = "ERROR: Failed to get image info.";
        return false;
    }
    // get format info, [width, height, color format, alpha format]
    std::string exprGetClrFmt = imgInfoVarName + "[2]";
    std::string clrFmtStr = debugger->GetExprValueResult(frame, exprGetClrFmt);
    int clrFmtVal = -1;
    if (clrFmtStr.empty() || !CommonUtils::StringToIntExt(clrFmtStr, clrFmtVal)) {
        error = "ERROR: Failed to get image color type by image info.";
        return false;
    }
    colorFormat = ToClrFmt(clrFmtVal);
    std::string exprGetAlphaFmt = imgInfoVarName + "[3]";
    std::string alphaFmtStr = debugger->GetExprValueResult(frame, exprGetAlphaFmt);
    int alphaFmtVal = -1;
    if (alphaFmtStr.empty() || !CommonUtils::StringToIntExt(alphaFmtStr, alphaFmtVal)) {
        error = "ERROR: Failed to get image alpha type by image info.";
        return false;
    }
    alphaFormat = ToAlphaFmt(alphaFmtVal);
    return true;
}

bool GetBitmapApiHandler::GetData(SBFrame &frame, const std::string &varName, uint64_t size)
{
    if (size == 0) {
        error = "ERROR: Data size is zero.";
        return false;
    }
    // get pixels address
    std::string exprGetPixels = "(char *)OH_Drawing_BitmapGetPixels(" + varName + ")";
    std::string pixelsStr = debugger->GetExprValueResult(frame, exprGetPixels);
    std::string::size_type beginPos = pixelsStr.find_first_of('\"');
    if (beginPos == std::string::npos) {
        error = "Error: Failed to get data address.";
        return false;
    }
    std::string addrStr = pixelsStr.substr(0, beginPos);
    addrStr = CommonUtils::TrimString(addrStr, " ");
    data = debugger->GetMemoryData(addrStr, size, 0);
    if (data.empty()) {
        error = "Error: Failed to get bitmap data.";
        return false;
    }
    return true;
}

uint64_t GetBitmapApiHandler::GetDataSize()
{
    static std::map<ColorFormat, int> fmt2size = {
        { ColorFormat::COLOR_FORMAT_DEFAULT, 4 },
        { ColorFormat::COLOR_FORMAT_UNKNOWN, 0 },
        { ColorFormat::COLOR_FORMAT_ALPHA_8, 1 },
        { ColorFormat::COLOR_FORMAT_RGB_565, 2 },
        { ColorFormat::COLOR_FORMAT_ARGB_4444, 2 },
        { ColorFormat::COLOR_FORMAT_RGBA_8888, 4 },
        { ColorFormat::COLOR_FORMAT_BGRA_8888, 4 }
    };
    if (fmt2size.count(colorFormat) == 0) {
        return 0;
    }
    return fmt2size[colorFormat] * width * height;
}

void GetBitmapApiHandler::RecordResponse(GetBitmapResponse &response)
{
    std::optional<json_t> jsonOpt = ResponseManager::Instance().ToJson(response);
    if (!jsonOpt->empty() || jsonOpt.value()["body"].contains("data")) {
        jsonOpt.value()["body"]["data"] = "...";
        string content = CommonUtils::JsonDump(jsonOpt.value());
        LOG_INFO("send response: \n", content);
    }
}

ColorFormat GetBitmapApiHandler::ToClrFmt(int val)
{
    static std::map<int, ColorFormat> int2fmt = {
        {-1, ColorFormat::COLOR_FORMAT_DEFAULT},
        {0, ColorFormat::COLOR_FORMAT_UNKNOWN},
        {1, ColorFormat::COLOR_FORMAT_ALPHA_8},
        {2, ColorFormat::COLOR_FORMAT_RGB_565},
        {3, ColorFormat::COLOR_FORMAT_ARGB_4444},
        {4, ColorFormat::COLOR_FORMAT_RGBA_8888},
        {5, ColorFormat::COLOR_FORMAT_BGRA_8888}
    };
    if (int2fmt.count(val) == 0) {
        return ColorFormat::COLOR_FORMAT_UNKNOWN;
    }
    return int2fmt[val];
}

AlphaFormat GetBitmapApiHandler::ToAlphaFmt(int val)
{
    static std::map<int, AlphaFormat> int2fmt = {
        {-1, AlphaFormat::ALPHA_FORMAT_DEFAULT},
        {0, AlphaFormat::ALPHA_FORMAT_UNKNOWN},
        {1, AlphaFormat::ALPHA_FORMAT_OPAQUE},
        {2, AlphaFormat::ALPHA_FORMAT_PREMUL},
        {3, AlphaFormat::ALPHA_FORMAT_UNPREMUL}
    };
    if (int2fmt.count(val) == 0) {
        return AlphaFormat::ALPHA_FORMAT_UNKNOWN;
    }
    return int2fmt[val];
}