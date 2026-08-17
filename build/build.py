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
import ssl
import subprocess
import sys
import stat
import tarfile
import urllib.request
import urllib.parse
import zipfile
from enum import Enum
from shutil import copyfile
from logging.handlers import TimedRotatingFileHandler

IS_WINDOWS = platform.system() == 'Windows'
IS_LINUX = platform.system() == 'Linux'
IS_DARWIN = platform.system() == 'Darwin'
MAKE_JOBS = multiprocessing.cpu_count() + 2

# An explicit SDK path takes precedence over downloading a DailyMain SDK.
DEFAULT_SDK_PATH = os.environ.get('CANGJIE_SDK_PATH') or os.environ.get('CANGJIE_HOME')
DEFAULT_SDK_SOURCE = os.environ.get('CANGJIE_SDK_SOURCE', 'auto')
DEFAULT_SDK_URL = os.environ.get('CANGJIE_SDK_URL')
DEFAULT_SDK_BASE_URL = os.environ.get(
    'CANGJIE_SDK_BASE_URL',
    'https://cangjie103.obs.cn-north-4.myhuaweicloud.com/CangjieDaily/Cangjie/DailyMain'
)
DEFAULT_SDK_VERSION = os.environ.get('CANGJIE_SDK_VERSION', '1.0.0')

BUILD_DIR = os.path.dirname(os.path.realpath(__file__))
CMAKE_BUILD_DIR = os.path.join(BUILD_DIR, 'build')
DEFAULT_SDK_CACHE_DIR = os.environ.get('CANGJIE_SDK_CACHE_DIR', os.path.join(BUILD_DIR, 'sdk'))

HOME_DIR = os.path.dirname(BUILD_DIR)
OPEN_SRC_DIR = os.path.join(HOME_DIR, 'open_source')
SRC_DIR = os.path.join(HOME_DIR, 'src')
TEST_DIR = os.path.join(HOME_DIR, 'test')

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
    LOG.info('test directory = {}'.format(TEST_DIR))
    LOG.info('output directory = {}'.format(OUTPUT_DIR))
    LOG.info('coverage report directory = {}'.format(UT_REPORT_DIR))
    LOG.info('build directory = {}'.format(BUILD_DIR))
    LOG.info('log directory = {}'.format(LOG_DIR))

def setup_build_tools():
    """Find cmake and build tool (ninja/mingw32-make), add to PATH if needed.
    Searches PATH first, then common locations (DevEco Studio, etc.).
    """
    if not IS_WINDOWS:
        if not shutil.which('cmake'):
            LOG.error('cmake not found in PATH. Please install cmake and add it to PATH.')
            sys.exit(1)
        return

    # Common cmake/ninja/clang locations on Windows
    search_dirs = []
    devec_home = os.environ.get('DEVECO_SDK_HOME', '')
    if devec_home:
        search_dirs.append(os.path.join(devec_home, 'build-tools', 'cmake', 'bin'))
        search_dirs.append(os.path.join(devec_home, 'native', 'build-tools', 'cmake', 'bin'))
        search_dirs.append(os.path.join(devec_home, 'native', 'llvm', 'bin'))
        search_dirs.append(os.path.join(devec_home, 'native', 'BUILD-~1', 'cmake', 'bin'))
    search_dirs.append(r'C:\Program Files\Huawei\DevEco Studio\sdk\default\openharmony\native\build-tools\cmake\bin')
    search_dirs.append(r'C:\Program Files\Huawei\DevEco Studio\sdk\default\openharmony\native\llvm\bin')

    found_dirs = set()
    for tool in ['cmake', 'ninja', 'mingw32-make', 'clang', 'clang++']:
        exe_names = [tool, tool + '.exe'] if IS_WINDOWS else [tool]
        if any(shutil.which(n) for n in exe_names):
            continue
        for d in search_dirs:
            tool_found = False
            for exe in exe_names:
                if os.path.isfile(os.path.join(d, exe)):
                    found_dirs.add(d)
                    tool_found = True
                    break
            if tool_found:
                break

    for d in found_dirs:
        os.environ['PATH'] = d + os.pathsep + os.environ['PATH']
        LOG.info('[DAP SERVER] added build tools to PATH: {}'.format(d))

    if not shutil.which('cmake') and not shutil.which('cmake.exe'):
        LOG.error('cmake not found. Please add it to PATH, e.g.:')
        LOG.error('  $env:Path = "C:\\Program Files\\Huawei\\DevEco Studio\\sdk\\default\\openharmony\\native\\build-tools\\cmake\\bin;" + $env:Path')
        sys.exit(1)


