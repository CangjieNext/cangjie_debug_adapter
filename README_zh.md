# cangjie_debug_adapter

[English](README.md)

## 概述

`dap_server` 是一个基于 [DAP（Debug Adapter Protocol）](https://microsoft.github.io/debug-adapter-protocol/)
协议实现的 C++ 调试适配器。它位于 DAP 客户端（IDE / 编辑器）与 LLDB 调试器之间，用于调试
仓颉和 C/C++ 程序。

适配器通过 TCP socket 与客户端通信，通过仓颉 SDK 提供的 `liblldb` 的 LLDB C++ API
（`SBDebugger`、`SBTarget`、`SBProcess` 等）驱动调试器。除标准 DAP 请求外，还支持一组
仓颉扩展请求，例如时光调试（`undo`、`reverseContinue`、`stepBack`、`setExecutionPoint`、
`gotoRecordPoint`）、源码路径映射（`addSourceMapping`、`resetSourceMapping`）、变量十六进制
查看（`variableHexView`）以及 LLDB 命令补全（`commandCompletion`）。

## 功能特性

- 基于 LLDB API 的 launch / attach 两种调试方式
- 源码断点、函数断点、条件断点、命中次数断点、日志断点、异常断点、数据断点、指令断点
- 按粒度单步（语句 / 行 / 指令）、暂停、继续
- 调用栈、作用域、变量、`setVariable`、表达式求值
- 反汇编、读写内存
- 时光调试（录制点、反向执行、undo）
- 仓颉与 C++ 变量值格式化、源码路径映射

## 项目目录结构

```
├─build                      // 构建与依赖脚本（build.py、prepare.py）
├─open_source                // 第三方源码与静态库，由 prepare.py 生成
├─report                     // 覆盖率报告输出目录
├─src
│  ├─channel                 // 基于 libuv 的消息通道（socket、线程、容器）
│  ├─common                  // 日志、参数解析、进程监控、dump、通用工具
│  ├─core                    // socket 服务、调试会话、消息缓冲、启动选项
│  ├─debugger                // 调试器抽象与公共辅助逻辑
│  │  ├─lldbapi              // LLDB API 调试器实现与事件处理
│  │  │  └─handler           // 各请求的处理器（launch、断点、变量等）
│  │  └─...                  // launch/attach 辅助、断点表、时光调试管理
│  ├─platform                // 平台适配层
│  │  ├─linux                // Linux 与 macOS
│  │  └─windows
│  └─protocol                // DAP 协议类型与请求/响应/事件管理
└─test                       // 基于 gtest 的单元测试，目录结构与 src 对应
   └─externals               // 单元测试中作为被调试程序使用的辅助程序
```

## 运行时依赖

`dap_server` 链接仓颉 SDK 中的 `liblldb`：

| 平台     | liblldb 路径                              |
| -------- | ----------------------------------------- |
| Windows  | `<sdk>/tools/bin/liblldb.dll`             |
| 其他平台 | `<sdk>/third_party/llvm/lib/liblldb.so`   |

第三方库由 `build/prepare.py` 下载并构建到 `open_source/` 目录：nlohmann/json（仅头文件）、
libuv、googletest / googlemock，以及 LLDB API 头文件。

Windows 下，MinGW 编译器与运行时 DLL 必须和仓颉 SDK 保持 ABI 兼容。`win_mingw64`
请使用受支持的 MinGW 10.x 工具链，并将对应的 `libwinpthread-1.dll`、
`libgcc_s_seh-1.dll`、`libstdc++-6.dll` 放在同一套运行环境中。不要混用其他 MinGW
版本的运行时 DLL，也不要让 `PATH` 优先命中其他 MinGW 安装目录。

## 启动参数

`dap_server` 由 DAP 客户端拉起，监听指定 TCP 端口：

```
dap_server --port=<port> [--logpath=<dir>] [--loglevel=<level>] [--logsize=<MB>] [--logEnable=<0|1>]
```

| 参数           | 说明                                                          |
| -------------- | ------------------------------------------------------------- |
| `--port=`      | 必填。监听端口，取值 `0`-`65535`。会话结束后进程退出。          |
| `--logpath=`   | 日志目录，不存在时自动创建。默认为当前目录。                    |
| `--loglevel=`  | `DEBUG` / `INFO` / `WARN` / `ERROR` / `FATAL`，默认 `INFO`。   |
| `--logsize=`   | 单个日志文件最大大小，单位 MB，默认 `32`。                      |
| `--logEnable=` | `1` 写日志文件，`0` 关闭，默认 `1`。                            |

无法识别的参数会打印告警并忽略。通信报文采用标准 DAP 帧格式
（`Content-Length: <n>\r\n\r\n<json>`）。

说明：日志目录会自动创建，但要求路径的第一级目录已经存在。例如
`--logpath=D:/logs/dap` 和 `--logpath=existing_dir/sub` 可以使用；如果直接指定
不存在的 `new_dir`，创建可能失败。建议使用绝对路径。目录不可用时服务会报错退出。

## 开发环境要求

| 工具     | 要求                                                            |
| -------- | --------------------------------------------------------------- |
| 编译器   | 支持 C++17：MinGW 10.x g++（Windows）、g++ 7.5 及以上（Linux）、clang / g++（macOS） |
| cmake    | 3.16 及以上                                                      |
| ninja    | 1.10.0 及以上（Windows 也可使用 `mingw32-make`）                  |
| python   | 3.6 及以上                                                       |
| git      | `prepare.py` 拉取依赖时需要                                       |
| gcovr    | 可选，用于生成代码覆盖率报告                                       |
| aarch64-linux-gnu | 可选，Linux ARM 交叉编译工具链                           |

## 构建

完整构建指导见 [build/README.md](build/README.md)，快速开始：

```bash
cd build
python3 prepare.py prepare                 # 首次构建：下载并编译依赖
python3 build.py build -g win_mingw64      # Windows：使用 MinGW 10.x 构建
python3 build.py build -u 1                # 构建、执行单元测试并生成覆盖率报告
```

构建产物输出到 `output/bin/<target>/`，`<target>` 取值为 `win_mingw64`、`win_ninja`、
`linux_x86_64`、`linux_aarch64`、`darwin_x86_64` 或 `darwin_aarch64`。

Windows 下启动服务时，应先加载仓颉 SDK 的环境配置，或者将与编译器匹配的运行时
DLL 放在 `dap_server.exe` 同目录。若双击启动时加载了其他 MinGW 版本的 DLL，可能出现
找不到 `nanosleep64` 等运行时输入点的错误。

可用以下命令检查产物依赖的 DLL 是否来自同一套 MinGW：

```powershell
where g++
where libwinpthread-1.dll
```

## 测试

单元测试位于 `test/` 目录，构建为 `gtest_dap_server_test`。单元测试与 smoke 测试的
执行方式见 [test/README.md](test/README.md)。

## Git 分支规范

| 分支类型     | 分支命名规则     |
| ------------ | ---------------- |
| 主分支       | main             |
| 开发分支     | dev              |
| 功能分支     | feature_xxx      |
| 问题修复分支 | issue(num)_xxx   |

## 许可

Apache-2.0 with Runtime Library Exception，详见 [LICENSE](LICENSE)。
