# dap_server Build Guide

Build instructions for `dap_server`. For an overview of the project see the
[root README](../README.md) / [中文](../README_zh.md).

## 1. Directory Layout

```
.
├── CMakeLists.txt
├── README.md
├── README_zh.md
├── build
│   ├── README.md
│   ├── build.py          // dap_server build script
│   ├── prepare.py        // dependency download & build script
│   ├── deps/             // work dir: cloned sources & dependency build trees
│   └── logs/             // build logs
├── open_source           // auto-populated by prepare.py
│   ├── googlemock/
│   ├── googletest/
│   ├── json_modern_c++/
│   ├── libuv/
│   └── lldb/
├── output                // build artifacts
├── report                // coverage reports
├── src
└── test
```

`build/deps/`, `build/logs/`, `build/build/` and `output/` are generated and git-ignored.

## 2. Prerequisites

| Tool     | Requirement                                                       |
| -------- | ----------------------------------------------------------------- |
| compiler | C++17: MinGW 10.x g++ (Windows), g++ 7.5+ (Linux), clang / g++ (macOS)  |
| cmake    | 3.16 and above                                                     |
| ninja    | 1.10.0 and above (or `mingw32-make` on Windows)                     |
| python   | 3.6 and above                                                      |
| git      | required by `prepare.py` to clone dependencies                      |
| gcovr    | optional, needed for `-u 1` coverage reports                        |

On Windows, both `build.py` and `prepare.py` look for `cmake` / `ninja` /
`mingw32-make` / `clang` in `PATH` first, then fall back to common DevEco Studio
locations (`%DEVECO_SDK_HOME%` and `C:\Program Files\Huawei\DevEco Studio\...`),
prepending whatever they find to `PATH` for the duration of the run.

For `win_mingw64`, `build.py` explicitly selects `gcc` / `g++` and uses the
`MinGW Makefiles` generator. Use MinGW 10.x, which is compatible with the Cangjie SDK
runtime. If the compiler changes, remove the corresponding CMake cache before
reconfiguring. Keep the compiler's runtime DLLs together with the executable; mixing
MinGW 14.x DLLs or SDK DLLs from a different runtime can cause missing `nanosleep64`
or `pthread_cond_timedwait64` entry-point errors.

## 3. Prepare Dependencies (first time only)

Before building `dap_server` for the first time, download and build the third-party
dependencies: googletest, googlemock, libuv, nlohmann/json and the LLDB API headers.

```bash
cd build
python3 prepare.py prepare           # download and build all dependencies
python3 prepare.py prepare --force   # redo even if already done
python3 prepare.py clean             # remove build/deps (downloads & build trees)
```

Static libraries are placed per platform under `open_source/<dep>/lib/`:

| Platform / toolchain             | Library directory                  |
| -------------------------------- | ---------------------------------- |
| Windows (MinGW Makefiles, g++)   | `open_source/<dep>/lib/mingw/`     |
| Windows (Ninja, g++)             | `open_source/<dep>/lib/mingw/`     |
| Linux x86_64                     | `open_source/<dep>/lib/gcc/`       |
| Linux aarch64                    | `open_source/<dep>/lib/aarch64/`   |
| macOS (universal x86_64 + arm64) | `open_source/<dep>/lib/darwin/`    |

Header-only artifacts (`json.hpp`, LLDB API headers) are shared across platforms and
downloaded once.

Notes:

- On Windows both generators use MinGW `g++` / `gcc` and share the same `mingw/`
  library directory, so a single `prepare` run covers `win_mingw64` and `win_ninja`.
- `prepare.py` skips a dependency whose static library already exists; use `--force`
  to rebuild it.
- `--sdk-path` is accepted by `prepare.py` for consistency with `build.py`; the
  dependency builds themselves do not require the Cangjie SDK.

## 4. Build dap_server

`dap_server` links `liblldb` from the Cangjie SDK:

| Platform | liblldb path                            |
| -------- | --------------------------------------- |
| Windows  | `<sdk>/tools/bin/liblldb.dll`           |
| Others   | `<sdk>/third_party/llvm/lib/liblldb.so` |

Two Windows targets are supported, both compiled with MinGW `g++`:

- `win_mingw64` : `MinGW Makefiles` generator
- `win_ninja`   : `Ninja` generator

The `win_mingw64` target is the recommended Windows target when using the supported
MinGW 10.x toolchain.

```bash
cd build

python3 build.py build                     # build all default targets for this platform
python3 build.py build -g win_mingw64      # build only the MinGW Makefiles target
python3 build.py build -g win_ninja        # build only the Ninja target
python3 build.py build --sdk-path <path>   # build with an explicit Cangjie SDK path
python3 build.py build -t release          # build type: debug (default) / release / relwithdebinfo
python3 build.py build -u 1                # also run unit tests + coverage report
python3 build.py build -u 2                # also run the smoke test subset
python3 build.py build -c 0                # do not copy artifacts with the target suffix
python3 build.py clean                     # remove output/, build/build/ and build/logs/
```

For the recommended Windows build, use the MinGW 10.x installation first in `PATH`:

```powershell
$env:Path = "D:\program\mingw64\bin;$env:Path"
python build.py build -g win_mingw64
```

