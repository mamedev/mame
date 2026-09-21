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


CACHE_SCHEMA_VERSION = 2
DEPENDENCY_SCHEMA_VERSION = 1
DEFINITION_SCHEMA_VERSION = 1
CACHE_KEY_PREFIX = 'mame-linux-clang-shard-v2'
DEPENDENCY_KEY_PREFIX = 'mame-linux-clang-dependencies-v1'
CHUNK_SIZE = 1024 * 1024

BROAD_INPUT_PATHS = (
        '.github/workflows/ci-linux.yml',
        'makefile',
        '3rdparty',
        'scripts',
        'src/devices',
        'src/emu',
        'src/frontend',
        'src/lib',
        'src/osd')

BUILD_INPUT_PATHS = (
        '.github/workflows/ci-linux.yml',
        'makefile',
        '3rdparty/genie',
        'scripts')

TRANSLATION_UNIT_SUFFIXES = frozenset(('.c', '.cc', '.cpp', '.cxx', '.m', '.mm'))


def add_solution_arguments(parser):
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument(
            '--group', required=True,
            choices=tuple(shard_config.LIBRARY_GROUPS) + shard_config.driver_groups())
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))


def add_build_arguments(parser):
    add_solution_arguments(parser)
    parser.add_argument('--toolchain-file', required=True, type=pathlib.Path)
    parser.add_argument('--runner-os', required=True)
    parser.add_argument('--runner-arch', required=True)


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', required=True)

    context_parser = subparsers.add_parser('context')
    add_build_arguments(context_parser)

    prepare_parser = subparsers.add_parser('prepare')
    add_build_arguments(prepare_parser)
    prepare_parser.add_argument('--manifest', required=True, type=pathlib.Path)
    prepare_parser.add_argument('--source-commit', required=True)
    prepare_parser.add_argument('--dependency-manifest', type=pathlib.Path)
    prepare_parser.add_argument('--require-dependency-manifest', action='store_true')

    capture_parser = subparsers.add_parser('capture')
    add_build_arguments(capture_parser)
    capture_parser.add_argument('--dependency-root', required=True, type=pathlib.Path)
    capture_parser.add_argument('--manifest', required=True, type=pathlib.Path)
    capture_parser.add_argument('--source-commit', required=True)

    verify_parser = subparsers.add_parser('verify')
    add_solution_arguments(verify_parser)
    verify_parser.add_argument('--expected-manifest', required=True, type=pathlib.Path)
    verify_parser.add_argument('--cache-root', required=True, type=pathlib.Path)

    return parser.parse_args()


def canonical_json(value):
    return (json.dumps(value, ensure_ascii=True, separators=(',', ':'), sort_keys=True) + '\n').encode('utf-8')


def canonical_hash(value):
    return hashlib.sha256(canonical_json(value)).hexdigest()


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
        result[relative_path] = source_root.joinpath(*relative_path.split('/'))
    return result


def require_input_file(relative_path, path):
    if not path.is_symlink() and not path.is_file():
        raise ValueError('cache input is not a file: %s' % relative_path)


def collect_tracked_roots(tracked, roots):
    files = {}
    matched = dict((root, False) for root in roots)
    for relative_path, path in tracked.items():
        for root in roots:
            if is_under(relative_path, root):
                require_input_file(relative_path, path)
                files[relative_path] = path
                matched[root] = True
                break
    missing = sorted(root for root, found in matched.items() if not found)
    if missing:
        raise ValueError('cache input does not contain tracked files: %s' % ' '.join(missing))
    return files


def collect_broad_input_files(tracked, group, projects):
    files = collect_tracked_roots(tracked, BROAD_INPUT_PATHS)
    selectors = list(BROAD_INPUT_PATHS)
    if group in shard_config.driver_groups():
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
                    require_input_file(relative_path, path)
                    files[relative_path] = path
                for root in project_roots:
                    if is_under(relative_path, root):
                        require_input_file(relative_path, path)
                        files[relative_path] = path
                        matched_projects[root] = True
                        break
        missing_projects = sorted(root for root, found in matched_projects.items() if not found)
        if missing_projects:
            raise ValueError(
                    'driver cache input does not contain tracked files: %s' %
                    ' '.join(missing_projects))
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
        require_input_file(relative_path, path)
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


