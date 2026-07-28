# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.16)
project(gtest_dap_server_test)

# open_source directory
set(OPENSRC_DIR ${PROJECT_SOURCE_DIR}/../open_source)
message("Directory of opensource: " ${OPENSRC_DIR})

# open_source include directories
set(GTEST_INCLUDE_DIR ${OPENSRC_DIR}/googletest/include)
set(GMOCK_INCLUDE_DIR ${OPENSRC_DIR}/googlemock/include)
set(JSON_INCLUDE_DIR ${OPENSRC_DIR}/json_modern_c++/include)
set(LIBUV_INCLUDE_DIR ${OPENSRC_DIR}/libuv/include/)
set(LLDB_API_INCLUDE_ROOT_DIR ${OPENSRC_DIR}/lldb/include/)
set(LLDB_API_INCLUDE_DIR ${OPENSRC_DIR}/lldb/include/lldb/API)

# dap_server directories
set(DAP_SERVER_SRC_DIR ${PROJECT_SOURCE_DIR}/../src)
set(DAP_SERVER_COMMON_DIR ${PROJECT_SOURCE_DIR}/../src/common)
set(DAP_SERVER_PROTOCOL_DIR ${PROJECT_SOURCE_DIR}/../src/protocol)
set(CORE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/core/)
set(PLATFORM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/platform/)
set(PLATFORM_WINDOWS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/platform/windows/)
set(PLATFORM_LINUX_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/platform/linux/)
set(CHANNEL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/channel/)
set(DEBUGGER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/debugger/)
set(DEBUGGER_LLDB_API_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/debugger/lldbapi)
set(DEBUGGER_LLDB_API_HANDLER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../src/debugger/lldbapi/handler)

# open_source lib directories
message(STATUS "${PROJECT_NAME} compiler type = ${COMPILER_TYPE}, cross_name = ${CROSS_NAME}.")
if (COMPILER_TYPE MATCHES "MinGW")
    set(GTEST_LIB_DIR ${OPENSRC_DIR}/googletest/lib/mingw)
    set(GMOCK_LIB_DIR ${OPENSRC_DIR}/googlemock/lib/mingw)
    set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/mingw/)
    add_definitions(-D_WIN32) # define _WIN32
elseif (COMPILER_TYPE MATCHES "Ninja")
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        if (CROSS_NAME MATCHES "aarch64-linux-gnu")
            if (CROSS_SET MATCHES "Yes")
                set(CMAKE_SYSTEM_PROCESSOR arm)
                set(CMAKE_C_COMPILER ${CROSS_NAME}-gcc)
                set(CMAKE_CXX_COMPILER ${CROSS_NAME}-g++)
            endif ()
            set(GTEST_LIB_DIR ${OPENSRC_DIR}/googletest/lib/aarch64)
            set(GMOCK_LIB_DIR ${OPENSRC_DIR}/googlemock/lib/aarch64)
            set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/aarch64)
        else ()
            set(GTEST_LIB_DIR ${OPENSRC_DIR}/googletest/lib/gcc)
            set(GMOCK_LIB_DIR ${OPENSRC_DIR}/googlemock/lib/gcc)
            set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/gcc/)
        endif ()
    else()
        set(GTEST_LIB_DIR ${OPENSRC_DIR}/googletest/lib/darwin)
        set(GMOCK_LIB_DIR ${OPENSRC_DIR}/googlemock/lib/darwin)
        set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/darwin/)
    endif()
else()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MinGW/Ninja .] command to set the compiler type.")
endif ()

# include open_sources header files' directories
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
include_directories(SYSTEM ${GMOCK_INCLUDE_DIR})
include_directories(SYSTEM ${JSON_INCLUDE_DIR})
include_directories(SYSTEM ${LIBUV_INCLUDE_DIR})
include_directories(SYSTEM ${LLDB_API_INCLUDE_ROOT_DIR})
include_directories(SYSTEM ${LLDB_API_INCLUDE_DIR})

