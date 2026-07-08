// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <iostream>
int main(int argc, const char* argv[])
{
    std::cout << "arg count = " << argc << "\n";
    std::cout << "args :\n";
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << "\n";
    }
    std::string cmd;
    int mode = 0;
    while (std::cin >> cmd) {
        if (cmd == "exit") {
            std::cout << "echo exit.\n";
            break;
        }
        if (cmd == "stderr") {
            mode = 1;
            std::cout << "change mode to stderr\n";
        } else if (cmd == "stdout") {
            mode = 0;
            std::cout << "change mode to stdout\n";
        } else {
            if (mode == 0) {
                std::cout << (cmd + "\n");
            } else {
                std::cerr << (cmd + "\n");
            }
        }
    }
    return 0;
}