def repository_file(source_root, path):
    resolved = path.resolve()
    try:
        relative_path = resolved.relative_to(source_root).as_posix()
    except ValueError:
        raise ValueError('%s: expected a path below %s' % (path, source_root))
    require_input_file(relative_path, resolved)
    return relative_path, resolved


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


def load_projects(solution_makefile, source_root, group):
    shard_map = shard_config.load_shard_map()
    projects = shard_config.read_solution_projects(solution_makefile)
    generated_drivers = shard_config.validate_buckets(projects, source_root, shard_map)
    return sorted(shard_config.select_projects(group, projects, generated_drivers, shard_map))


def shard_definition(group, projects):
    if group in shard_config.LIBRARY_GROUPS:
        return {
                'outputs': ['lib%s.a' % project for project in projects],
                'partition': 'manual-v1',
                'pool': 'libraries',
                'schema_version': DEFINITION_SCHEMA_VERSION,
                'shard': group,
                'targets': projects}

    shard_map = shard_config.load_shard_map()
    checked = shard_config.find_driver_shard(shard_map, group)
    definition = shard_config.driver_shard_definition(projects)
    if definition['targets'] != checked['targets']:
        raise ValueError('%s: cache definition does not match checked shard map' % group)
    if canonical_hash(definition) != checked['definition_hash']:
        raise ValueError('%s: cache definition hash does not match checked shard map' % group)
    return definition


def project_shape(solution_makefile, source_root, projects):
    files = {}
    for project in projects:
        relative_path, path = repository_file(
                source_root, solution_makefile.parent / ('%s.make' % project))
        files[relative_path] = path
    return sorted(files), hash_inputs(files)


def build_context(options):
    source_root = options.source_root.resolve()
    solution_makefile = options.solution_makefile.resolve()
    projects = load_projects(solution_makefile, source_root, options.group)
    definition = shard_definition(options.group, projects)
    identity = build_identity(
            options.runner_os, options.runner_arch,
            read_toolchain(options.toolchain_file))
    project_makefiles, shape_hash = project_shape(solution_makefile, source_root, projects)
    return {
            'build': identity,
            'build_hash': canonical_hash(identity),
            'definition': definition,
            'definition_hash': canonical_hash(definition),
            'group': options.group,
            'project_makefiles': project_makefiles,
            'project_shape_hash': shape_hash,
            'projects': projects,
            'solution_makefile': solution_makefile,
            'source_root': source_root}


def dependency_cache_prefix(options):
    context = build_context(options)
    return '%s-%s-%s-%s-' % (
            DEPENDENCY_KEY_PREFIX,
            context['group'],
            context['definition_hash'],
            context['build_hash'])


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


def validate_relative_path(value, description):
    if not isinstance(value, str) or not value:
        raise ValueError('%s must be a non-empty string' % description)
    if '\\' in value:
        raise ValueError('%s is not normalized: %s' % (description, value))
    path = pathlib.PurePosixPath(value)
    if not path.parts or path.is_absolute() or path.as_posix() != value or '..' in path.parts:
        raise ValueError('%s is not a repository-relative path: %s' % (description, value))
    if path.parts[0] == '.git':
        raise ValueError('%s may not refer to Git metadata: %s' % (description, value))
    return value


def dependency_manifest_identity(manifest):
    return {
            'build_hash': manifest.get('build_hash'),
            'definition_hash': manifest.get('definition_hash'),
            'dependencies': manifest.get('dependencies'),
            'schema_version': manifest.get('schema_version'),
            'shard': manifest.get('shard')}


