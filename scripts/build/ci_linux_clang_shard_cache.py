#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import collections
import hashlib
import io
import json
import os
import pathlib
import stat
import subprocess
import sys

import ci_linux_clang_driver_buckets as shard_config


CACHE_SCHEMA_VERSION = 1
DEFINITION_SCHEMA_VERSION = 1
CACHE_KEY_PREFIX = 'mame-linux-clang-shard-v1'
CHUNK_SIZE = 1024 * 1024

COMMON_INPUT_PATHS = (
        '.github/workflows/ci-linux.yml',
        'makefile',
        '3rdparty',
        'scripts',
        'src/devices',
        'src/emu',
        'src/frontend',
        'src/lib',
        'src/osd')

TRANSLATION_UNIT_SUFFIXES = frozenset(('.c', '.cc', '.cpp', '.cxx', '.m', '.mm'))


def add_solution_arguments(parser):
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument(
            '--group', required=True,
            choices=tuple(shard_config.LIBRARY_GROUPS) + tuple(shard_config.BUCKETS))
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', required=True)

    prepare_parser = subparsers.add_parser('prepare')
    add_solution_arguments(prepare_parser)
    prepare_parser.add_argument('--toolchain-file', required=True, type=pathlib.Path)
    prepare_parser.add_argument('--manifest', required=True, type=pathlib.Path)
    prepare_parser.add_argument('--source-commit', required=True)
    prepare_parser.add_argument('--runner-os', required=True)
    prepare_parser.add_argument('--runner-arch', required=True)

    verify_parser = subparsers.add_parser('verify')
    add_solution_arguments(verify_parser)
    verify_parser.add_argument('--expected-manifest', required=True, type=pathlib.Path)
    verify_parser.add_argument('--cache-root', required=True, type=pathlib.Path)

    return parser.parse_args()


def canonical_json(value):
    return (json.dumps(value, ensure_ascii=True, separators=(',', ':'), sort_keys=True) + '\n').encode('utf-8')


def canonical_hash(value):
    return hashlib.sha256(canonical_json(value)).hexdigest()


def load_projects(solution_makefile, source_root, group):
    projects = shard_config.read_solution_projects(solution_makefile)
    generated_drivers = shard_config.validate_buckets(projects, source_root)
    return sorted(shard_config.select_projects(group, projects, generated_drivers))


def shard_definition(group, projects):
    pool = 'libraries' if group in shard_config.LIBRARY_GROUPS else 'drivers'
    return {
            'outputs': ['lib%s.a' % project for project in projects],
            'partition': 'manual-v1',
            'pool': pool,
            'schema_version': DEFINITION_SCHEMA_VERSION,
            'shard': group,
            'targets': projects}


def is_under(relative_path, root):
    return relative_path == root or relative_path.startswith(root.rstrip('/') + '/')