def check_compiler(gxx_type):
    """Check that the required compiler is available for the gxx_type."""
    if gxx_type == 'win_mingw64':
        if not (shutil.which('g++') or shutil.which('g++.exe')):
            LOG.error('[DAP SERVER] g++ not found. win_mingw64 requires MinGW g++.')
            LOG.error('Please install MinGW.')
            sys.exit(1)
    elif gxx_type == 'win_ninja':
        if not (shutil.which('g++') or shutil.which('g++.exe')):
            LOG.error('[DAP SERVER] g++ not found. win_ninja requires g++.')
            LOG.error('Please install MinGW.')
            sys.exit(1)

def get_liblldb_path(sdk_path):
    """Resolve liblldb dynamic library path from Cangjie SDK path.

    Windows : <sdk>/tools/bin/liblldb.dll
    Linux   : <sdk>/third_party/llvm/lib/liblldb.so
    macOS   : <sdk>/third_party/llvm/lib/liblldb.dylib
    """
    if not sdk_path:
        LOG.error('Cangjie SDK path is required. Please pass --sdk-path or set CANGJIE_SDK_PATH env var.')
        sys.exit(1)
    if IS_WINDOWS:
        relative_path = os.path.join('tools', 'bin', 'liblldb.dll')
    elif IS_DARWIN:
        relative_path = os.path.join('third_party', 'llvm', 'lib', 'liblldb.dylib')
    else:
        relative_path = os.path.join('third_party', 'llvm', 'lib', 'liblldb.so')
    liblldb = os.path.join(sdk_path, relative_path)
    if not os.path.exists(liblldb):
        LOG.error('liblldb not found at: {}'.format(liblldb))
        sys.exit(1)
    return liblldb.replace('\\', '/')


def get_sdk_archive(gxx_type, version):
    """Return the DailyMain SDK platform name and archive name for a target."""
    platforms = {
        'win_mingw64': ('windows-x64', 'zip'),
        'win_ninja': ('windows-x64', 'zip'),
        'linux_x86_64': ('linux-x64', 'tar.gz'),
        'linux_aarch64': ('linux-aarch64', 'tar.gz'),
        'darwin_x86_64': ('mac-x64', 'tar.gz'),
        'darwin_aarch64': ('mac-aarch64', 'tar.gz'),
    }
    if gxx_type not in platforms:
        raise RuntimeError('Unsupported SDK target: {}'.format(gxx_type))
    platform_name, extension = platforms[gxx_type]
    archive_name = 'cangjie-sdk-{}-{}.{}'.format(platform_name, version, extension)
    return platform_name, archive_name


def download_file(url, target_path, insecure=False):
    """Download an SDK archive atomically."""
    LOG.info('[DAP SERVER] downloading Cangjie SDK: %s', url)
    os.makedirs(os.path.dirname(target_path), exist_ok=True)
    temporary_path = target_path + '.part'
    if os.path.exists(temporary_path):
        os.remove(temporary_path)
    request = urllib.request.Request(url, headers={'User-Agent': 'Cangjie-DAP-Build'})
    context = ssl._create_unverified_context() if insecure else None
    try:
        with urllib.request.urlopen(request, timeout=120, context=context) as response:
            with open(temporary_path, 'wb') as output:
                shutil.copyfileobj(response, output)
        os.replace(temporary_path, target_path)
    finally:
        if os.path.exists(temporary_path):
            os.remove(temporary_path)


def validate_archive_member(destination, member_name):
    """Reject archive entries which escape the extraction directory."""
    destination = os.path.realpath(destination)
    target = os.path.realpath(os.path.join(destination, member_name))
    if target != destination and not target.startswith(destination + os.sep):
        raise RuntimeError('Unsafe path in SDK archive: {}'.format(member_name))