def validate_dependency_manifest(manifest, context, expected_shape_hash=None):
    if manifest.get('schema_version') != DEPENDENCY_SCHEMA_VERSION:
        raise ValueError('dependency manifest schema does not match')
    if manifest.get('shard') != context['group']:
        raise ValueError('dependency manifest shard does not match')
    if manifest.get('definition_hash') != context['definition_hash']:
        raise ValueError('dependency manifest shard definition does not match')
    if manifest.get('build_hash') != context['build_hash']:
        raise ValueError('dependency manifest build identity does not match')
    if expected_shape_hash is not None:
        if manifest.get('project_shape_hash') != expected_shape_hash:
            raise ValueError('dependency manifest project shape does not match')

    dependency_root = validate_relative_path(
            manifest.get('dependency_root'), 'dependency root')
    dependencies = manifest.get('dependencies')
    if not isinstance(dependencies, list) or not dependencies:
        raise ValueError('dependency manifest contains no dependencies')
    if dependencies != sorted(set(dependencies)):
        raise ValueError('dependency manifest paths are not sorted and unique')
    for relative_path in dependencies:
        validate_relative_path(relative_path, 'dependency path')
        if is_under(relative_path, dependency_root):
            raise ValueError('dependency path refers to a compiled intermediate: %s' % relative_path)
        path = context['source_root'].joinpath(*relative_path.split('/'))
        require_input_file(relative_path, path)

    dependency_files = manifest.get('dependency_files')
    if not isinstance(dependency_files, list) or not dependency_files:
        raise ValueError('dependency manifest contains no compiler dependency files')
    if dependency_files != sorted(set(dependency_files)):
        raise ValueError('compiler dependency file paths are not sorted and unique')
    for relative_path in dependency_files:
        validate_relative_path(relative_path, 'compiler dependency file')
        if not is_under(relative_path, dependency_root) or not relative_path.endswith('.d'):
            raise ValueError('compiler dependency file is outside its root: %s' % relative_path)

    if manifest.get('dependency_count') != len(dependencies):
        raise ValueError('dependency manifest path count does not match')
    if manifest.get('dependency_file_count') != len(dependency_files):
        raise ValueError('compiler dependency file count does not match')
    if manifest.get('manifest_hash') != canonical_hash(dependency_manifest_identity(manifest)):
        raise ValueError('dependency manifest hash does not match')
    return dependencies


def dependency_manifest_for_key(path, context, required):
    if path is None or not path.is_file():
        if required:
            raise ValueError('required dependency manifest is missing')
        return None, 'missing'
    try:
        manifest = read_manifest(path)
        validate_dependency_manifest(manifest, context)
        return manifest, 'available'
    except (IOError, OSError, UnicodeError, ValueError) as error:
        if required:
            raise
        sys.stderr.write(
                'Dependency manifest is unusable; using conservative inputs: %s\n' % error)
        return None, 'invalid'


def collect_precise_input_files(context, tracked, dependency_manifest):
    files = collect_tracked_roots(tracked, BUILD_INPUT_PATHS)
    for relative_path in dependency_manifest['dependencies']:
        path = context['source_root'].joinpath(*relative_path.split('/'))
        require_input_file(relative_path, path)
        files[relative_path] = path
    selectors = list(BUILD_INPUT_PATHS)
    selectors.append('dependency-manifest:%s' % dependency_manifest['manifest_hash'])
    return files, sorted(selectors)


def prepare_cache(options):
    context = build_context(options)
    tracked = tracked_files(context['source_root'])
    dependency_manifest, dependency_status = dependency_manifest_for_key(
            options.dependency_manifest,
            context,
            options.require_dependency_manifest)

    if dependency_manifest is None:
        files, selectors = collect_broad_input_files(
                tracked, context['group'], context['projects'])
        input_mode = 'conservative'
        dependency_hash = None
        dependency_commit = None
    else:
        files, selectors = collect_precise_input_files(context, tracked, dependency_manifest)
        input_mode = 'dependencies'
        dependency_hash = dependency_manifest['manifest_hash']
        dependency_commit = dependency_manifest.get('source_commit', 'unknown')

    files_hash = hash_inputs(files)
    inputs_hash = canonical_hash({
            'dependency_manifest_hash': dependency_hash,
            'files_hash': files_hash,
            'mode': input_mode})
    cache_key = '%s-%s-%s-%s-%s-%s' % (
            CACHE_KEY_PREFIX,
            context['group'],
            context['definition_hash'],
            context['build_hash'],
            context['project_shape_hash'],
            inputs_hash)
    manifest = {
            'build': context['build'],
            'build_hash': context['build_hash'],
            'cache_key': cache_key,
            'definition': context['definition'],
            'definition_hash': context['definition_hash'],
            'dependency_manifest_hash': dependency_hash,
            'dependency_manifest_status': dependency_status,
            'dependency_source_commit': dependency_commit,
            'input_file_count': len(files),
            'input_files_hash': files_hash,
            'input_hash': inputs_hash,
            'input_mode': input_mode,
            'input_selectors': selectors,
            'project_makefiles': context['project_makefiles'],
            'project_shape_hash': context['project_shape_hash'],
            'schema_version': CACHE_SCHEMA_VERSION,
            # The commit is audit metadata; relevant content determines reuse.
            'source_commit': options.source_commit}
    write_manifest(options.manifest, manifest)
    return cache_key


