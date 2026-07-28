#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

"""DAP Server build entry"""
import argparse
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
import stat
from enum import Enum
from shutil import copyfile
from logging.handlers import TimedRotatingFileHandler

IS_WINDOWS = platform.system() == 'Windows'
IS_LINUX = platform.system() == 'Linux'
IS_DARWIN = platform.system() == 'Darwin'
MAKE_JOBS = multiprocessing.cpu_count() + 2

BUILD_DIR = os.path.dirname(os.path.realpath(__file__))
CMAKE_BUILD_DIR = os.path.join(BUILD_DIR, 'build')

HOME_DIR = os.path.dirname(BUILD_DIR)
OPEN_SRC_DIR = os.path.join(HOME_DIR, 'open_source')
SRC_DIR = os.path.join(HOME_DIR, 'src')
TEST_DIR = os.path.join(HOME_DIR, 'test')
SRC_LAUNCHER_DIR = os.path.join(SRC_DIR, 'launcher')
OUTPUT_DIR = os.path.join(HOME_DIR, 'output')
OUTPUT_BIN_DIR = os.path.join(OUTPUT_DIR, 'bin')
OUTPUT_EXE_DIR = ''
UT_REPORT_DIR = os.path.join(HOME_DIR, 'report')
UT_EXE_NAME = 'gtest_dap_server_test.exe' if IS_WINDOWS else 'gtest_dap_server_test'
UT_EXE_PATH = ''

LOG_DIR = os.path.join(BUILD_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'dap_server.log')

def information():
    LOG.info('os platform = {}'.format(platform.system()))
    LOG.info('root directory = {}'.format(HOME_DIR))
    LOG.info('open_source directory = {}'.format(OPEN_SRC_DIR))
    LOG.info('src directory = {}'.format(SRC_DIR))
    LOG.info('src launcher directory = {}'.format(SRC_LAUNCHER_DIR))
    LOG.info('test directory = {}'.format(TEST_DIR))
    LOG.info('output directory = {}'.format(OUTPUT_DIR))
    LOG.info('coverage report directory = {}'.format(UT_REPORT_DIR))
    LOG.info('build directory = {}'.format(BUILD_DIR))
    LOG.info('log directory = {}'.format(LOG_DIR))

def log_output(output):
    """log command output"""
    while True:
        line = output.stdout.readline()
        if not line:
            ret = output.poll()
            if ret and ret != 0:
                LOG.error('build error: %d!\n', ret)
                sys.exit(1)
            break
        LOG.info(line.decode('UTF-8').rstrip())

def build(args):
    """dap server build"""
    LOG.info('[DAP SERVER] begin build...\n')
    generator = 'MinGW Makefiles' if IS_WINDOWS else 'Ninja'
    compiler_type = 'MinGW' if IS_WINDOWS else 'Ninja'
    LOG.info('[DAP SERVER] gxx_types = {}\n'.format(args.gxx_types))
    for i, gxx_type in enumerate(args.gxx_types):
        LOG.info('[DAP SERVER] gxx_type = {}\n'.format(gxx_type))
        OUTPUT_EXE_DIR = os.path.join(OUTPUT_BIN_DIR, gxx_type)
        UT_EXE_PATH = os.path.join(OUTPUT_EXE_DIR, UT_EXE_NAME)
        LOG.info('[DAP SERVER] unit test executable file path = {}'.format(UT_EXE_PATH))
        build_dir = os.path.join(CMAKE_BUILD_DIR, gxx_type)
        if not os.path.exists(build_dir):
            os.makedirs(build_dir) 
        build_cmds = ['cmake', HOME_DIR, '-G', generator, '-DCMAKE_BUILD_TYPE={}'.format(args.build_type.value), '-DCOMPILER_TYPE={}'.format(compiler_type)]
        if args.unit_test_enable == 1:
            build_cmds.append('-DENABLE_COVERAGE=ON')
        # cross toolchain does not need to be set on windows or linux x86_64
        # aarch64-linux-gnu
        if gxx_type == 'linux_aarch64':
            build_cmds.append('-DCROSS_NAME=aarch64-linux-gnu')
            build_cmds.append('-DCROSS_SET=Yes')
        output = subprocess.Popen(build_cmds, cwd = build_dir, stdout = subprocess.PIPE)
        log_output(output)

        build_cmds = ['cmake', '--build', '.', '-j', str(MAKE_JOBS)]
        output = subprocess.Popen(build_cmds, cwd = build_dir, stdout = subprocess.PIPE)
        log_output(output)
        LOG.info('[DAP SERVER] end build.\n')

        # copy dap_server and launcher with OS suffix
        if args.copy_target_enable == 1:
            copy_to_target(OUTPUT_EXE_DIR, gxx_type)

        # enable unit test
        LOG.info('[DAP SERVER] gxx_type = {},  unit test enable = {}'.format(gxx_type, args.unit_test_enable))
        if args.unit_test_enable == 1:
            # unit tests can be driven by win_mingw64 and linux_x86_64
            if gxx_type == 'win_mingw64' or gxx_type == 'linux_x86_64' or gxx_type == 'dawin_x86_64':
                test(args, gxx_type, OUTPUT_EXE_DIR, UT_EXE_PATH)
                coverage(gxx_type, True)
                coverage(gxx_type, False)
            else:
                LOG.info('[DAP SERVER] gxx_type = {}, unit tests can be driven only by win_mingw64 and linux_x86_64'.format(gxx_type))
		
        if args.unit_test_enable == 2:
            # smoke tests can be driven by win_mingw64 and linux_x86_64
            if gxx_type == 'win_mingw64' or gxx_type == 'linux_x86_64' or gxx_type == 'dawin_x86_64':
                smoketest(args, gxx_type, OUTPUT_EXE_DIR, UT_EXE_PATH)
            else:
                LOG.info('[DAP SERVER] gxx_type = {}, smoke tests can be driven only by win_mingw64 and linux_x86_64'.format(gxx_type))

