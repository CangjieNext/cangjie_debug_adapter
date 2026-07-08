# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.16)
project(dap_server)

# c++17
add_compile_options(-std=c++17)

# set open_source directory
set(OPENSRC_DIR ${PROJECT_SOURCE_DIR}/../open_source/)
message(STATUS "Directory of opensource: " ${OPENSRC_DIR})

# set open_source include directories
set(JSON_INCLUDE_DIR ${OPENSRC_DIR}/json_modern_c++/include/)
set(LIBUV_INCLUDE_DIR ${OPENSRC_DIR}/libuv/include/)
set(LLDB_API_INCLUDE_ROOT_DIR ${OPENSRC_DIR}/lldb/include/)
set(LLDB_API_INCLUDE_DIR ${OPENSRC_DIR}/lldb/include/lldb/API)

# set project include directories
set(SRC_INCLUDE_DIR ./)
set(COMMON_INCLUDE_DIR ./common/)
set(CORE_INCLUDE_DIR ./core/)
set(PLATFORM_INCLUDE_DIR ./platform/)
set(PLATFORM_WINDOWS_INCLUDE_DIR ./platform/windows/)
set(PLATFORM_LINUX_INCLUDE_DIR ./platform/linux/)
set(CHANNEL_INCLUDE_DIR ./channel/)
set(DEBUGGER_INCLUDE_DIR ./debugger/)
set(DEBUGGER_LLDB_API_INCLUDE_DIR ./debugger/lldbapi)
set(DEBUGGER_LLDB_API_HANDLER_INCLUDE_DIR ./debugger/lldbapi/handler)
set(PROTOCOL_INCLUDE_DIR ./protocol/)

# set directories of open_source include & library files
message(STATUS "${PROJECT_NAME} compiler type=${COMPILER_TYPE}, cross_name=${CROSS_NAME}, cross_set=${CROSS_SET}.")
if  (COMPILER_TYPE MATCHES "MinGW" OR CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/mingw/)
    add_definitions(-D_WIN32) # define _WIN32
elseif (COMPILER_TYPE MATCHES "Ninja")
    if (CMAKE_SYSTEM_NAME MATCHES "Linux") 
        if (CROSS_NAME MATCHES "aarch64-linux-gnu" OR LINUX_ARCH MATCHES "aarch64-linux-gnu")
            if (CROSS_SET MATCHES "Yes")
                set(CMAKE_SYSTEM_PROCESSOR arm)
                set(CMAKE_C_COMPILER ${CROSS_NAME}-gcc)
                set(CMAKE_CXX_COMPILER ${CROSS_NAME}-g++)
            endif ()
            set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/aarch64/)
        else() #default
            set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/gcc/)
        endif()
    elseif (CMAKE_SYSTEM_NAME MATCHES "OHOS")
        set(LIBUV_LIB_DIR ${OHOS_NATIVE_HOME}/sysroot/usr/lib/aarch64-linux-ohos/)
    else() #darwin
        set(LIBUV_LIB_DIR ${OPENSRC_DIR}/libuv/lib/darwin/)
    endif()
else()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MinGW/Ninja .] command to set the compiler type.")
endif()

# include open_source header files
include_directories(SYSTEM ${JSON_INCLUDE_DIR})
include_directories(SYSTEM ${LIBUV_INCLUDE_DIR})
include_directories(SYSTEM ${LLDB_API_INCLUDE_ROOT_DIR})
include_directories(SYSTEM ${LLDB_API_INCLUDE_DIR})

# include project header files
include_directories(${SRC_INCLUDE_DIR})
include_directories(${COMMON_INCLUDE_DIR})
include_directories(${CORE_INCLUDE_DIR})
include_directories(${PLATFORM_INCLUDE_DIR})
include_directories(${PLATFORM_LINUX_INCLUDE_DIR})
include_directories(${PLATFORM_WINDOWS_INCLUDE_DIR})
include_directories(${CHANNEL_INCLUDE_DIR})
include_directories(${DEBUGGER_INCLUDE_DIR})
include_directories(${DEBUGGER_LLDB_API_INCLUDE_DIR})
include_directories(${DEBUGGER_LLDB_API_HANDLER_INCLUDE_DIR})
include_directories(${PROTOCOL_INCLUDE_DIR})

# add directories of open_source libraries
link_directories(${LIBUV_LIB_DIR})

# add directories of project source files
aux_source_directory(./ MAIN)
aux_source_directory(./common/ COMMON)
aux_source_directory(./core/ CORE)
aux_source_directory(./channel/ CHANNEL)
aux_source_directory(./platform/windows/ WINDOWSPLATFORM)
aux_source_directory(./platform/linux/ LINUXPLATFORM)
aux_source_directory(./protocol/ PROTOCOL)
aux_source_directory(./debugger DEBUGGER)
aux_source_directory(./debugger/lldbapi LLDB_API)
aux_source_directory(./debugger/lldbapi/handler LLDB_API_HANDLER)

# generate library
set(PROJECT_LIB_NAME ${PROJECT_NAME}_lib)
add_definitions(-D_LLDBAPI)
add_library(${PROJECT_LIB_NAME} ${CORE} ${CHANNEL} ${LINUXPLATFORM} ${WINDOWSPLATFORM} ${PROTOCOL} ${DEBUGGER}
        ${COMMON} ${LLDB_API} ${LLDB_API_HANDLER})
if (COMPILER_TYPE MATCHES "MinGW")
    target_link_libraries(${PROJECT_LIB_NAME}
            uv
            dbghelp
            UserEnv
            ws2_32
            Iphlpapi
            Shlwapi)
elseif (CROSS_WINDOWS MATCHES "ON" AND CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-DCROSS_WINDOWS_FLAG)
    target_link_libraries(${PROJECT_LIB_NAME}
            uv
            dbghelp
            userenv
            ws2_32
            iphlpapi
            shlwapi)
elseif (COMPILER_TYPE MATCHES "Ninja")
    # Compatible with x86 and ARM
    target_link_libraries(${PROJECT_LIB_NAME}
            uv
            dl
            pthread)
else()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MinGW/Ninja .] command to set the compiler type.")
endif()

# generate executable binary file
set(PROJECT_EXE_NAME ${PROJECT_NAME})
add_executable(${PROJECT_EXE_NAME} ${MAIN})
target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_LIB_NAME})
if (NOT LIBLLDB_PATH)
    message(FATAL_ERROR "input param LIBLLDB_PATH not defined!")
elseif(NOT EXISTS ${LIBLLDB_PATH})
    message(FATAL_ERROR "LIBLLDB_PATH error, liblldb doesn't exist!")
endif()
message(STATUS "Location of liblldb: " ${LIBLLDB_PATH})
target_link_libraries(${PROJECT_EXE_NAME} ${LIBLLDB_PATH})

# enable dynamic_base compile option
if (COMPILER_TYPE MATCHES "MinGW")
    target_link_libraries(${PROJECT_NAME} -lssp)
endif()

# target include directories
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${JSON_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LIBUV_INCLUDE_DIR})