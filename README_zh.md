# dap_server

* 概述

  一个基于 DAP（Debug Adapter Protocol）协议实现的 C++ 调试适配器。

* 项目目录结构
```
├─build                  // 构建脚本
├─open_source            // 第三方库
├─src
│  ├─channel             // 消息通道
│  ├─common              // 内部工具
│  ├─core                // 核心实现
│  ├─debugger            // 调试器适配层
│  │  ├─gdb
│  │  ├─lldb
│  │  └─mi
│  │     ├─output        // MI 输出解析
│  │     └─handler       // MI 公共处理器
│  │       ├─breakpoint  // 断点相关
│  │       ├─control     // 程序控制相关
│  │       └─display     // 展示相关
│  ├─platform            // 平台适配层
│  │  ├─darwin
│  │  ├─linux
│  │  └─windows
│  └─protocol            // DAP 协议实现
└─test                   // 测试相关
```

* Git 分支规范

| 分支类型       | 分支命名规则     |
| -------------- | ---------------- |
| 主分支         | master           |
| 开发分支       | dev              |
| 功能分支       | feature_xxx      |
| 问题修复分支   | issue(num)_xxx   |

## 开发环境要求

* Windows

| 工具     | 说明                                      |
| ------- | ----------------------------------------- |
| mingw64 | g++ & gdb，版本 8.1.0（与 lsp server 保持一致）|
| cmake   | 版本 3.16 及以上                           |
| ninja   | 版本 1.10.0 及以上                         |
| python  | Python 3.6                                 |
| gcovr   | 代码覆盖率工具                             |

* Linux

| 工具               | 说明                  |
| ------------------ | --------------------- |
| compiler           | g++ 版本 7.5 及以上    |
| cmake              | 版本 3.16 及以上       |
| ninja              | 版本 1.10.0 及以上     |
| python             | Python 3.6            |
| aarch64-linux-gnu  | 交叉编译工具链         |
| gcovr              | 代码覆盖率工具         |

* MacOS

| 工具     | 说明                |
| -------- | ------------------- |
| compiler | clang \| g++        |
| cmake    | 版本 3.16 及以上     |
| ninja    | 版本 1.10.0 及以上   |
| python   | Python 3.6          |
| gcovr    | 代码覆盖率工具       |