def tracked_files(source_root):
    try:
        output = subprocess.check_output(
                ('git', '-c', 'safe.directory=%s' % source_root.as_posix(),
                 '-C', str(source_root), 'ls-files', '-z'),
                stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as error:
        details = error.output.decode('utf-8', errors='replace').strip()
        raise ValueError('unable to list tracked cache inputs: %s' % details)
    except OSError as error:
        raise ValueError('unable to list tracked cache inputs: %s' % error)

    result = {}
    for encoded_path in output.split(b'\0'):
        if not encoded_path:
            continue
        relative_path = encoded_path.decode('utf-8')
        path = source_root.joinpath(*relative_path.split('/'))
        if not path.is_symlink() and not path.is_file():
            raise ValueError('tracked cache input is not a file: %s' % relative_path)
        result[relative_path] = path
    return result


def collect_input_files(source_root, group, projects):
    tracked = tracked_files(source_root)
    files = {}
    selectors = list(COMMON_INPUT_PATHS)
    matched_common = dict((relative_path, False) for relative_path in COMMON_INPUT_PATHS)
    for relative_path, path in tracked.items():
        for root in COMMON_INPUT_PATHS:
            if is_under(relative_path, root):
                files[relative_path] = path
                matched_common[root] = True
                break
    missing_common = sorted(root for root, matched in matched_common.items() if not matched)
    if missing_common:
        raise ValueError('cache input does not contain tracked files: %s' % ' '.join(missing_common))

    if group in shard_config.BUCKETS:
        # Driver projects own translation units in their directory, but may
        # include headers from anywhere under src/mame.
        selectors.append('src/mame/** excluding translation units')
        project_roots = ['src/mame/%s' % project for project in projects]
        selectors.extend(project_roots)
        matched_projects = dict((root, False) for root in project_roots)
        for relative_path, path in tracked.items():
            if is_under(relative_path, 'src/mame'):
                suffix = pathlib.PurePosixPath(relative_path).suffix.lower()
                if suffix not in TRANSLATION_UNIT_SUFFIXES:
                    files[relative_path] = path
                for root in project_roots:
                    if is_under(relative_path, root):
                        files[relative_path] = path
                        matched_projects[root] = True
                        break
        missing_projects = sorted(root for root, matched in matched_projects.items() if not matched)
        if missing_projects:
            raise ValueError('driver cache input does not contain tracked files: %s' % ' '.join(missing_projects))

    return files, sorted(set(selectors))


def hash_file(path):
    digest = hashlib.sha256()
    with open(str(path), 'rb') as source_file:
        while True:
            block = source_file.read(CHUNK_SIZE)
            if not block:
                break
            digest.update(block)
    return digest.hexdigest()


def hash_inputs(files):
    digest = hashlib.sha256()
    for relative_path in sorted(files):
        path = files[relative_path]
        if path.is_symlink():
            record = {
                    'path': relative_path,
                    'target': os.readlink(str(path)),
                    'type': 'symlink'}
        else:
            record = {
                    'executable': bool(path.stat().st_mode & (stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)),
                    'hash': hash_file(path),
                    'path': relative_path,
                    'type': 'file'}
        digest.update(canonical_json(record))
    return digest.hexdigest()


def read_toolchain(path):
    with io.open(path, 'r', encoding='utf-8') as toolchain_file:
        value = toolchain_file.read()
    return value.replace('\r\n', '\n').replace('\r', '\n').rstrip('\n') + '\n'


def build_identity(runner_os, runner_arch, toolchain):
    return {
            'architecture': 'x64',
            'compiler': 'clang',
            'configuration': 'release64',
            'make_jobs': 3,
            'options': {
                    'OSD': 'sdl',
                    'OVERRIDE_CC': 'clang',
                    'OVERRIDE_CXX': 'clang++',
                    'PRECOMPILE': '1',
                    'SUBTARGET': 'mame',
                    'TARGET': 'mame',
                    'TOOLS': '1'},
            'runner_arch': runner_arch,
            'runner_os': runner_os,
            'target_os': 'linux',
            'toolchain': toolchain}


def write_manifest(path, manifest):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(str(path), 'wb') as manifest_file:
        manifest_file.write(canonical_json(manifest))


def read_manifest(path):
    with io.open(path, 'r', encoding='utf-8') as manifest_file:
        value = json.load(manifest_file)
    if not isinstance(value, dict):
        raise ValueError('%s: expected a JSON object' % path)
    return value


def prepare_cache(options):
    source_root = options.source_root.resolve()
    projects = load_projects(options.solution_makefile, source_root, options.group)
    definition = shard_definition(options.group, projects)
    definition_hash = canonical_hash(definition)
    files, selectors = collect_input_files(source_root, options.group, projects)
    inputs_hash = hash_inputs(files)
    identity = build_identity(options.runner_os, options.runner_arch, read_toolchain(options.toolchain_file))
    identity_hash = canonical_hash(identity)
    cache_key = '%s-%s-%s-%s-%s' % (
            CACHE_KEY_PREFIX, options.group, definition_hash, identity_hash, inputs_hash)
    # The commit is audit metadata; relevant tracked content determines reuse.
    manifest = {
            'build': identity,
            'build_hash': identity_hash,
            'cache_key': cache_key,
            'definition': definition,
            'definition_hash': definition_hash,
            'input_file_count': len(files),
            'input_hash': inputs_hash,
            'input_selectors': selectors,
            'schema_version': CACHE_SCHEMA_VERSION,
            'source_commit': options.source_commit}
    write_manifest(options.manifest, manifest)
    return cache_key


def verify_manifest(expected, actual, actual_path):
    fields = (
            'build',
            'build_hash',
            'cache_key',
            'definition',
            'definition_hash',
            'input_file_count',
            'input_hash',
            'input_selectors',
            'schema_version')
    mismatches = [field for field in fields if expected.get(field) != actual.get(field)]
    if mismatches:
        raise ValueError('%s: cache manifest mismatch: %s' % (actual_path, ' '.join(mismatches)))


def verify_archives(cache_root, outputs):
    if not cache_root.is_dir():
        raise ValueError('%s: shard cache directory does not exist' % cache_root)

    expected = set(outputs)
    archives = list(cache_root.rglob('lib*.a'))
    names = [path.name for path in archives]
    actual = set(names)
    missing = sorted(expected - actual)
    unexpected = sorted(actual - expected)
    duplicates = sorted(name for name, count in collections.Counter(names).items() if count != 1)
    empty = sorted(path.as_posix() for path in archives if not path.stat().st_size)
    if missing or unexpected or duplicates or empty:
        details = []
        if missing:
            details.append('missing archives: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected archives: %s' % ' '.join(unexpected))
        if duplicates:
            details.append('duplicate archives: %s' % ' '.join(duplicates))
        if empty:
            details.append('empty archives: %s' % ' '.join(empty))
        raise ValueError('; '.join(details))


def verify_cache(options):
    source_root = options.source_root.resolve()
    projects = load_projects(options.solution_makefile, source_root, options.group)
    expected = read_manifest(options.expected_manifest)
    actual_path = options.cache_root / ('shard-manifest-%s.json' % options.group)
    actual = read_manifest(actual_path)
    verify_manifest(expected, actual, actual_path)

    definition = shard_definition(options.group, projects)
    if actual.get('definition') != definition:
        raise ValueError('%s: manifest does not match generated project selection' % actual_path)
    verify_archives(options.cache_root, definition['outputs'])
    return len(projects), actual.get('source_commit', 'unknown')


def main():
    options = parse_args()
    try:
        if options.command == 'prepare':
            sys.stdout.write('%s\n' % prepare_cache(options))
        else:
            project_count, source_commit = verify_cache(options)
            sys.stdout.write('Validated %d cached archives for %s from %s.\n' % (
                    project_count, options.group, source_commit))
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error managing CI shard cache: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
