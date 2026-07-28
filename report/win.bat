@rem Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
@rem This source file is part of the Cangjie project, licensed under Apache-2.0
@rem with Runtime Library Exception.
@rem
@rem See https://cangjie-lang.cn/pages/LICENSE for license information.
@rem Description: asn script

@echo off
set logs=.\*.log
set testname=gtest_dap_server_test.exe
set dstdir=win_mingw64
if not exist %dstdir% (
    mkdir %dstdir%
)
cd ..\output\bin\win_mingw64
%TestName%
del /q /s /f %logs%
cd ..\..\report
del /q /s /f %dstdir%\*
@echo "generate coverage report ..."
gcovr -r .. -e ../open_source -e ../test --html --html-details -o %dstdir%/index.html
@echo "generate coverage report over."
pause