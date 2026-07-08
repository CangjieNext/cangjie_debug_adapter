# dap_server

* Overview

  A C++ debug adapter using DAP (debug adapter protocol) .

* Project Directories
```
├─build                  // 构建脚本
├─open_source            // 第三方库
├─src
│  ├─channel             // Message通道
│  ├─common              // 内部工具
│  ├─core                // 核心实现
│  ├─debugger            // 调试器适配层
│  │  ├─gdb
│  │  ├─lldb
│  │  └─mi
│  │     ├─output        // mi回显处理
│  │     └─handler       // mi公共handler
│  │       ├─breakpoint  // 断点相关
│  │       ├─control     // 程序控制相关
│  │       └─display     // 展示相关
│  ├─platform            // 平台适配层
│  │  ├─darwin
│  │  ├─linux
│  │  └─windows
│  └─protocol            // DAP协议实现
└─test                   // 测试相关
```

* Git Branch Arrangement

| Branch Type    | Branch Name Rule |
| -------------- | ---------------- |
| master branch  | master           |
| develop branch | dev              |
| feature branch | feature_xxx      |
| issue branch   | issue(num)_xxx   |

## Develop Environment

* Windows

| Tool    | Description                                      |
| ------- | ------------------------------------------------ |
| mingw64 | g++ & gdb，vesion 8.1.0 ( same with lsp server ) |
| cmake   | version 3.16 and above                           |
| ninja   | 1.10.0 and above                                 |
| python  | python 3.6                                       |
| gcovr   | a code coverage tool                             |

*  Linux

| Tool               | Description                  |
| ------------------ | ---------------------------- |
| compiler           | g++ version 7.5 and above    |
| cmake              | version 3.16 and above       |
| ninja              | 1.10.0 and above             |
| python             | python 3.6                   |
| aarch64-linux-gnu  | Cross-compilation toolchain  |
| gcovr              | a code coverage tool         |

* MacOS

| Tool     | Description            |
| -------- | ---------------------- |
| compiler | clang \| g++           |
| cmake    | version 3.16 and above |
| ninja    | 1.10.0 and above       |
| python   | python 3.6             |
| gcovr    | a code coverage tool   |