// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_DEFS_H
#define DAP_DEFS_H

#include <vector>
#include <any>
#include "json.hpp"

using json_t = nlohmann::json;
using any = std::any;

#define NS_DAP_BEGIN namespace Dap {
#define NS_DAP_END }

#define NS_PROTOCOL_BEGIN namespace Protocol {
#define NS_PROTOCOL_END }

#endif // DAP_DEFS_H