If CMake reports a different compiler than the selected MinGW installation, remove
`build/build/win_mingw64/CMakeCache.txt` and its `CMakeFiles/` directory, then configure
again. A stale CMake cache can retain a previously selected compiler.

Available `-g` values per platform:

| Platform | Choices                              | Default                                  |
| -------- | ------------------------------------ | ---------------------------------------- |
| Windows  | `win_mingw64`, `win_ninja`           | both                                     |
| Linux    | `linux_x86_64`, `linux_aarch64`      | `linux_x86_64`                           |
| macOS    | `darwin_x86_64`, `darwin_aarch64`    | matches the host architecture            |

SDK path resolution (required for dap_server build, which depends on liblldb):
set the `CANGJIE_SDK_PATH` environment variable or pass `--sdk-path`.
The SDK provides the liblldb dynamic library at:
- Windows : `<sdk>/tools/bin/liblldb.dll`
- Linux   : `<sdk>/third_party/llvm/lib/liblldb.so`
- macOS   : `<sdk>/third_party/llvm/lib/liblldb.dylib`
The build fails early if `liblldb` is not found at the resolved path.

Unit tests and smoke tests (`-u 1` / `-u 2`) only run for `win_mingw64`,
`linux_x86_64`, `darwin_x86_64` and `darwin_aarch64`; other targets skip them.
`-u 1` also configures cmake with `-DENABLE_COVERAGE=ON` and generates a `gcovr`
report under `report/<target>/`.

## 5. Build By CMake

`build.py` is a wrapper around cmake. To invoke cmake directly, pass
`-DLIBLLDB_PATH=<path>` — the build aborts without it. Output directory names are
derived from `-DWIN_OUTPUT_NAME` on Windows and from the detected system / arch
elsewhere.

### Windows + MinGW Makefiles

```tex
cmake . -DCOMPILER_TYPE=MinGW -DLIBLLDB_PATH=<sdk>/tools/bin/liblldb.dll -DWIN_OUTPUT_NAME=win_mingw64 -G "MinGW Makefiles" -B ./build/win_mingw64
```

Then build in `./build/win_mingw64`:

```tex
mingw32-make.exe
```

### Windows + Ninja

```tex
cmake . -DCOMPILER_TYPE=MinGW -DLIBLLDB_PATH=<sdk>/tools/bin/liblldb.dll -DWIN_OUTPUT_NAME=win_ninja -G "Ninja" -B ./build/win_ninja
```

Then build in `./build/win_ninja`:

```ninja
ninja all
```

### Linux x86_64 / macOS + Ninja

```cmake
cmake . -DCOMPILER_TYPE=Ninja -DLIBLLDB_PATH=<sdk>/third_party/llvm/lib/liblldb.so -G "Ninja" -B ./build/ninja
```

Then build in `./build/ninja`:

```ninja
ninja all
```

### Linux ARM + Ninja

On an aarch64 host with the `aarch64-linux-gnu` toolchain:

```cmake
cmake . -G "Ninja" -DCOMPILER_TYPE=Ninja -DCROSS_NAME=aarch64-linux-gnu -DLIBLLDB_PATH=<sdk>/third_party/llvm/lib/liblldb.so -B aarch64
```

Cross compiling from an x86_64 host, add `-DCROSS_SET=Yes` so cmake picks the
`aarch64-linux-gnu-gcc` / `aarch64-linux-gnu-g++` compilers:

```cmake
cmake . -G "Ninja" -DCOMPILER_TYPE=Ninja -DCROSS_NAME=aarch64-linux-gnu -DCROSS_SET=Yes -DLIBLLDB_PATH=<sdk>/third_party/llvm/lib/liblldb.so -B aarch64
```

Then build in `./aarch64`:

```ninja
ninja all
```

### Useful cmake options

| Option                | Description                                                    |
| --------------------- | -------------------------------------------------------------- |
| `-DLIBLLDB_PATH=`     | Required. Path to `liblldb.dll` / `liblldb.so`.                 |
| `-DCOMPILER_TYPE=`    | `MinGW` or `Ninja`; selects link libraries and lib directories.  |
| `-DWIN_OUTPUT_NAME=`  | Windows output subdirectory name. Default `win_mingw64`.         |
| `-DCROSS_NAME=`       | Cross toolchain prefix, e.g. `aarch64-linux-gnu`.                |
| `-DCROSS_SET=Yes`     | Use the cross compilers named by `CROSS_NAME`.                   |
| `-DENABLE_COVERAGE=ON`| Build with `-fprofile-arcs -ftest-coverage` for gcovr.           |
| `-DCMAKE_BUILD_TYPE=` | `Debug` (default), `Release` or `RelWithDebInfo`.                |

## 6. Output

After a successful build the targets are placed under `output/`:

```
output
├── bin
│   └── <target>                        // win_mingw64 | win_ninja | linux_x86_64 |
│       ├── dap_server                  // linux_aarch64 | darwin_x86_64 | darwin_aarch64
│       ├── dap_server_<target>         // copy with target suffix, unless -c 0
│       ├── gtest_dap_server_test       // unit test executable
│       └── Testgdb                     // test helper debuggee
└── lib
    └── <target>
        └── libdap_server_lib.a
```