def first_make_rule(text, path):
    logical = []
    index = 0
    while index < len(text):
        character = text[index]
        if character == '\\' and (index + 1) < len(text):
            if text[index + 1] == '\n':
                logical.append(' ')
                index += 2
                continue
            if text[index + 1] == '\r' and (index + 2) < len(text) and text[index + 2] == '\n':
                logical.append(' ')
                index += 3
                continue
        if character in ('\r', '\n'):
            break
        logical.append(character)
        index += 1
    line = ''.join(logical)

    escaped = False
    for index, character in enumerate(line):
        if escaped:
            escaped = False
        elif character == '\\':
            escaped = True
        elif character == ':':
            return line[index + 1:]
    raise ValueError('%s: compiler dependency file contains no rule' % path)


def parse_make_words(value, path):
    words = []
    word = []
    index = 0
    while index < len(value):
        character = value[index]
        if character == '\\':
            index += 1
            if index >= len(value):
                raise ValueError('%s: unterminated escape in dependency rule' % path)
            word.append(value[index])
        elif character == '$' and (index + 1) < len(value) and value[index + 1] == '$':
            word.append('$')
            index += 1
        elif character.isspace():
            if word:
                words.append(''.join(word))
                word = []
        else:
            word.append(character)
        index += 1
    if word:
        words.append(''.join(word))
    return words


def read_compiler_dependencies(path):
    with io.open(path, 'r', encoding='utf-8') as dependency_file:
        text = dependency_file.read()
    dependencies = parse_make_words(first_make_rule(text, path), path)
    if not dependencies:
        raise ValueError('%s: compiler dependency rule contains no prerequisites' % path)
    return dependencies


def normalize_compiler_dependency(token, context, dependency_root):
    path = pathlib.Path(token)
    if not path.is_absolute():
        path = context['solution_makefile'].parent / path
    resolved = path.resolve()
    try:
        relative_path = resolved.relative_to(context['source_root']).as_posix()
    except ValueError:
        return 'external', token

    if is_under(relative_path, dependency_root):
        return 'intermediate', relative_path
    require_input_file(relative_path, resolved)
    return 'dependency', relative_path


def collect_compiler_dependencies(dependency_files, context, dependency_root):
    dependencies = set()
    external = set()
    intermediate = set()
    for dependency_file in dependency_files:
        for token in read_compiler_dependencies(dependency_file):
            category, value = normalize_compiler_dependency(token, context, dependency_root)
            if category == 'dependency':
                dependencies.add(value)
            elif category == 'external':
                external.add(value)
            else:
                intermediate.add(value)
    return sorted(dependencies), len(external), len(intermediate)


def capture_dependencies(options):
    context = build_context(options)
    dependency_root = options.dependency_root.resolve()
    try:
        relative_root = dependency_root.relative_to(context['source_root']).as_posix()
    except ValueError:
        raise ValueError('%s: dependency root must be below the source root' % dependency_root)
    if not dependency_root.is_dir():
        raise ValueError('%s: compiler dependency root does not exist' % dependency_root)

    dependency_files = sorted(path for path in dependency_root.rglob('*.d') if path.is_file())
    if not dependency_files:
        raise ValueError('%s: no compiler dependency files found' % dependency_root)
    relative_files = [path.relative_to(context['source_root']).as_posix() for path in dependency_files]
    dependencies, external_count, intermediate_count = collect_compiler_dependencies(
            dependency_files, context, relative_root)
    if not dependencies:
        raise ValueError('%s: compiler dependency files contain no repository inputs' % dependency_root)

    manifest = {
            'build_hash': context['build_hash'],
            'definition_hash': context['definition_hash'],
            'dependencies': dependencies,
            'dependency_count': len(dependencies),
            'dependency_file_count': len(relative_files),
            'dependency_files': relative_files,
            'dependency_root': relative_root,
            'ignored_external_count': external_count,
            'ignored_intermediate_count': intermediate_count,
            'project_shape_hash': context['project_shape_hash'],
            'schema_version': DEPENDENCY_SCHEMA_VERSION,
            'shard': context['group'],
            'source_commit': options.source_commit}
    manifest['manifest_hash'] = canonical_hash(dependency_manifest_identity(manifest))
    write_manifest(options.manifest, manifest)
    return len(relative_files), len(dependencies)


