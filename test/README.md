# dap_server

* Overview

  Unit tests of a C++ debug adapter using DAP (debug adapter protocol) .

* Project Directories
```
├─build                     // 构建脚本
├─test                      // 测试相关
│  ├─channel                // Message通道相关测试
│  ├─common                 // 通用部分相关测试
│  ├─core                   // 核心实现相关测试
│  ├─debugger               // 调试器适配层相关测试
│  ├─externals              // 配合单元测试使用的工具
│  ├─protocol               // DAP协议实现相关测试
│  └─ut_exclude_filter.txt  // 屏蔽测试用例用文件
```

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

| Tool     | Description               |
| -------- | ------------------------- |
| compiler | g++ version 7.5 and above |
| cmake    | version 3.16 and above    |
| ninja    | 1.10.0 and above          |
| python   | python 3.6                |
| gcovr    | a code coverage tool      |

* MacOS

| Tool     | Description            |
| -------- | ---------------------- |
| compiler | clang \| g++           |
| cmake    | version 3.16 and above |
| ninja    | 1.10.0 and above       |
| python   | python 3.6             |
| gcovr    | a code coverage tool   |

## Commands for running unit test

1. Go to the directory "build"

```tex
cd build
```

2. Execute python script

```
python3 build.py build -u 1
```

## Commands for running smoke test

1. Go to the directory "build"

```tex
cd build
```

2. Execute python script

```
python3 build.py build -u 2
```

Attention: Smoke test is a subset of unit test, which excludes the unstable tests. If you want to enable or disable some tests in the set of smoke test, you should check the contents below.

## How to change the smoke test filter

1. Go to the directory "test"

```tex
cd test
```

2. Change the content of "ut_exclude_filter.txt".

(For example: If you want to exclude the tests that start with "Channel", you can add "Channel*" into the text file.)