def extract_sdk_archive(archive_path, destination):
    """Extract a zip or gzip-compressed tar SDK archive safely."""
    if archive_path.endswith('.zip'):
        with zipfile.ZipFile(archive_path) as archive:
            for member in archive.namelist():
                validate_archive_member(destination, member)
            archive.extractall(destination)
        return
    with tarfile.open(archive_path, 'r:gz') as archive:
        for member in archive.getmembers():
            validate_archive_member(destination, member.name)
        archive.extractall(destination)


def get_liblldb_relative_path():
    if IS_WINDOWS:
        return os.path.join('tools', 'bin', 'liblldb.dll')
    if IS_DARWIN:
        return os.path.join('third_party', 'llvm', 'lib', 'liblldb.dylib')
    return os.path.join('third_party', 'llvm', 'lib', 'liblldb.so')


def find_sdk_root(destination):
    """Find an extracted SDK root by looking for its platform liblldb."""
    relative_liblldb = get_liblldb_relative_path()
    candidates = [destination, os.path.join(destination, 'cangjie')]
    if os.path.isdir(destination):
        candidates.extend(
            os.path.join(destination, name)
            for name in os.listdir(destination)
            if os.path.isdir(os.path.join(destination, name))
        )
    for candidate in candidates:
        if os.path.isfile(os.path.join(candidate, relative_liblldb)):
            return candidate
    return None


def download_sdk(gxx_type, base_url, version, cache_dir, insecure=False, sdk_url=None):
    """Download and extract the DailyMain SDK for one DAP target."""
    platform_name, archive_name = get_sdk_archive(gxx_type, version)
    if sdk_url:
        supplied_name = os.path.basename(urllib.parse.urlparse(sdk_url).path)
        if supplied_name:
            archive_name = supplied_name
    archive_path = os.path.join(cache_dir, 'downloads', archive_name)
    extract_dir = os.path.join(cache_dir, '{}-{}'.format(platform_name, version))
    sdk_path = find_sdk_root(extract_dir)
    if sdk_path:
        LOG.info('[DAP SERVER] reusing cached Cangjie SDK: %s', sdk_path)
        return sdk_path

    if not os.path.isfile(archive_path):
        url = sdk_url or '{}/{}'.format(base_url.rstrip('/'), archive_name)
        download_file(url, archive_path, insecure)
    else:
        LOG.info('[DAP SERVER] reusing downloaded SDK archive: %s', archive_path)

    if os.path.isdir(extract_dir):
        shutil.rmtree(extract_dir)
    os.makedirs(extract_dir)
    LOG.info('[DAP SERVER] extracting Cangjie SDK to: %s', extract_dir)
    extract_sdk_archive(archive_path, extract_dir)
    sdk_path = find_sdk_root(extract_dir)
    if not sdk_path:
        raise RuntimeError(
            'Downloaded SDK does not contain {}: {}'.format(
                get_liblldb_relative_path(), archive_path
            )
        )
    return sdk_path


def resolve_sdk_path(args, gxx_type):
    """Resolve the SDK according to --sdk-source."""
    if args.sdk_source not in ('auto', 'path', 'download'):
        LOG.error('[DAP SERVER] unsupported SDK source: {}'.format(args.sdk_source))
        sys.exit(1)
    if args.sdk_source == 'path' and not args.sdk_path:
        LOG.error('[DAP SERVER] --sdk-source path requires --sdk-path or CANGJIE_SDK_PATH.')
        sys.exit(1)
    if args.sdk_source in ('auto', 'path') and args.sdk_path:
        if os.path.isfile(os.path.join(args.sdk_path, get_liblldb_relative_path())):
            LOG.info('[DAP SERVER] using explicit Cangjie SDK: %s', args.sdk_path)
            return args.sdk_path
        if args.sdk_source == 'path':
            LOG.error('[DAP SERVER] liblldb is missing from SDK: {}'.format(args.sdk_path))
            sys.exit(1)
        LOG.warning('[DAP SERVER] configured SDK is invalid, falling back to download: %s', args.sdk_path)
    if args.sdk_source == 'auto' and args.sdk_url:
        LOG.info('[DAP SERVER] SDK URL is configured, download SDK: %s', args.sdk_url)
    try:
        return download_sdk(
            gxx_type,
            args.sdk_base_url,
            args.sdk_version,
            args.sdk_cache_dir,
            args.sdk_insecure,
            args.sdk_url,
        )
    except Exception as error:
        LOG.error('[DAP SERVER] failed to prepare Cangjie SDK: %s', error)
        sys.exit(1)