def test(args, gxx_type, dir, path):
    LOG.info('[DAP SERVER] unit test file dir = {}, unit test file path = {}'.format(dir, path))
    if path and os.path.exists(path):
        LOG.info('[DAP SERVER] {} - unit test begin...\n'.format(gxx_type))
        test_cmds = [path]
        subprocess.run(test_cmds, cwd = dir)
        LOG.info('[DAP_SERVER] {} - unit test end.\n'.format(gxx_type))
    else:
        LOG.info('[DAP SERVER] {} - unit tests script does not exist.'.format(gxx_type))

def smoketest(args, gxx_type, dir, path):
    LOG.info('[DAP SERVER] smoke test file dir = {}, smoke test file path = {}'.format(dir, path))
    exclude_filter_path = os.path.join(TEST_DIR, "smoke_test_exclude_filter.txt")
    exclude_filter_cmd = ""
    exclude_filter_list = []
    output_file_path = os.path.join(TEST_DIR, "output.json")
    output_file_cmd = "--gtest_output=json:{}".format(output_file_path)
    if os.path.exists(exclude_filter_path):
        with open(exclude_filter_path, 'r') as f:
            for line in f.readlines():
                exclude_filter_list.append(line.strip())
        if len(exclude_filter_list) != 0:
            exclude_filter_files = ":".join(str(i) for i in exclude_filter_list)
            exclude_filter_cmd = "".join("--gtest_filter=-{}".format(exclude_filter_files))
    if path and os.path.exists(path):
        LOG.info('[DAP SERVER] {} - smoke test begin...\n'.format(gxx_type))
        test_cmds = [path, exclude_filter_cmd, output_file_cmd]
        subprocess.run(test_cmds, cwd = dir)
        LOG.info('[DAP_SERVER] {} - smoke test end.\n'.format(gxx_type))
    else:
        LOG.info('[DAP SERVER] {} - smoke tests script does not exist.'.format(gxx_type))
		
def coverage(gxx_type, html_enable=True):
    LOG.info('[DAP SERVER] {} - coverage report(html = {}) is generating...\n'.format(gxx_type, html_enable))
    if html_enable:
        report_dir = os.path.join(UT_REPORT_DIR, gxx_type)
        if os.path.exists(report_dir):
            shutil.rmtree(report_dir, ignore_errors = True)
        os.makedirs(report_dir)
        report_path = os.path.join(report_dir, 'index.html')
        gcovr_cmds = ['gcovr', '-r', '.', 
                    '-e', OPEN_SRC_DIR.replace('\\', '/'), 
                    '-e', TEST_DIR.replace('\\', '/'),
                    '-e', SRC_LAUNCHER_DIR.replace('\\', '/'),
                    '--html', '--html-details',
                    '-o', report_path.replace('\\', '/')]
    else:
        gcovr_cmds = ['gcovr', '-r', '.', 
                      '-e', OPEN_SRC_DIR.replace('\\', '/'), 
                      '-e', TEST_DIR.replace('\\', '/'),
                      '-e', SRC_LAUNCHER_DIR.replace('\\', '/')]
    output = subprocess.Popen(gcovr_cmds, cwd = HOME_DIR, stdout = subprocess.PIPE)
    log_output(output)
    LOG.info('[DAP SERVER] {} - coverage report(html = {}) is generated.\n'.format(gxx_type, html_enable))

