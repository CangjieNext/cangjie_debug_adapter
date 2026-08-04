# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required (VERSION 3.16)

project(dap_server)

set(CMAKE_CXX_STANDARD 17)

# set default build type to debug
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

# For Windows
if(COMPILER_TYPE MATCHES "MinGW")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong")
    add_link_options(-Wl,--dynamicbase)
    add_link_options(-Wl,--nxcompat)
endif()

# For Linux
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_SKIP_RPATH TRUE)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_SKIP_INSTALL_RPATH TRUE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ftrapv")
    add_link_options(-Wl,-z,noexecstack)
    add_link_options(-s)
    add_link_options(-Wl,-z,relro,-z,now)
    set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS    "${LINK_FLAGS}")
    if (NOT LINUX_ARCH MATCHES "aarch64-linux-gnu")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIE -pie")
    endif ()
endif()

# Code coverage is disabled by default.
if(ENABLE_COVERAGE)
    message(STATUS "Use lcov for code coverage test")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if(COMPILER_TYPE MATCHES "MinGW" OR CMAKE_SYSTEM_NAME MATCHES "Windows") # window
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WINDOWS")
    if (NOT WIN_OUTPUT_NAME)
        set(WIN_OUTPUT_NAME "win_mingw64")
    endif()
    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/${WIN_OUTPUT_NAME})
    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/${WIN_OUTPUT_NAME})
else() # linux / macos ninja
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")  
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
        if (CROSS_NAME MATCHES "aarch64-linux-gnu" OR LINUX_ARCH MATCHES "aarch64-linux-gnu") # linux aarch64
            set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/linux_aarch64)
            set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/linux_aarch64)
        else() # linux x86_64
            set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/linux_x86_64)
            set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/linux_x86_64)
        endif()
    elseif (CMAKE_SYSTEM_NAME MATCHES "OHOS")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
        set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/ohos)
        set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/ohos)
    else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DARWIN")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
            set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/darwin_aarch64)
            set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/darwin_aarch64)
        else()
            set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin/darwin_x86_64)
            set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib/darwin_x86_64)
        endif()
    endif()
endif()

message(STATUS "OS name: " ${CMAKE_SYSTEM_NAME})
message(STATUS "Directory of executable: " ${EXECUTABLE_OUTPUT_PATH})
message(STATUS "Directory of library: " ${LIBRARY_OUTPUT_PATH})

# debug adapter server
add_subdirectory(src)
# external dependencies
add_subdirectory(test/externals)
# gtest for unit test
enable_testing()
add_subdirectory(test)
set(strict_targets "dap_server" "dap_server_lib" "Testgdb" "gtest_dap_server_test")

foreach(target ${strict_targets})
    if(TARGET ${target})
        target_compile_options(${target} PRIVATE
                "$<$<COMPILE_LANGUAGE:C>:-fno-common>"
                "$<$<COMPILE_LANGUAGE:C>:-Wall>"
                "$<$<COMPILE_LANGUAGE:C>:-Wextra>"
                "$<$<COMPILE_LANGUAGE:C>:-Wfloat-equal>"
        )
        target_compile_options(${target} PRIVATE
                "$<$<COMPILE_LANGUAGE:CXX>:-fno-common>"
                "$<$<COMPILE_LANGUAGE:CXX>:-Wall>"
                "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>"
                "$<$<COMPILE_LANGUAGE:CXX>:-Wfloat-equal>"
        )
    endif()
endforeach()
