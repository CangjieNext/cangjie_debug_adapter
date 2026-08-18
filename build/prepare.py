#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

"""Prepare open_source dependencies for dap_server build.

Downloads and builds the following dependencies into open_source/:
  - googletest + googlemock : static libraries + headers
  - libuv                   : static library + headers
  - nlohmann/json           : header-only (json.hpp)
  - lldb API headers        : headers only (from Cangjie llvm-project)

Each platform builds only its own static libraries:
  - Windows          -> open_source/<dep>/lib/mingw/
  - Linux x86_64     -> open_source/<dep>/lib/gcc/
  - Linux aarch64    -> open_source/<dep>/lib/aarch64/
  - macOS (universal)-> open_source/<dep>/lib/darwin/

Usage:
  python3 prepare.py prepare           # download and build all dependencies
  python3 prepare.py prepare --force   # redo even if already done
  python3 prepare.py clean             # clean downloaded and built dependencies
"""

import argparse
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
import urllib.request
from logging.handlers import TimedRotatingFileHandler

IS_WINDOWS = platform.system() == 'Windows'
IS_LINUX = platform.system() == 'Linux'
IS_DARWIN = platform.system() == 'Darwin'
MAKE_JOBS = multiprocessing.cpu_count() + 2

# Default Cangjie SDK path per platform.
DEFAULT_SDK_PATH = r'D:\cangjie\sdk\cangjie120' if IS_WINDOWS else os.environ.get('CANGJIE_SDK_PATH')