def verify_manifest(expected, actual, actual_path):
    fields = (
            'build',
            'build_hash',
            'cache_key',
            'definition',
            'definition_hash',
            'dependency_manifest_hash',
            'input_file_count',
            'input_files_hash',
            'input_hash',
            'input_mode',
            'input_selectors',
            'project_makefiles',
            'project_shape_hash',
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


def verify_dependency_payload(cache_root, expected_manifest, context):
    manifest_path = cache_root / ('dependency-manifest-%s.json' % context['group'])
    manifest = read_manifest(manifest_path)
    validate_dependency_manifest(
            manifest,
            context,
            expected_shape_hash=expected_manifest['project_shape_hash'])
    if expected_manifest.get('input_mode') != 'dependencies':
        raise ValueError('cache payload was not keyed from compiler dependencies')
    if expected_manifest.get('dependency_manifest_hash') != manifest.get('manifest_hash'):
        raise ValueError('%s: dependency manifest does not match cache key' % manifest_path)

    expected_files = set(manifest['dependency_files'])
    actual_paths = list(cache_root.rglob('*.d'))
    actual_files = set(path.relative_to(cache_root).as_posix() for path in actual_paths)
    missing = sorted(expected_files - actual_files)
    unexpected = sorted(actual_files - expected_files)
    empty = sorted(path.relative_to(cache_root).as_posix() for path in actual_paths if not path.stat().st_size)
    if missing or unexpected or empty:
        details = []
        if missing:
            details.append('missing compiler dependencies: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected compiler dependencies: %s' % ' '.join(unexpected))
        if empty:
            details.append('empty compiler dependencies: %s' % ' '.join(empty))
        raise ValueError('; '.join(details))
    return len(actual_files)


def verify_cache(options):
    source_root = options.source_root.resolve()
    solution_makefile = options.solution_makefile.resolve()
    projects = load_projects(solution_makefile, source_root, options.group)
    expected = read_manifest(options.expected_manifest)
    actual_path = options.cache_root / ('shard-manifest-%s.json' % options.group)
    actual = read_manifest(actual_path)
    verify_manifest(expected, actual, actual_path)

    definition = shard_definition(options.group, projects)
    if actual.get('definition') != definition:
        raise ValueError('%s: manifest does not match generated project selection' % actual_path)
    project_makefiles, shape_hash = project_shape(solution_makefile, source_root, projects)
    if actual.get('project_makefiles') != project_makefiles:
        raise ValueError('%s: manifest does not match generated project makefiles' % actual_path)
    if actual.get('project_shape_hash') != shape_hash:
        raise ValueError('%s: manifest does not match generated project shape' % actual_path)

    verify_archives(options.cache_root, definition['outputs'])
    context = {
            'build_hash': expected['build_hash'],
            'definition_hash': expected['definition_hash'],
            'group': options.group,
            'source_root': source_root}
    dependency_count = verify_dependency_payload(options.cache_root, expected, context)
    return len(projects), dependency_count, actual.get('source_commit', 'unknown')


def main():
    options = parse_args()
    try:
        if options.command == 'context':
            sys.stdout.write('%s\n' % dependency_cache_prefix(options))
        elif options.command == 'prepare':
            sys.stdout.write('%s\n' % prepare_cache(options))
        elif options.command == 'capture':
            file_count, dependency_count = capture_dependencies(options)
            sys.stdout.write('Captured %d compiler dependency files with %d inputs for %s.\n' % (
                    file_count, dependency_count, options.group))
        else:
            project_count, dependency_count, source_commit = verify_cache(options)
            sys.stdout.write(
                    'Validated %d cached archives and %d compiler dependency files for %s from %s.\n' % (
                            project_count, dependency_count, options.group, source_commit))
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error managing CI shard cache: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
