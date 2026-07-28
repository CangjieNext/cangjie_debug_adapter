// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_PROTOCOLSUITE_H
#define DAP_SERVER_PROTOCOLSUITE_H

class ProtocolSuite : public testing::Test {
    virtual void SetUp()
    {
        std::cout << "SetUp" << std::endl;
    }

    virtual void TearDown()
    {
        std::cout << "TearDown" << std::endl;
    }
};


#endif // DAP_SERVER_PROTOCOLSUITE_H
