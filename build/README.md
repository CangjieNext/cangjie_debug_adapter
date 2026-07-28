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



## Build Guide

### 1. Project Tree View

```
.
├── CMakeLists.txt
├── README.md
├── build
│   ├── README.md
│   └── build.py
├── src
...
├── test
```



### 2. Build By Python Script

1. Go to the directory "build"

```tex
cd build
```

2. Execute python script

```
python3 build.py build   -- build project

python3 build.py test    -- perform unit tests (after project built)

python3 build.py clean   -- clean project
```

### 3. Build By CMake

#### windows + mingw

1. Go to the project root directory
2. Execute cmake command below

```tex
cmake . -DCOMPILER_TYPE=MinGW -G "MinGW Makefiles" -B ./build/mingw
```

3. Go to the directory "./build/mingw"，and execute make command below

```tex
make
```

or

```tex
mingw32-make.exe
```

#### linux x86/macos + ninja

1. Go to the project root directory
2. Execute cmake command below

```cmake
cmake . -DCOMPILER_TYPE=Ninja -G "Ninja" -B ./build/ninja
```

3. Go to the directory "./build/ninja", and execute ninja command below

```ninja
ninja all
```

#### linux ARM + ninja

1. Go to the project root directory
2. Execute cmake command below

arm compilation platform with the compilation toolchain named "aarch64-linux-gnu"
```cmake
cmake . -G "Ninja" -DCOMPILER_TYPE=Ninja -DCROSS_NAME=aarch64-linux-gnu -B aarch64
```
linux x86_64 compilation platform with the cross-compilation toolchain named "aarch64-linux-gnu"
```cmake
cmake . -G "Ninja" -DCOMPILER_TYPE=Ninja -DCROSS_NAME=aarch64-linux-gnu -DCROSS_SET=Yes -B aarch64
```

3. Go to the directory "./aarch64", and execute ninja command below

```ninja
ninja all
```

#### output

You should get the targets under the output directory after built successfully.

```
output
└── bin
    ├── dap_server
    └── gtest_dap_server_test
```