BUILD_DIR = os.path.dirname(os.path.realpath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
OPEN_SRC_DIR = os.path.join(HOME_DIR, 'open_source')
DEPS_DIR = os.path.join(BUILD_DIR, 'deps')
DOWNLOAD_DIR = os.path.join(DEPS_DIR, 'downloads')
DEPS_BUILD_DIR = os.path.join(DEPS_DIR, 'build')

LOG_DIR = os.path.join(BUILD_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'prepare.log')

GTEST_REPO_URLS = [
    'https://github.com/google/googletest.git',
    'https://gitee.com/mirrors/googletest.git',
    'https://gitclone.com/github.com/google/googletest.git',
]
GTEST_TAG = 'v1.13.0'
LIBUV_REPO_URLS = [
    'https://github.com/libuv/libuv.git',
    'https://gitee.com/mirrors/libuv.git',
    'https://gitclone.com/github.com/libuv/libuv.git',
]
LIBUV_TAG = 'v1.44.1'
JSON_VERSION = 'v3.11.3'
JSON_HEADER_URLS = [
    'https://github.com/nlohmann/json/releases/download/{}/json.hpp',
    'https://gitee.com/mirrors/JSON-for-Modern-CPlusPlus/releases/download/{}/json.hpp',
]
LLDB_REPO_URL = 'https://gitcode.com/Cangjie/llvm-project.git'


class BuildConfig:
    """Build configuration for a specific compiler/toolchain."""
    def __init__(self, name, lib_subdir, generator, cmake_flags=None):
        self.name = name
        self.lib_subdir = lib_subdir
        self.generator = generator
        self.cmake_flags = cmake_flags or []


def get_build_configs(sdk_path=None, target=None):
    """Get list of build configurations for the current platform.
    On Windows, returns configs for each available compiler (g++ and/or clang++).
    """
    system = platform.system()
    machine = platform.machine()

    if target in ('win_mingw64', 'win_ninja'):
        if system != 'Windows':
            raise RuntimeError('{} must be prepared on Windows'.format(target))
        gpp = shutil.which('g++') or shutil.which('g++.exe')
        gcc = shutil.which('gcc') or shutil.which('gcc.exe')
        if not gpp or not gcc:
            LOG.error('No MinGW C++ compiler found for {}'.format(target))
            sys.exit(1)
        compiler_flags = [
            '-DCMAKE_C_COMPILER={}'.format(gcc.replace('\\', '/')),
            '-DCMAKE_CXX_COMPILER={}'.format(gpp.replace('\\', '/')),
        ]
        if target == 'win_mingw64':
            generator = 'MinGW Makefiles'
            if not shutil.which('mingw32-make') and not shutil.which('mingw32-make.exe'):
                raise RuntimeError('mingw32-make is required for {}'.format(target))
        else:
            generator = 'Ninja'
            if not shutil.which('ninja') and not shutil.which('ninja.exe'):
                raise RuntimeError('ninja is required for {}'.format(target))
        return [BuildConfig(target, 'mingw', generator, compiler_flags)]
    if target in ('linux_x86_64', 'linux_aarch64'):
        if system != 'Linux':
            raise RuntimeError('{} must be prepared on Linux'.format(target))
        if target == 'linux_x86_64':
            return [BuildConfig(target, 'gcc', 'Ninja')]
        if machine == 'aarch64':
            return [BuildConfig(target, 'aarch64', 'Ninja')]
        cross_gcc = shutil.which('aarch64-linux-gnu-gcc')
        cross_gxx = shutil.which('aarch64-linux-gnu-g++')
        if not cross_gcc or not cross_gxx:
            raise RuntimeError('aarch64-linux-gnu-gcc/g++ are required for cross compiling {}'.format(target))
        return [BuildConfig(target, 'aarch64', 'Ninja', [
            '-DCMAKE_SYSTEM_NAME=Linux',
            '-DCMAKE_SYSTEM_PROCESSOR=aarch64',
            '-DCMAKE_C_COMPILER={}'.format(cross_gcc),
            '-DCMAKE_CXX_COMPILER={}'.format(cross_gxx),
        ])]
    if target in ('darwin_x86_64', 'darwin_aarch64'):
        if system != 'Darwin':
            raise RuntimeError('{} must be prepared on macOS'.format(target))
        architecture = 'x86_64' if target == 'darwin_x86_64' else 'arm64'
        return [BuildConfig(target, 'darwin', 'Ninja', [
            '-DCMAKE_OSX_ARCHITECTURES={}'.format(architecture),
        ])]

    if system == 'Windows':
        configs = []
        gpp = shutil.which('g++') or shutil.which('g++.exe')
        gcc = shutil.which('gcc') or shutil.which('gcc.exe')
        if gpp and gcc:
            compiler_flags = [
                '-DCMAKE_C_COMPILER={}'.format(gcc.replace('\\', '/')),
                '-DCMAKE_CXX_COMPILER={}'.format(gpp.replace('\\', '/')),
            ]
            has_mingw_make = shutil.which('mingw32-make') or shutil.which('mingw32-make.exe')
            has_ninja = shutil.which('ninja') or shutil.which('ninja.exe')
            if has_mingw_make:
                configs.append(BuildConfig('win_mingw64', 'mingw', 'MinGW Makefiles', compiler_flags))
            if has_ninja:
                configs.append(BuildConfig('win_ninja', 'mingw', 'Ninja', compiler_flags))
            if not configs and has_mingw_make:
                configs.append(BuildConfig('win_mingw64', 'mingw', 'MinGW Makefiles', compiler_flags))
            if not configs and has_ninja:
                configs.append(BuildConfig('win_ninja', 'mingw', 'Ninja', compiler_flags))
        if not configs:
            LOG.error('No C++ compiler/build tool found. Please install MinGW g++ and/or ninja.')
            sys.exit(1)
        return configs
    elif system == 'Linux':
        if machine == 'aarch64':
            return [BuildConfig('linux_aarch64', 'aarch64', 'Ninja')]
        return [BuildConfig('linux_x86_64', 'gcc', 'Ninja')]
    elif system == 'Darwin':
        name = 'darwin_aarch64' if machine == 'arm64' else 'darwin_x86_64'
        return [BuildConfig(name, 'darwin', 'Ninja', ['-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64'])]
    raise RuntimeError('Unsupported platform: {}'.format(system))


def information(configs):
    LOG.info('os platform = %s (%s)', platform.system(), platform.machine())
    LOG.info('build configurations:')
    for config in configs:
        LOG.info('  %s: lib_subdir=%s, generator=%s, extra_flags=%s',
                 config.name, config.lib_subdir, config.generator, config.cmake_flags)
    LOG.info('open_source directory = %s', OPEN_SRC_DIR)
    LOG.info('deps work directory = %s', DEPS_DIR)


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
    filehandler = TimedRotatingFileHandler(LOG_FILE, when='W6', interval=1, backupCount=60)
    filehandler.setLevel(logging.DEBUG)
    filehandler.setFormatter(formatter)
    log.addHandler(filehandler)
    return log


def setup_build_tools():
    """Find cmake and build tool, add to PATH if needed."""
    if not IS_WINDOWS:
        if not shutil.which('cmake'):
            LOG.error('cmake not found in PATH. Please install cmake and add it to PATH.')
            sys.exit(1)
        return

    search_dirs = []
    devec_home = os.environ.get('DEVECO_SDK_HOME', '')
    if devec_home:
        search_dirs.append(os.path.join(devec_home, 'build-tools', 'cmake', 'bin'))
        search_dirs.append(os.path.join(devec_home, 'native', 'build-tools', 'cmake', 'bin'))
        search_dirs.append(os.path.join(devec_home, 'native', 'llvm', 'bin'))
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
        LOG.info('added build tools to PATH: %s', d)

    if not shutil.which('cmake') and not shutil.which('cmake.exe'):
        LOG.error('cmake not found. Please add it to PATH, e.g.:')
        LOG.error('  $env:Path = "C:\\Program Files\\Huawei\\DevEco Studio\\sdk\\default\\openharmony\\native\\build-tools\\cmake\\bin;" + $env:Path')
        sys.exit(1)


def run_cmd(cmd, cwd=None, check=True):
    """Run a command and log output."""
    LOG.info('Running: %s', ' '.join(str(c) for c in cmd))
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.stdout:
        for line in result.stdout.strip().split('\n'):
            if line:
                LOG.info(line)
    if result.stderr:
        for line in result.stderr.strip().split('\n'):
            if line:
                LOG.warning(line)
    if check and result.returncode != 0:
        LOG.error('Command failed with return code %d', result.returncode)
        sys.exit(1)
    return result


def copy_tree(src, dst):
    """Copy a directory tree, removing destination first."""
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def find_file(directory, name):
    """Find a file by name recursively in a directory."""
    for root, _, files in os.walk(directory):
        if name in files:
            return os.path.join(root, name)
    return None


def download_file(urls_or_url, target_path):
    """Download a file from URL(s) to target path. Tries multiple URLs."""
    LOG.info('Downloading to: %s', target_path)
    os.makedirs(os.path.dirname(target_path), exist_ok=True)
    urls = urls_or_url if isinstance(urls_or_url, list) else [urls_or_url]
    for url in urls:
        try:
            LOG.info('Trying: %s', url)
            req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
            with urllib.request.urlopen(req, timeout=30) as response:
                with open(target_path, 'wb') as f:
                    shutil.copyfileobj(response, f)
            LOG.info('Downloaded: %s', target_path)
            return
        except Exception as e:
            LOG.warning('Download from %s failed: %s', url, e)
    LOG.error('All download URLs failed')
    raise RuntimeError('All download URLs failed')


def git_clone(tag, url_or_urls, target_dir, force=False):
    """Clone a git repo at a specific tag (shallow). Tries multiple URLs."""
    if os.path.isdir(target_dir):
        LOG.info('Repo already cloned, reusing: %s', target_dir)
        return
    os.makedirs(os.path.dirname(target_dir), exist_ok=True)
    urls = url_or_urls if isinstance(url_or_urls, list) else [url_or_urls]
    for url in urls:
        try:
            run_cmd(['git', 'clone', '--depth', '1', '--branch', tag, url, target_dir], check=True)
            return
        except SystemExit:
            LOG.warning('Clone from %s failed, trying next...', url)
            if os.path.isdir(target_dir):
                shutil.rmtree(target_dir, ignore_errors=True)
    LOG.error('All clone URLs failed for %s', target_dir)
    sys.exit(1)


def prepare_json(force=False):
    """Download nlohmann/json single header."""
    LOG.info('[json] preparing nlohmann/json header...')
    target = os.path.join(OPEN_SRC_DIR, 'json_modern_c++', 'include', 'json.hpp')
    if os.path.exists(target):
        LOG.info('[json] json.hpp already exists, skipping.')
        return
    try:
        download_file([u.format(JSON_VERSION) for u in JSON_HEADER_URLS], target)
    except Exception as e:
        LOG.error('[json] download failed: %s', e)
        LOG.error('[json] you can manually download json.hpp from:')
        LOG.error('       https://github.com/nlohmann/json/releases')
        sys.exit(1)
    LOG.info('[json] done.')


def _copy_gtest_headers(src_dir):
    """Copy gtest and gmock headers to open_source."""
    gtest_include_src = os.path.join(src_dir, 'googletest', 'include', 'gtest')
    gtest_include_dst = os.path.join(OPEN_SRC_DIR, 'googletest', 'include', 'gtest')
    if os.path.isdir(gtest_include_src):
        copy_tree(gtest_include_src, gtest_include_dst)
        LOG.info('[googletest] copied gtest headers')

    gmock_include_src = os.path.join(src_dir, 'googlemock', 'include', 'gmock')
    gmock_include_dst = os.path.join(OPEN_SRC_DIR, 'googlemock', 'include', 'gmock')
    if os.path.isdir(gmock_include_src):
        copy_tree(gmock_include_src, gmock_include_dst)
        LOG.info('[googlemock] copied gmock headers')

    gmock_gmock_h = os.path.join(src_dir, 'googlemock', 'include', 'gmock', 'gmock.h')
    gmock_root_dst = os.path.join(OPEN_SRC_DIR, 'googlemock', 'gmock.h')
    if os.path.isfile(gmock_gmock_h):
        os.makedirs(os.path.dirname(gmock_root_dst), exist_ok=True)
        shutil.copy2(gmock_gmock_h, gmock_root_dst)


def prepare_googletest(config, force=False):
    """Clone and build googletest (includes gmock) for a build config."""
    LOG.info('[googletest] preparing googletest + googlemock for %s...', config.name)

    src_dir = os.path.join(DOWNLOAD_DIR, 'googletest')
    build_dir = os.path.join(DEPS_BUILD_DIR, 'googletest_{}'.format(config.lib_subdir))

    gtest_lib = os.path.join(OPEN_SRC_DIR, 'googletest', 'lib', config.lib_subdir, 'libgtest.a')
    gmock_lib = os.path.join(OPEN_SRC_DIR, 'googlemock', 'lib', config.lib_subdir, 'libgmock.a')
    if not force and os.path.exists(gtest_lib) and os.path.exists(gmock_lib):
        LOG.info('[googletest] libraries already exist for %s, skipping. Use --force to redo.', config.lib_subdir)
        if os.path.isdir(src_dir):
            _copy_gtest_headers(src_dir)
        return

    git_clone(GTEST_TAG, GTEST_REPO_URLS, src_dir, force)

    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    os.makedirs(build_dir)

    cmake_cmd = [
        'cmake', src_dir,
        '-G', config.generator,
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_GMOCK=ON',
        '-DINSTALL_GTEST=OFF',
        '-DBUILD_SHARED_LIBS=OFF',
        '-Dgtest_build_tests=OFF',
        '-Dgmock_build_tests=OFF',
    ]
    cmake_cmd.extend(config.cmake_flags)

    run_cmd(cmake_cmd, cwd=build_dir)
    run_cmd(['cmake', '--build', '.', '-j', str(MAKE_JOBS)], cwd=build_dir)

    _copy_gtest_headers(src_dir)

    gtest_lib_dir = os.path.join(OPEN_SRC_DIR, 'googletest', 'lib', config.lib_subdir)
    gmock_lib_dir = os.path.join(OPEN_SRC_DIR, 'googlemock', 'lib', config.lib_subdir)
    os.makedirs(gtest_lib_dir, exist_ok=True)
    os.makedirs(gmock_lib_dir, exist_ok=True)

    for lib_name in ['libgtest.a', 'libgtest_main.a']:
        src = find_file(build_dir, lib_name)
        if src:
            shutil.copy2(src, gtest_lib_dir)
            LOG.info('[googletest] copied %s -> %s', lib_name, gtest_lib_dir)
        else:
            LOG.error('[googletest] %s not found in build output!', lib_name)
            sys.exit(1)

    for lib_name in ['libgmock.a', 'libgmock_main.a']:
        src = find_file(build_dir, lib_name)
        if src:
            shutil.copy2(src, gmock_lib_dir)
            LOG.info('[googlemock] copied %s -> %s', lib_name, gmock_lib_dir)
        else:
            LOG.error('[googlemock] %s not found in build output!', lib_name)
            sys.exit(1)

    LOG.info('[googletest] done.')


def _copy_libuv_headers(src_dir):
    """Copy libuv headers to open_source."""
    include_src = os.path.join(src_dir, 'include')
    include_dst = os.path.join(OPEN_SRC_DIR, 'libuv', 'include')
    if os.path.isdir(include_src):
        copy_tree(include_src, include_dst)
        LOG.info('[libuv] copied headers')


def prepare_libuv(config, force=False):
    """Clone and build libuv for a build config."""
    LOG.info('[libuv] preparing libuv for %s...', config.name)

    src_dir = os.path.join(DOWNLOAD_DIR, 'libuv')
    build_dir = os.path.join(DEPS_BUILD_DIR, 'libuv_{}'.format(config.lib_subdir))

    libuv_lib = os.path.join(OPEN_SRC_DIR, 'libuv', 'lib', config.lib_subdir, 'libuv.a')
    if not force and os.path.exists(libuv_lib):
        LOG.info('[libuv] library already exists for %s, skipping. Use --force to redo.', config.lib_subdir)
        if os.path.isdir(src_dir):
            _copy_libuv_headers(src_dir)
        return

    git_clone(LIBUV_TAG, LIBUV_REPO_URLS, src_dir, force)

    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    os.makedirs(build_dir)

    cmake_cmd = [
        'cmake', src_dir,
        '-G', config.generator,
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_SHARED_LIBS=OFF',
        '-DBUILD_TESTING=OFF',
    ]
    cmake_cmd.extend(config.cmake_flags)

    run_cmd(cmake_cmd, cwd=build_dir)
    run_cmd(['cmake', '--build', '.', '-j', str(MAKE_JOBS)], cwd=build_dir)

    _copy_libuv_headers(src_dir)

    lib_dir = os.path.join(OPEN_SRC_DIR, 'libuv', 'lib', config.lib_subdir)
    os.makedirs(lib_dir, exist_ok=True)

    src = find_file(build_dir, 'libuv.a') or find_file(build_dir, 'libuv_a.a')
    if src:
        dst = os.path.join(lib_dir, 'libuv.a')
        shutil.copy2(src, dst)
        LOG.info('[libuv] copied libuv.a -> %s', lib_dir)
    else:
        LOG.error('[libuv] libuv.a not found in build output!')
        sys.exit(1)

    LOG.info('[libuv] done.')


def prepare_lldb(force=False):
    """Download lldb API headers from Cangjie llvm-project."""
    LOG.info('[lldb] preparing lldb API headers...')

    target_dir = os.path.join(OPEN_SRC_DIR, 'lldb', 'include', 'lldb')
    if os.path.isdir(target_dir) and os.listdir(target_dir):
        LOG.info('[lldb] headers already exist, skipping.')
        return

    repo_dir = os.path.join(DOWNLOAD_DIR, 'llvm-project')

    need_clone = force or not os.path.isdir(repo_dir)
    if force and os.path.isdir(repo_dir):
        shutil.rmtree(repo_dir)

    if need_clone:
        os.makedirs(os.path.dirname(repo_dir), exist_ok=True)
        clone_cmd = [
            'git', 'clone',
            '--filter=blob:none',
            '--sparse',
            '--depth', '1',
            LLDB_REPO_URL, repo_dir
        ]
        result = run_cmd(clone_cmd, check=False)
        if result.returncode != 0:
            LOG.warning('[lldb] partial clone failed, trying shallow clone...')
            clone_cmd = [
                'git', 'clone',
                '--no-checkout',
                '--depth', '1',
                LLDB_REPO_URL, repo_dir
            ]
            run_cmd(clone_cmd)

    run_cmd(['git', 'sparse-checkout', 'set', 'lldb/include/lldb'], cwd=repo_dir)
    run_cmd(['git', 'checkout'], cwd=repo_dir)

    src_lldb = os.path.join(repo_dir, 'lldb', 'include', 'lldb')
    if os.path.isdir(src_lldb):
        os.makedirs(os.path.dirname(target_dir), exist_ok=True)
        copy_tree(src_lldb, target_dir)
        LOG.info('[lldb] copied lldb headers')
    else:
        LOG.error('[lldb] lldb/include/lldb not found in repo after checkout!')
        sys.exit(1)

    LOG.info('[lldb] done.')


def prepare(args):
    """Prepare all dependencies."""
    setup_build_tools()
    configs = get_build_configs(args.sdk_path, args.target)
    information(configs)
    LOG.info('')

    # Download header-only deps (once, shared across configs)
    prepare_json(force=args.force)
    LOG.info('')
    prepare_lldb(force=args.force)
    LOG.info('')

    # Build static libs for each config
    for config in configs:
        LOG.info('==== Building for config: %s ====', config.name)
        prepare_libuv(config, force=args.force)
        LOG.info('')
        prepare_googletest(config, force=args.force)
        LOG.info('')

    LOG.info('='.ljust(60, '='))
    LOG.info('All dependencies prepared successfully!')
    LOG.info('='.ljust(60, '='))


def clean(args):
    """Clean downloaded and built dependencies."""
    LOG.info('Cleaning dependency work directory: %s', DEPS_DIR)
    if os.path.exists(DEPS_DIR):
        shutil.rmtree(DEPS_DIR, ignore_errors=True)
    LOG.info('Clean done.')


def main():
    """prepare entry"""
    parser = argparse.ArgumentParser(description='Prepare open_source dependencies for dap_server')
    subparsers = parser.add_subparsers(help='sub command help')

    parser_prepare = subparsers.add_parser('prepare', help='download and build dependencies')
    parser_prepare.add_argument('-f',
                                '--force',
                                dest='force',
                                action='store_true',
                                help='redo even if already done')
    parser_prepare.add_argument('--sdk-path',
                                dest='sdk_path',
                                type=str,
                                default=DEFAULT_SDK_PATH,
                                help='Cangjie SDK path (needed for clang on Windows to locate mingw runtime libs).')
    parser_prepare.add_argument('--target',
                                dest='target',
                                type=str,
                                choices=['win_mingw64', 'win_ninja', 'linux_x86_64',
                                         'linux_aarch64', 'darwin_x86_64', 'darwin_aarch64'],
                                default=None,
                                help='prepare dependencies for one DAP target only')
    parser_prepare.set_defaults(func=prepare)

    parser_clean = subparsers.add_parser('clean', help='clean downloaded and built dependencies')
    parser_clean.set_defaults(func=clean)

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        args = parser.parse_args(['prepare'] + sys.argv[1:])

    args.func(args)


if __name__ == '__main__':
    LOG = init_log('prepare')
    os.environ['LANG'] = 'C'
    main()
