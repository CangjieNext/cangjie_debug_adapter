# cangjie_debug_adapter

[中文](README_zh.md)

## Overview

`dap_server` is a C++ debug adapter that implements the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/)
(DAP). It sits between a DAP client (IDE / editor) and the LLDB debugger, and is used to
debug Cangjie and C/C++ programs.

The adapter talks to the client over a TCP socket, and drives the debugger through the
LLDB C++ API (`SBDebugger`, `SBTarget`, `SBProcess`, ...) provided by `liblldb` from the
Cangjie SDK. Besides the standard DAP requests, it also supports a set of Cangjie-specific
extensions such as time travel debugging (`undo`, `reverseContinue`, `stepBack`,
`setExecutionPoint`, `gotoRecordPoint`), source mapping (`addSourceMapping`,
`resetSourceMapping`), hex view of variables (`variableHexView`) and LLDB command
completion (`commandCompletion`).

## Features

- Launch and attach debugging, driven by the LLDB API
- Source, function, conditional, hit-condition, log, exception, data and instruction breakpoints
- Stepping with granularity (statement / line / instruction), pause, continue
- Stack traces, scopes, variables, `setVariable`, expression evaluation
- Disassembly, read / write memory
- Time travel debugging (record points, reverse execution, undo)
- Cangjie and C++ value formatting, source path mapping

## Project Structure

```
├─build                      // build & dependency scripts (build.py, prepare.py)
├─open_source                // third-party sources & static libs, populated by prepare.py
├─report                     // coverage report output
├─src
│  ├─channel                 // libuv based message channels (socket, thread, container)
│  ├─common                  // logging, param parsing, process monitor, dump, utils
│  ├─core                    // socket server, debug session, message buffer, options
│  ├─debugger                // debugger abstraction & shared helpers
│  │  ├─lldbapi              // LLDB API debugger implementation & event handler
│  │  │  └─handler           // per-request handlers (launch, breakpoints, variables, ...)
│  │  └─...                  // launch/attach helpers, breakpoint maps, time travel manager
│  ├─platform                // platform adaptation layer
│  │  ├─linux                // Linux & macOS
│  │  └─windows
│  └─protocol                // DAP protocol types, request/response/event managers
└─test                       // gtest based unit tests, mirrors the src layout
   └─externals               // helper programs used as debuggees in tests
```

## Runtime Dependencies

`dap_server` links against `liblldb` from the Cangjie SDK:

| Platform | liblldb location            |
| -------- | --------------------------- |
| Windows  | `<sdk>/tools/bin/liblldb.dll` |
| Others   | `<sdk>/third_party/llvm/lib/liblldb.so` |

Third-party libraries are fetched and built into `open_source/` by `build/prepare.py`:
nlohmann/json (header only), libuv, googletest / googlemock, and the LLDB API headers.

On Windows, the MinGW compiler and runtime DLLs must be ABI-compatible with the
Cangjie SDK. Use the supported MinGW 10.x toolchain for `win_mingw64` and keep its
`libwinpthread-1.dll`, `libgcc_s_seh-1.dll` and `libstdc++-6.dll` together. Do not mix
runtime DLLs from another MinGW installation or rely on a different MinGW version in
`PATH`.

## Usage

`dap_server` is started by the DAP client and listens on a TCP port:

```
dap_server --port=<port> [--logpath=<dir>] [--loglevel=<level>] [--logsize=<MB>] [--logEnable=<0|1>]
```

| Option        | Description                                                       |
| ------------- | ----------------------------------------------------------------- |
| `--port=`     | Required. Listening port, `0`-`65535`. The server exits when the session ends. |
| `--logpath=`  | Log directory. Default: current directory. An absolute path is recommended; see the note below. |
| `--loglevel=` | `DEBUG` / `INFO` / `WARN` / `ERROR` / `FATAL`. Default: `INFO`.     |
| `--logsize=`  | Max size of one log file in MB. Default: `32`.                      |
| `--logEnable=`| `1` to write log files, `0` to disable. Default: `1`.               |

Unknown options are ignored with a warning. Messages on the wire use the standard DAP
framing (`Content-Length: <n>\r\n\r\n<json>`).

Note on `--logpath`: missing directories are created, but only if the first component of
the path already exists. `--logpath=D:/logs/dap` and `--logpath=existing_dir/sub` work,
while `--logpath=new_dir` fails when `new_dir` does not exist yet. Passing an absolute
path avoids the issue. The server exits with an error if the directory cannot be used.

## Development Environment

| Tool     | Requirement                                                     |
| -------- | --------------------------------------------------------------- |
| compiler | C++17: MinGW 10.x g++ (Windows), g++ 7.5+ (Linux), clang / g++ (macOS) |
| cmake    | 3.16 and above                                                   |
| ninja    | 1.10.0 and above (or `mingw32-make` on Windows)                   |
| python   | 3.6 and above                                                    |
| git      | required by `prepare.py` to fetch dependencies                    |
| gcovr    | optional, code coverage report                                   |
| aarch64-linux-gnu | optional, Linux ARM cross compilation toolchain         |

## Build

See [build/README.md](build/README.md) for the full build guide. Quick start:

```bash
cd build
python3 prepare.py prepare                 # first time only: fetch & build dependencies
python3 build.py build -g win_mingw64      # Windows: build with MinGW 10.x
python3 build.py build -u 1                # build, run unit tests and generate coverage
```

Build targets are written to `output/bin/<target>/`, where `<target>` is
`win_mingw64`, `win_ninja`, `linux_x86_64`, `linux_aarch64`, `darwin_x86_64` or
`darwin_aarch64`.

For Windows, run the server from a shell that has the Cangjie SDK environment loaded,
or place the matching runtime DLLs beside `dap_server.exe`. Double-clicking a binary
with DLLs from another MinGW installation can fail with missing `nanosleep64` or other
runtime entry points.

## Tests

Unit tests live in `test/` and are built as `gtest_dap_server_test`. See
[test/README.md](test/README.md) for how to run the unit test and smoke test sets.

## Git Branch Convention

| Branch Type    | Branch Name Rule |
| -------------- | ---------------- |
| main branch    | main             |
| develop branch | dev              |
| feature branch | feature_xxx      |
| issue branch   | issue(num)_xxx   |

## License

Apache-2.0 with Runtime Library Exception. See [LICENSE](LICENSE) for details.