def log_output(output):
    """log command output"""
    while True:
        line = output.stdout.readline()
        if not line:
            ret = output.wait()
            if ret != 0:
                LOG.error('build error: %d!\n', ret)
                sys.exit(1)
            break
        LOG.info(line.decode('UTF-8').rstrip())

def detect_generator():
    """Detect the available CMake generator on Windows.
    Prefers Ninja (DevEco Studio, etc.) and falls back to MinGW Makefiles.
    """
    if IS_WINDOWS:
        for name in ['ninja', 'ninja.exe']:
            if shutil.which(name):
                return 'Ninja'
        for name in ['mingw32-make', 'mingw32-make.exe']:
            if shutil.which(name):
                return 'MinGW Makefiles'
        return 'Ninja'
    return 'Ninja'


def detect_compiler():
    """Detect available C++ compiler on Windows. Returns 'gpp' or 'clang' or None."""
    for name in ['g++.exe', 'g++']:
        if shutil.which(name):
            return 'gpp'
    for name in ['clang++.exe', 'clang++']:
        if shutil.which(name):
            return 'clang'
    return None


def get_windows_cmake_flags(sdk_path, compiler):
    """Get extra cmake flags for Windows based on compiler type.
    When using clang (DevEco Studio, etc.), use CROSS_WINDOWS mode with
    libc++ and lld, linking against the SDK's mingw runtime libraries.
    """
    if compiler != 'clang':
        return []
    mingw_lib = os.path.join(sdk_path, 'third_party', 'mingw', 'lib').replace('\\', '/')
    return [
        '-DCROSS_WINDOWS=ON',
        '-DCMAKE_CXX_FLAGS=-stdlib=libc++',
        '-DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld -L{} -LC:/Windows/System32 -lunwind -lpsapi'.format(mingw_lib),
    ]


def get_build_config(gxx_type, sdk_path):
    """Get generator, compiler_type and extra cmake flags for a gxx_type."""
    if gxx_type == 'win_mingw64':
        gpp = shutil.which('g++.exe') or shutil.which('g++')
        gcc = shutil.which('gcc.exe') or shutil.which('gcc')
        if not gcc or not gpp:
            LOG.error('[DAP SERVER] gcc and g++ are required for win_mingw64.')
            sys.exit(1)
        return 'MinGW Makefiles', 'MinGW', [
            '-DCMAKE_C_COMPILER={}'.format(gcc.replace('\\', '/')),
            '-DCMAKE_CXX_COMPILER={}'.format(gpp.replace('\\', '/')),
        ]
    elif gxx_type == 'win_ninja':
        gpp = shutil.which('g++.exe') or shutil.which('g++')
        gcc = shutil.which('gcc.exe') or shutil.which('gcc')
        if not gcc or not gpp:
            LOG.error('[DAP SERVER] gcc and g++ are required for win_ninja.')
            sys.exit(1)
        return 'Ninja', 'MinGW', [
            '-DCMAKE_C_COMPILER={}'.format(gcc.replace('\\', '/')),
            '-DCMAKE_CXX_COMPILER={}'.format(gpp.replace('\\', '/')),
        ]
    elif gxx_type == 'linux_aarch64':
        flags = ['-DCROSS_NAME=aarch64-linux-gnu']
        if platform.machine().lower() not in ('aarch64', 'arm64'):
            cross_gcc = shutil.which('aarch64-linux-gnu-gcc')
            cross_gxx = shutil.which('aarch64-linux-gnu-g++')
            if not cross_gcc or not cross_gxx:
                LOG.error('[DAP SERVER] aarch64-linux-gnu-gcc/g++ are required for linux_aarch64 cross build.')
                sys.exit(1)
            flags.extend([
                '-DCMAKE_SYSTEM_NAME=Linux',
                '-DCMAKE_SYSTEM_PROCESSOR=aarch64',
                '-DCMAKE_C_COMPILER={}'.format(cross_gcc),
                '-DCMAKE_CXX_COMPILER={}'.format(cross_gxx),
            ])
            flags.append('-DCROSS_SET=Yes')
        return 'Ninja', 'Ninja', flags
    elif gxx_type in ('darwin_x86_64', 'darwin_aarch64'):
        architecture = 'x86_64' if gxx_type == 'darwin_x86_64' else 'arm64'
        return 'Ninja', 'Ninja', [
            '-DCMAKE_OSX_ARCHITECTURES={}'.format(architecture),
            '-DCMAKE_SYSTEM_PROCESSOR={}'.format(architecture),
        ]
    else:
        return 'Ninja', 'Ninja', []


