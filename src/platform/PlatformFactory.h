// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_PLATFORMFACTORY_H
#define DAP_SERVER_PLATFORMFACTORY_H

#include <memory>
#include "Platform.h"
#include "LinuxPlatform.h"
#include "WindowsPlatform.h"

class PlatformFactory {
public:
    static std::unique_ptr <Platform> Generate()
    {
#ifdef _WIN32
            return std::make_unique<WindowsPlatform>();
#else
            return std::make_unique<LinuxPlatform>();
#endif
    }
};

#endif // DAP_SERVER_PLATFORMFACTORY_H