# include dap_server header files' directories
include_directories(${DAP_SERVER_SRC_DIR})
include_directories(${DAP_SERVER_COMMON_DIR})
include_directories(${DAP_SERVER_PROTOCOL_DIR})
include_directories(${CORE_INCLUDE_DIR})
include_directories(${PLATFORM_INCLUDE_DIR})
include_directories(${PLATFORM_LINUX_INCLUDE_DIR})
include_directories(${PLATFORM_WINDOWS_INCLUDE_DIR})
include_directories(${CHANNEL_INCLUDE_DIR})
include_directories(${CHANNEL_INCLUDE_DIR}/extend)
include_directories(${DEBUGGER_INCLUDE_DIR})
include_directories(${DEBUGGER_LLDB_API_INCLUDE_DIR})
include_directories(${DEBUGGER_LLDB_API_HANDLER_INCLUDE_DIR})

# add directories of open_source libraries
link_directories(${GTEST_LIB_DIR})
link_directories(${GMOCK_LIB_DIR})
link_directories(${LIBUV_LIB_DIR})

# add directories of dap_server source files
aux_source_directory(${DAP_SERVER_COMMON_DIR} DAP_SRC_COMMON)
aux_source_directory(${DAP_SERVER_PROTOCOL_DIR} DAP_SRC_PROTOCOL)
aux_source_directory(${CORE_INCLUDE_DIR} DAP_SRC_CORE)
aux_source_directory(${PLATFORM_INCLUDE_DIR} DAP_SRC_PLATFORM)
aux_source_directory(${PLATFORM_LINUX_INCLUDE_DIR} DAP_SRC_LINUX)
aux_source_directory(${PLATFORM_WINDOWS_INCLUDE_DIR} DAP_SRC_WINDOWS)
aux_source_directory(${CHANNEL_INCLUDE_DIR} DAP_SRC_CHANNEL)
aux_source_directory(${DEBUGGER_INCLUDE_DIR} DAP_SRC_DEBUGGER)
aux_source_directory(${DEBUGGER_LLDB_API_INCLUDE_DIR} DAP_SRC_LLDB_API)
aux_source_directory(${DEBUGGER_LLDB_API_HANDLER_INCLUDE_DIR} DAP_SRC_LLDB_API_HANDLER)

# add directories of project source files
aux_source_directory(./ TEST_SRC_MAIN)
aux_source_directory(./protocol TEST_SRC_PROTOCOL)
aux_source_directory(./debugger TEST_SRC_DEBUGGER)
aux_source_directory(./common TEST_SRC_COMMON)
aux_source_directory(./channel TEST_SRC_CHANNEL)
aux_source_directory(./core TEST_SRC_CORE)
add_executable(
        ${PROJECT_NAME}
        # Test source
        ${TEST_SRC_MAIN}
        ${TEST_SRC_PROTOCOL}
        ${TEST_SRC_DEBUGGER}
        ${TEST_SRC_COMMON}
        ${TEST_SRC_CHANNEL}
        ${TEST_SRC_CORE}
        # Dap source
        ${DAP_SRC_COMMON}
        ${DAP_SRC_PROTOCOL} ${DAP_SRC_CORE} ${DAP_SRC_PLATFORM} ${DAP_SRC_LINUX} ${DAP_SRC_WINDOWS}
        ${DAP_SRC_CHANNEL} ${DAP_SRC_DEBUGGER} ${DAP_SRC_LLDB_API} ${DAP_SRC_LLDB_API_HANDLER})
if (COMPILER_TYPE MATCHES "MinGW")
    target_link_libraries(${PROJECT_NAME}
            gtest
            pthread
            gmock
            uv
            dbghelp
            UserEnv
            ws2_32
            Iphlpapi
            Shlwapi)
elseif (COMPILER_TYPE MATCHES "Ninja")
    # Compatible with x86 and ARM
    target_link_libraries(${PROJECT_NAME}
            gtest
            gmock
            pthread
            uv
            dl
            pthread)
else()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MinGW/Ninja .] command to set the compiler type.")
endif()
set(PROJECT_EXE_NAME ${PROJECT_NAME})
target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_LIB_NAME})
if (NOT LIBLLDB_PATH)
    message(FATAL_ERROR "input param LIBLLDB_PATH not defined!")
elseif(NOT EXISTS ${LIBLLDB_PATH})
    message(FATAL_ERROR "LIBLLDB_PATH error, liblldb doesn't exist!")
endif()
message(STATUS "Location of liblldb: " ${LIBLLDB_PATH})
target_link_libraries(${PROJECT_EXE_NAME} ${LIBLLDB_PATH})
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})