def reset_incompatible_cmake_cache(build_dir, compiler_path):
    """Discard CMake's compiler cache when switching toolchains."""
    cache_file = os.path.join(build_dir, 'CMakeCache.txt')
    if not os.path.isfile(cache_file):
        return
    with open(cache_file, 'r') as cache:
        cache_content = cache.read().lower().replace('\\', '/')
    expected_compiler = os.path.normcase(os.path.abspath(compiler_path)).replace('\\', '/')
    if expected_compiler in cache_content:
        return
    LOG.info('[DAP SERVER] clearing incompatible CMake compiler cache in {}'.format(build_dir))
    os.remove(cache_file)
    cmake_files_dir = os.path.join(build_dir, 'CMakeFiles')
    if os.path.isdir(cmake_files_dir):
        shutil.rmtree(cmake_files_dir)


def build(args):
    """dap server build"""
    LOG.info('[DAP SERVER] begin build...\n')
    setup_build_tools()
    LOG.info('[DAP SERVER] gxx_types = {}\n'.format(args.gxx_types))
    for i, gxx_type in enumerate(args.gxx_types):
        sdk_path = resolve_sdk_path(args, gxx_type)
        liblldb_path = get_liblldb_path(sdk_path)
        check_compiler(gxx_type)
        generator, compiler_type, extra_flags = get_build_config(gxx_type, sdk_path)
        LOG.info('[DAP SERVER] gxx_type = {}'.format(gxx_type))
        LOG.info('[DAP SERVER] generator = {}'.format(generator))
        LOG.info('[DAP SERVER] compiler_type = {}'.format(compiler_type))
        OUTPUT_EXE_DIR = os.path.join(OUTPUT_BIN_DIR, gxx_type)
        UT_EXE_PATH = os.path.join(OUTPUT_EXE_DIR, UT_EXE_NAME)
        LOG.info('[DAP SERVER] unit test executable file path = {}'.format(UT_EXE_PATH))
        build_dir = os.path.join(CMAKE_BUILD_DIR, gxx_type)
        if not os.path.exists(build_dir):
            os.makedirs(build_dir)
        if gxx_type in ('win_mingw64', 'win_ninja'):
            reset_incompatible_cmake_cache(build_dir, shutil.which('g++.exe') or shutil.which('g++'))
        build_cmds = ['cmake', HOME_DIR, '-G', generator, '-DCMAKE_BUILD_TYPE={}'.format(args.build_type.value), '-DCOMPILER_TYPE={}'.format(compiler_type), '-DLIBLLDB_PATH={}'.format(liblldb_path), '-DWIN_OUTPUT_NAME={}'.format(gxx_type)]
        build_cmds.extend(extra_flags)
        if args.unit_test_enable == 1:
            build_cmds.append('-DENABLE_COVERAGE=ON')
        output = subprocess.Popen(build_cmds, cwd = build_dir, stdout = subprocess.PIPE)
        log_output(output)

        build_cmds = ['cmake', '--build', '.', '-j', str(MAKE_JOBS)]
        if args.target:
            build_cmds.extend(['--target'] + args.target)
        output = subprocess.Popen(build_cmds, cwd = build_dir, stdout = subprocess.PIPE)
        log_output(output)
        LOG.info('[DAP SERVER] end build.\n')

        # copy dap_server with OS suffix
        if args.copy_target_enable == 1:
            copy_to_target(OUTPUT_EXE_DIR, gxx_type)

        # enable unit test
        LOG.info('[DAP SERVER] gxx_type = {},  unit test enable = {}'.format(gxx_type, args.unit_test_enable))
        if args.unit_test_enable == 1:
            if gxx_type in ('win_mingw64', 'win_ninja', 'linux_x86_64', 'darwin_x86_64', 'darwin_aarch64'):
                test(args, gxx_type, OUTPUT_EXE_DIR, UT_EXE_PATH)
                coverage(gxx_type, True)
                coverage(gxx_type, False)
            else:
                LOG.info('[DAP SERVER] gxx_type = {}, unit tests can be driven only on native platforms'.format(gxx_type))
		
        if args.unit_test_enable == 2:
            if gxx_type in ('win_mingw64', 'win_ninja', 'linux_x86_64', 'darwin_x86_64', 'darwin_aarch64'):
                smoketest(args, gxx_type, OUTPUT_EXE_DIR, UT_EXE_PATH)
            else:
                LOG.info('[DAP SERVER] gxx_type = {}, smoke tests can be driven only on native platforms'.format(gxx_type))

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
                    '--html', '--html-details',
                    '-o', report_path.replace('\\', '/')]
    else:
        gcovr_cmds = ['gcovr', '-r', '.',
                      '-e', OPEN_SRC_DIR.replace('\\', '/'),
                      '-e', TEST_DIR.replace('\\', '/')]
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
    srcFiles = ['dap_server']
    suffix = '_{}'.format(gxx_type)
    for file in srcFiles:
        if IS_WINDOWS:
            srcName = os.path.join(outputDir, '{}.exe'.format(file))
            dstName = os.path.join(outputDir, '{}{}.exe'.format(file, suffix))
        else:
            srcName = os.path.join(outputDir, file)
            dstName = os.path.join(outputDir, '{}{}'.format(file, suffix))
        if not os.path.exists(srcName):
            LOG.warning('[DAP SERVER] {} not found, skip copy'.format(srcName))
            continue
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
        gxx_choices = ['win_mingw64', 'win_ninja']
        gxx_default = ['win_mingw64', 'win_ninja']
    elif IS_DARWIN:
        gxx_choices = ['darwin_x86_64', 'darwin_aarch64']
        if platform.machine() == 'arm64':
            gxx_default = ['darwin_aarch64']
        else:
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
    parser_build.add_argument('--sdk-path',
                              dest = 'sdk_path',
                              type = str,
                              default = DEFAULT_SDK_PATH,
                              help = 'Cangjie SDK path. If omitted, download the matching DailyMain SDK.')
    parser_build.add_argument('--sdk-source',
                              dest = 'sdk_source',
                              type = str,
                              choices = ['auto', 'path', 'download'],
                              default = DEFAULT_SDK_SOURCE,
                              help = 'SDK source: auto, path, or download.')
    parser_build.add_argument('--sdk-url',
                              dest = 'sdk_url',
                              type = str,
                              default = DEFAULT_SDK_URL,
                              help = 'Full SDK archive URL. Overrides --sdk-base-url when downloading.')
    parser_build.add_argument('--sdk-base-url',
                              dest = 'sdk_base_url',
                              type = str,
                              default = DEFAULT_SDK_BASE_URL,
                              help = 'Base URL used to download a Cangjie SDK when --sdk-path is omitted.')
    parser_build.add_argument('--sdk-version',
                              dest = 'sdk_version',
                              type = str,
                              default = DEFAULT_SDK_VERSION,
                              help = 'Cangjie SDK version used in the downloaded archive name.')
    parser_build.add_argument('--sdk-cache-dir',
                              dest = 'sdk_cache_dir',
                              type = str,
                              default = DEFAULT_SDK_CACHE_DIR,
                              help = 'Directory used to cache downloaded and extracted SDKs.')
    parser_build.add_argument('--sdk-insecure',
                              dest = 'sdk_insecure',
                              action = 'store_true',
                              default = os.environ.get('CANGJIE_SDK_INSECURE', '').lower() in ('1', 'true', 'yes'),
                              help = 'Disable TLS certificate verification for SDK download.')
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
    parser_build.add_argument('--target',
                              dest = 'target',
                              type = str,
                              nargs = '+',
                              default = None,
                              help = 'build specific cmake target(s) only, e.g. --target dap_server. Default: all targets.')
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