def clean(args):
    """clean build outputs and logs"""
    LOG.info('[DAP SERVER] begin clean...\n')
    output_dirs = ['output', LOG_DIR, CMAKE_BUILD_DIR]
    for file_path in output_dirs:
        abs_file_path = os.path.join(HOME_DIR, file_path)
        if os.path.isdir(abs_file_path):
            shutil.rmtree(abs_file_path, ignore_errors = True)
        if os.path.isfile(abs_file_path):
            os.remove(abs_file_path)
    LOG.info('[DAP SERVER] end clean.\n')

def init_log(name):
    """init log config"""
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    log = logging.getLogger(name)
    log.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(asctime)s] [%(module)s] [%(levelname)s] %(message)s')
    streamhandler = logging.StreamHandler(sys.stdout)
    streamhandler.setLevel(logging.DEBUG)
    streamhandler.setFormatter(formatter)
    log.addHandler(streamhandler)
    filehandler = TimedRotatingFileHandler(LOG_FILE,
                                           when = 'W6',
                                           interval = 1,
                                           backupCount = 60)
    filehandler.setLevel(logging.DEBUG)
    filehandler.setFormatter(formatter)
    log.addHandler(filehandler)
    return log

class BuildType(Enum):
    """CMAKE_BUILD_TYPE options"""
    debug = 'Debug'
    release = 'Release'
    relwithdebinfo = 'RelWithDebInfo'

    def __str__(self):
        return self.name

    def __repr(self):
        return str(self)

    @staticmethod
    def argparse(s):
        try:
            return BuildType[s]
        except KeyError:
            return s

def copy_to_target(outputDir, gxx_type):
    srcFiles = ['dap_server', 'launcher']
    suffix = '_{}'.format(gxx_type)
    for file in srcFiles:
        if IS_WINDOWS:
            srcName = os.path.join(outputDir, '{}.exe'.format(file))
            dstName = os.path.join(outputDir, '{}{}.exe'.format(file, suffix))
        else:
            srcName = os.path.join(outputDir, file)
            dstName = os.path.join(outputDir, '{}{}'.format(file, suffix))
        copyfile(srcName, dstName)
        os.chmod(dstName, stat.S_IRWXU + stat.S_IRGRP + stat.S_IXGRP + stat.S_IROTH + stat.S_IXOTH)

def main():
    """build entry"""
    parser = argparse.ArgumentParser(description ='build dap server project')
    subparsers = parser.add_subparsers(help ='sub command help')
    parser_build = subparsers.add_parser('build', help ='build dap server')
    parser_build.add_argument('-t',
                              '--build-type',
                              type = BuildType.argparse,
                              dest = 'build_type',
                              default = 'debug',
                              choices = list(BuildType),
                              help ='select target build type')
    gxx_choices = []
    gxx_default = []
    if IS_WINDOWS:
        gxx_choices = ['win_mingw64']
        gxx_default = ['win_mingw64']
    elif IS_DARWIN:
        gxx_choices = ['darwin_x86_64']
        gxx_default = ['darwin_x86_64']   
    else:
        gxx_choices = ['linux_x86_64', 'linux_aarch64']
        gxx_default = ['linux_x86_64']   

    parser_build.add_argument('-g',
                              '--gxx_types', 
                              dest = 'gxx_types', 
                              type = str, 
                              nargs = '+', 
                              choices = gxx_choices, 
                              default = gxx_default,
                              help = 'gxx toochains name for build.')
    parser_build.add_argument('-u',
                              '--unit-test-enable',
                              dest = 'unit_test_enable',
                              type = int,
                              default = 0,
                              help = '0-disable, 1-enable, it will execute unit tests after targets are build over.')
    parser_build.add_argument('-c',
                              '--copy-target-enable',
                              dest = 'copy_target_enable',
                              type = int,
                              default = 1,
                              help = '0-disable, 1-enable, it will copy targets with OS suffix after targets are build over.')                            
    parser_build.set_defaults(func = build)

    parser_clean = subparsers.add_parser('clean', help='clean build')
    parser_clean.set_defaults(func = clean)

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        args = parser.parse_args(['build'] + sys.argv[1:])

    args.func(args)

if __name__ == '__main__':
    LOG = init_log('root')
    os.environ['LANG'] = 'C'
    information()
    main()