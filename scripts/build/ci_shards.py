#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import collections
import csv
import decimal
import hashlib
import html
import io
import json
import os
import pathlib
import re
import shutil
import stat
import subprocess
import sys


CACHE_SCHEMA_VERSION = 3
DEPENDENCY_SCHEMA_VERSION = 2
DEFINITION_SCHEMA_VERSION = 1
CACHE_KEY_PREFIX = 'mame-ci-shard-v3'
DEPENDENCY_KEY_PREFIX = 'mame-ci-dependencies-v2'
PROFILE_SCHEMA_VERSION = 2
DRIVER_MAP_SCHEMA_VERSION = 1
PARTITION_ALGORITHM = 'lpt-v1'
DRIVER_POOL = 'driver-libs'
INVENTORY_SCHEMA_VERSION = 1
DRIFT_SCHEMA_VERSION = 2
FALLBACK_SCHEMA_VERSION = 1
REBALANCE_THRESHOLD = decimal.Decimal('0.05')
MILLISECOND = decimal.Decimal('0.001')
CHUNK_SIZE = 1024 * 1024

BROAD_INPUT_PATHS = (
        'makefile',
        '3rdparty',
        'scripts',
        'src/devices',
        'src/emu',
        'src/frontend',
        'src/lib',
        'src/osd')

BUILD_INPUT_PATHS = (
        'makefile',
        '3rdparty/genie',
        'scripts')

TRANSLATION_UNIT_SUFFIXES = frozenset(('.c', '.cc', '.cpp', '.cxx', '.m', '.mm'))
PROJECTS_PREFIX = 'PROJECTS := '
MAKE_ASSIGNMENT_PATTERN = re.compile(
        r'^\s*(?:override\s+)?(TARGETDIR|TARGET|LIBDEPS|LINKCMD)\s*'
        r'(\+=|:=|\?=|=)\s*(.*)$')
MAKE_CONDITIONAL_PATTERN = re.compile(
        r'^(?:ifeq|ifneq|ifdef|ifndef)(?:\s|\()')
MAKE_VARIABLE_PATTERN = re.compile(r'\$\(([^()]+)\)|\$\{([^{}]+)\}')


class MonolithicFallbackRequired(ValueError):
    pass


def add_profile_arguments(parser):
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument('--profile', required=True, type=pathlib.Path)
    parser.add_argument('--group', required=True)
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))


def add_build_arguments(parser):
    add_profile_arguments(parser)
    parser.add_argument('--toolchain-file', required=True, type=pathlib.Path)
    parser.add_argument('--runner-os', required=True)
    parser.add_argument('--runner-arch', required=True)


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', required=True)

    matrix_parser = subparsers.add_parser('matrix')
    matrix_parser.add_argument('--profile', action='append', type=pathlib.Path)
    matrix_parser.add_argument('--profiles-json')
    matrix_parser.add_argument('--kind', required=True, choices=('shards', 'profiles'))

    github_env_parser = subparsers.add_parser('github-env')
    github_env_parser.add_argument('--profile', required=True, type=pathlib.Path)
    github_env_parser.add_argument('--group', required=True)
    github_env_parser.add_argument('--expected-profile-id', required=True)
    github_env_parser.add_argument('--expected-runner', required=True)
    github_env_parser.add_argument('--expected-shell', required=True)
    github_env_parser.add_argument('--expected-install-kind', required=True)
    github_env_parser.add_argument('--expected-msystem', required=True)
    github_env_parser.add_argument('--expected-packages', required=True)
    github_env_parser.add_argument('--expected-python', required=True)
    github_env_parser.add_argument('--github-output', type=pathlib.Path)
    github_env_parser.add_argument('--github-env', type=pathlib.Path)

    validate_parser = subparsers.add_parser('validate')
    add_profile_arguments(validate_parser)

    inventory_parser = subparsers.add_parser('inventory')
    inventory_parser.add_argument('solution_makefile', type=pathlib.Path)
    inventory_parser.add_argument('--profile', required=True, type=pathlib.Path)
    inventory_parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))
    inventory_parser.add_argument('--inventory-output', required=True, type=pathlib.Path)
    inventory_parser.add_argument('--drift-output', required=True, type=pathlib.Path)
    inventory_parser.add_argument('--summary-output', type=pathlib.Path)
    inventory_parser.add_argument('--github-output', type=pathlib.Path)

    reconcile_parser = subparsers.add_parser('reconcile')
    reconcile_parser.add_argument('--profile', required=True, type=pathlib.Path)
    reconcile_parser.add_argument('--inventory', required=True, type=pathlib.Path)
    reconcile_parser.add_argument('--output', required=True, type=pathlib.Path)
    reconcile_parser.add_argument('--drift-output', required=True, type=pathlib.Path)
    reconcile_parser.add_argument('--fallback-marker', required=True, type=pathlib.Path)
    reconcile_parser.add_argument('--summary-output', type=pathlib.Path)
    reconcile_parser.add_argument('--github-output', type=pathlib.Path)

    list_parser = subparsers.add_parser('list')
    add_profile_arguments(list_parser)

    archives_parser = subparsers.add_parser('verify-archives')
    add_profile_arguments(archives_parser)
    archives_parser.add_argument('--archive-dir', required=True, type=pathlib.Path)

    definition_parser = subparsers.add_parser('write-definition')
    add_profile_arguments(definition_parser)
    definition_parser.add_argument('--output', required=True, type=pathlib.Path)

    definitions_parser = subparsers.add_parser('verify-definitions')
    definitions_parser.add_argument('solution_makefile', type=pathlib.Path)
    definitions_parser.add_argument('--profile', required=True, type=pathlib.Path)
    definitions_parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))
    definitions_parser.add_argument('--manifest-dir', required=True, type=pathlib.Path)

    fallback_parser = subparsers.add_parser('verify-fallback')
    fallback_parser.add_argument('--profile', required=True, type=pathlib.Path)
    fallback_parser.add_argument('--manifest-dir', required=True, type=pathlib.Path)
    fallback_parser.add_argument('--expected-marker', type=pathlib.Path)

    rebalance_parser = subparsers.add_parser('rebalance')
    rebalance_parser.add_argument('solution_makefile', type=pathlib.Path)
    rebalance_parser.add_argument('--profile', required=True, type=pathlib.Path)
    rebalance_parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))
    rebalance_parser.add_argument('--timing-csv', action='append', required=True, type=pathlib.Path)
    rebalance_parser.add_argument('--source-run', action='append', required=True)
    rebalance_parser.add_argument('--identity', action='append', required=True, metavar='NAME=VALUE')
    rebalance_parser.add_argument('--jobs', required=True, type=int)
    rebalance_parser.add_argument('--output', required=True, type=pathlib.Path)
    rebalance_parser.add_argument('--force', action='store_true')

    context_parser = subparsers.add_parser('context')
    add_build_arguments(context_parser)

    prepare_parser = subparsers.add_parser('prepare')
    add_build_arguments(prepare_parser)
    prepare_parser.add_argument('--manifest', required=True, type=pathlib.Path)
    prepare_parser.add_argument('--source-commit', required=True)
    prepare_parser.add_argument('--dependency-manifest', type=pathlib.Path)
    prepare_parser.add_argument('--require-dependency-manifest', action='store_true')

    materialize_parser = subparsers.add_parser('materialize')
    add_build_arguments(materialize_parser)
    materialize_parser.add_argument('--dependency-manifest', required=True, type=pathlib.Path)
    materialize_parser.add_argument('--make-program', default='make')

    capture_parser = subparsers.add_parser('capture')
    add_build_arguments(capture_parser)
    capture_parser.add_argument('--dependency-root', required=True, type=pathlib.Path)
    capture_parser.add_argument('--manifest', required=True, type=pathlib.Path)
    capture_parser.add_argument('--source-commit', required=True)

    stage_parser = subparsers.add_parser('stage')
    add_profile_arguments(stage_parser)
    stage_parser.add_argument('--build-root', required=True, type=pathlib.Path)
    stage_parser.add_argument('--cache-root', required=True, type=pathlib.Path)
    stage_parser.add_argument('--cache-manifest', required=True, type=pathlib.Path)
    stage_parser.add_argument('--dependency-manifest', required=True, type=pathlib.Path)
    stage_parser.add_argument('--timing-dir', type=pathlib.Path)

    verify_parser = subparsers.add_parser('verify')
    add_profile_arguments(verify_parser)
    verify_parser.add_argument('--expected-manifest', required=True, type=pathlib.Path)
    verify_parser.add_argument('--cache-root', required=True, type=pathlib.Path)

    return parser.parse_args()


def canonical_json(value):
    return (json.dumps(value, ensure_ascii=True, separators=(',', ':'), sort_keys=True) + '\n').encode('utf-8')


def canonical_hash(value):
    return hashlib.sha256(canonical_json(value)).hexdigest()


def read_json(path, description):
    with io.open(path, 'r', encoding='utf-8') as source:
        value = json.load(source)
    if not isinstance(value, dict):
        raise ValueError('%s: expected a JSON object for %s' % (path, description))
    return value


def schema_type_matches(value, expected):
    if expected == 'object':
        return isinstance(value, dict)
    if expected == 'array':
        return isinstance(value, list)
    if expected == 'string':
        return isinstance(value, str)
    if expected == 'integer':
        return isinstance(value, int) and not isinstance(value, bool)
    if expected == 'number':
        return isinstance(value, (int, float)) and not isinstance(value, bool)
    if expected == 'boolean':
        return isinstance(value, bool)
    if expected == 'null':
        return value is None
    raise ValueError('unsupported JSON Schema type: %s' % expected)


def resolve_schema_reference(root_schema, reference):
    if not reference.startswith('#/'):
        raise ValueError('unsupported external JSON Schema reference: %s' % reference)
    value = root_schema
    for token in reference[2:].split('/'):
        token = token.replace('~1', '/').replace('~0', '~')
        if not isinstance(value, dict) or token not in value:
            raise ValueError('unresolved JSON Schema reference: %s' % reference)
        value = value[token]
    return value


def validate_json_schema(value, schema, root_schema, location='$'):
    if '$ref' in schema:
        validate_json_schema(
                value, resolve_schema_reference(root_schema, schema['$ref']),
                root_schema, location)
        return

    expected_types = schema.get('type')
    if expected_types is not None:
        if isinstance(expected_types, str):
            expected_types = [expected_types]
        if not any(schema_type_matches(value, expected) for expected in expected_types):
            raise ValueError('%s: expected JSON type %s' % (
                    location, ' or '.join(expected_types)))

    if 'const' in schema and value != schema['const']:
        raise ValueError('%s: value does not match JSON Schema constant' % location)
    if 'enum' in schema and value not in schema['enum']:
        raise ValueError('%s: value is not in the allowed set' % location)

    if isinstance(value, dict):
        required = schema.get('required', [])
        missing = sorted(set(required) - set(value))
        if missing:
            raise ValueError('%s: missing required fields: %s' % (
                    location, ' '.join(missing)))
        properties = schema.get('properties', {})
        if schema.get('additionalProperties') is False:
            unexpected = sorted(set(value) - set(properties))
            if unexpected:
                raise ValueError('%s: unexpected fields: %s' % (
                        location, ' '.join(unexpected)))
        for name, child in value.items():
            if name in properties:
                validate_json_schema(
                        child, properties[name], root_schema,
                        '%s.%s' % (location, name))
            elif isinstance(schema.get('additionalProperties'), dict):
                validate_json_schema(
                        child, schema['additionalProperties'], root_schema,
                        '%s.%s' % (location, name))
        minimum = schema.get('minProperties')
        if minimum is not None and len(value) < minimum:
            raise ValueError('%s: object has too few fields' % location)

    if isinstance(value, list):
        minimum = schema.get('minItems')
        maximum = schema.get('maxItems')
        if minimum is not None and len(value) < minimum:
            raise ValueError('%s: array has too few items' % location)
        if maximum is not None and len(value) > maximum:
            raise ValueError('%s: array has too many items' % location)
        if schema.get('uniqueItems'):
            encoded = [canonical_json(item) for item in value]
            if len(encoded) != len(set(encoded)):
                raise ValueError('%s: array items are not unique' % location)
        item_schema = schema.get('items')
        if item_schema is not None:
            for index, child in enumerate(value):
                validate_json_schema(
                        child, item_schema, root_schema,
                        '%s[%d]' % (location, index))

    if isinstance(value, str):
        minimum = schema.get('minLength')
        if minimum is not None and len(value) < minimum:
            raise ValueError('%s: string is too short' % location)
        pattern = schema.get('pattern')
        if pattern is not None and re.search(pattern, value) is None:
            raise ValueError('%s: string does not match required pattern' % location)

    if isinstance(value, (int, float)) and not isinstance(value, bool):
        minimum = schema.get('minimum')
        maximum = schema.get('maximum')
        if minimum is not None and value < minimum:
            raise ValueError('%s: number is below minimum' % location)
        if maximum is not None and value > maximum:
            raise ValueError('%s: number is above maximum' % location)


def number_value(value, description, allow_none=False):
    if allow_none and value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError('%s must be a number' % description)
    try:
        result = decimal.Decimal(str(value))
        rounded = result.quantize(MILLISECOND)
    except decimal.InvalidOperation:
        raise ValueError('%s must be a non-negative value with at most three decimals' % description)
    if not result.is_finite() or result < 0 or result != rounded:
        raise ValueError('%s must be a non-negative value with at most three decimals' % description)
    return result


def shard_definition(pool, targets, outputs):
    return {
            'outputs': sorted(outputs),
            'pool': pool,
            'schema_version': DEFINITION_SCHEMA_VERSION,
            'targets': sorted(targets)}


def driver_shard_definition(targets, archive):
    sorted_targets = sorted(targets)
    return shard_definition(
            DRIVER_POOL, sorted_targets,
            ['%s%s%s' % (archive['prefix'], target, archive['suffix'])
             for target in sorted_targets])


def driver_map_identity(driver_map):
    return dict(
            (key, value) for key, value in driver_map.items()
            if key != 'complete_map_hash')


def profile_map_identity(profile_map):
    return dict(
            (key, value) for key, value in profile_map.items()
            if key != 'complete_map_hash')


def validate_driver_map(driver_map, path, archive):
    if driver_map.get('schema_version') != DRIVER_MAP_SCHEMA_VERSION:
        raise ValueError('%s: driver map schema does not match' % path)
    if driver_map.get('partition_algorithm') != PARTITION_ALGORITHM:
        raise ValueError('%s: partition algorithm does not match' % path)
    if driver_map.get('pool') != DRIVER_POOL:
        raise ValueError('%s: driver pool does not match' % path)

    requested = driver_map.get('requested_shards')
    shards = driver_map.get('shards')
    if isinstance(requested, bool) or not isinstance(requested, int) or requested < 1:
        raise ValueError('%s: requested driver shard count is invalid' % path)
    if not isinstance(shards, list) or len(shards) != requested:
        raise ValueError('%s: driver shard list does not match requested count' % path)

    timing = driver_map.get('timing')
    timing_targets = timing.get('targets', {}) if isinstance(timing, dict) else {}
    weights = {}
    for target, record in timing_targets.items():
        weights[target] = number_value(
                record.get('weight') if isinstance(record, dict) else None,
                'timing weight for %s' % target)

    all_targets = []
    predicted_weights = []
    expected_names = ['driver-%d' % index for index in range(1, requested + 1)]
    actual_names = []
    for shard in shards:
        name = shard.get('name')
        actual_names.append(name)
        targets = shard.get('targets')
        if (
                not isinstance(targets, list)
                or not targets
                or targets != sorted(set(targets))):
            raise ValueError('%s: targets for %s are not sorted and unique' % (path, name))
        if shard.get('target_count') != len(targets):
            raise ValueError('%s: target count for %s does not match' % (path, name))
        definition = driver_shard_definition(targets, archive)
        if shard.get('outputs') != definition['outputs']:
            raise ValueError('%s: outputs for %s do not match targets' % (path, name))
        if shard.get('definition_hash') != canonical_hash(definition):
            raise ValueError('%s: definition hash for %s does not match' % (path, name))
        missing_weights = sorted(set(targets) - set(weights))
        if missing_weights:
            raise ValueError('%s: %s has targets without timing weights: %s' % (
                    path, name, ' '.join(missing_weights)))
        predicted = number_value(
                shard.get('predicted_cpu_seconds'),
                'predicted weight for %s' % name)
        calculated = sum(
                (weights[target] for target in targets),
                decimal.Decimal('0')).quantize(MILLISECOND)
        if predicted != calculated:
            raise ValueError('%s: predicted weight for %s does not match targets' % (path, name))
        predicted_weights.append(predicted)
        all_targets.extend(targets)

    if actual_names != expected_names:
        raise ValueError('%s: driver shard names are not canonical' % path)
    duplicates = sorted(
            target for target, count in collections.Counter(all_targets).items()
            if count != 1)
    if duplicates:
        raise ValueError('%s: driver targets assigned more than once: %s' % (
                path, ' '.join(duplicates)))
    if sorted(all_targets) != sorted(timing_targets):
        raise ValueError('%s: assigned driver targets do not match timing targets' % path)

    previous = number_value(
            driver_map.get('previous_maximum_cpu_seconds'),
            'previous maximum weight', allow_none=True)
    proposed = number_value(
            driver_map.get('proposed_maximum_cpu_seconds'),
            'proposed maximum weight')
    predicted = number_value(
            driver_map.get('predicted_maximum_cpu_seconds'),
            'predicted maximum weight')
    if predicted != max(predicted_weights):
        raise ValueError('%s: predicted maximum does not match shard weights' % path)
    if predicted != proposed and predicted != previous:
        raise ValueError('%s: selected driver map is neither proposed nor previous' % path)
    if driver_map.get('complete_map_hash') != canonical_hash(driver_map_identity(driver_map)):
        raise ValueError('%s: driver map hash does not match' % path)
    return driver_map


def validate_profile_map(profile_map, path):
    if profile_map.get('schema_version') != PROFILE_SCHEMA_VERSION:
        raise ValueError('%s: profile map schema does not match' % path)
    profile = profile_map['profile']
    if pathlib.Path(path).stem != profile['id']:
        raise ValueError('%s: profile id does not match file name' % path)
    validate_driver_map(
            profile_map['driver_map'], '%s driver map' % path,
            profile['archive'])

    archive = profile['archive']
    names = []
    targets = []
    for shard in profile_map['library_shards']:
        names.append(shard['name'])
        if shard['targets'] != sorted(set(shard['targets'])):
            raise ValueError('%s: library targets for %s are not sorted and unique' % (
                    path, shard['name']))
        outputs = [
                '%s%s%s' % (archive['prefix'], target, archive['suffix'])
                for target in shard['targets']]
        definition = shard_definition('libraries', shard['targets'], outputs)
        if shard['outputs'] != definition['outputs']:
            raise ValueError('%s: library outputs for %s do not match targets' % (
                    path, shard['name']))
        if shard['definition_hash'] != canonical_hash(definition):
            raise ValueError('%s: library definition hash for %s does not match' % (
                    path, shard['name']))
        targets.extend(shard['targets'])
    if names != sorted(set(names)):
        raise ValueError('%s: library shard names are not sorted and unique' % path)

    driver_names = [shard['name'] for shard in profile_map['driver_map']['shards']]
    duplicate_names = sorted(
            name for name, count in collections.Counter(names + driver_names).items()
            if count != 1)
    if duplicate_names:
        raise ValueError('%s: shard names are duplicated: %s' % (
                path, ' '.join(duplicate_names)))
    targets.extend(
            target for shard in profile_map['driver_map']['shards']
            for target in shard['targets'])
    duplicates = sorted(
            target for target, count in collections.Counter(targets).items()
            if count != 1)
    if duplicates:
        raise ValueError('%s: build targets are assigned more than once: %s' % (
                path, ' '.join(duplicates)))
    deferred = profile['deferred_projects']
    if deferred != sorted(set(deferred)):
        raise ValueError('%s: deferred projects are not sorted and unique' % path)
    overlap = sorted(set(targets) & set(deferred))
    if overlap:
        raise ValueError('%s: projects are both sharded and deferred: %s' % (
                path, ' '.join(overlap)))
    if profile['executable_project'] not in deferred:
        raise ValueError('%s: executable project is not deferred' % path)
    if not set(profile['final_projects']).issubset(deferred):
        raise ValueError('%s: final projects are not all deferred' % path)
    if profile_map.get('complete_map_hash') != canonical_hash(profile_map_identity(profile_map)):
        raise ValueError('%s: complete profile map hash does not match' % path)
    return profile_map


def load_profile_map(path):
    path = path.resolve()
    profile_map = read_json(path, 'CI shard profile map')
    schema_reference = profile_map.get('$schema')
    if not isinstance(schema_reference, str) or not schema_reference:
        raise ValueError('%s: profile map does not reference a JSON Schema' % path)
    schema_path = (path.parent / schema_reference).resolve()
    schema = read_json(schema_path, 'CI shard map JSON Schema')
    validate_json_schema(profile_map, schema, schema, str(path))
    return validate_profile_map(profile_map, str(path))


def library_shards(profile_map):
    return profile_map['library_shards']


def driver_shards(profile_map):
    return profile_map['driver_map']['shards']


def all_shards(profile_map):
    return library_shards(profile_map) + driver_shards(profile_map)


def find_shard(profile_map, name):
    for shard in all_shards(profile_map):
        if shard['name'] == name:
            return shard
    raise ValueError('unknown shard for %s: %s' % (profile_map['profile']['id'], name))


def artifact_shard_definition(profile_map, name):
    shard = find_shard(profile_map, name)
    pool = 'libraries' if shard in library_shards(profile_map) else DRIVER_POOL
    definition = shard_definition(pool, shard['targets'], shard['outputs'])
    return {
            'complete_map_hash': profile_map['complete_map_hash'],
            'definition': definition,
            'definition_hash': shard['definition_hash'],
            'profile': profile_map['profile']['id'],
            'shard': name}


def matrix_profile_entry(path, profile_map):
    profile = profile_map['profile']
    build = profile['build']
    options = build['options']
    ci = profile['ci']
    suffix = profile['executable_suffix']
    return {
            'archopts': options['ARCHOPTS'],
            'ar': options['OVERRIDE_AR'],
            'artifact_name': profile['artifact_name'],
            'build_root': profile['build_root'],
            'cc': options['OVERRIDE_CC'],
            'collect_timings': profile['collect_timings'],
            'configuration': build['configuration'],
            'cxx': options['OVERRIDE_CXX'],
            'executable': './%s%s' % (profile['executable'], suffix),
            'executable_project': profile['executable_project'],
            'executable_suffix': suffix,
            'final_projects': ' '.join(profile['final_projects']),
            'install_kind': ci['install_kind'],
            'make_jobs': build['make_jobs'],
            'msystem': ci['msystem'],
            'osd': options['OSD'],
            'packages': ci['packages'],
            'precompile': options['PRECOMPILE'],
            'profile_id': profile['id'],
            'profile_path': pathlib.Path(path).as_posix(),
            'python': ci['python'],
            'reconcile_list': profile['reconcile_list'] or '',
            'run_orm': profile['run_orm'],
            'runner': ci['runner'],
            'shell': ci['shell'],
            'solution_dir': profile['solution_dir'],
            'subtarget': options['SUBTARGET'],
            'target': options['TARGET'],
            'tools': options['TOOLS'],
            'use_libsdl': options['USE_LIBSDL'],
            'use_sdl3': options['USE_SDL3']}


def matrix_output(options):
    paths = list(options.profile or [])
    if options.profiles_json:
        try:
            values = json.loads(options.profiles_json)
        except ValueError as error:
            raise ValueError('invalid --profiles-json value: %s' % error)
        if not isinstance(values, list) or any(not isinstance(value, str) for value in values):
            raise ValueError('--profiles-json must be an array of paths')
        paths.extend(pathlib.Path(value) for value in values)
    if not paths:
        raise ValueError('provide at least one --profile or --profiles-json entry')

    include = []
    seen = set()
    for path in paths:
        profile_map = load_profile_map(path)
        entry = matrix_profile_entry(path, profile_map)
        if entry['profile_id'] in seen:
            raise ValueError('duplicate matrix profile: %s' % entry['profile_id'])
        seen.add(entry['profile_id'])
        if options.kind == 'profiles':
            include.append(entry)
        else:
            for shard in all_shards(profile_map):
                shard_entry = dict(entry)
                shard_entry['group'] = shard['name']
                include.append(shard_entry)
    return {'include': include}


def workflow_bool(value):
    if isinstance(value, bool):
        return 'true' if value else 'false'
    return str(value)


def write_github_values(path, values):
    if path is None:
        return
    with io.open(path, 'a', encoding='utf-8', newline='\n') as output:
        for name, value in values.items():
            text = workflow_bool(value)
            if '\n' in text or '\r' in text:
                delimiter = 'MAME_CI_%s' % hashlib.sha256(
                        ('%s\0%s' % (name, text)).encode('utf-8')).hexdigest()
                output.write('%s<<%s\n%s\n%s\n' % (
                        name, delimiter, text, delimiter))
            else:
                output.write('%s=%s\n' % (name, text))


def profile_groups(profile_map):
    return set(shard['name'] for shard in all_shards(profile_map))


def write_github_profile_environment(options):
    if options.github_output is None and options.github_env is None:
        raise ValueError('provide --github-output or --github-env')

    profile_map = load_profile_map(options.profile)
    entry = matrix_profile_entry(options.profile, profile_map)
    expected = {
            'install_kind': options.expected_install_kind,
            'msystem': options.expected_msystem,
            'packages': options.expected_packages,
            'profile_id': options.expected_profile_id,
            'python': options.expected_python,
            'runner': options.expected_runner,
            'shell': options.expected_shell}
    for field, expected_value in sorted(expected.items()):
        actual_value = workflow_bool(entry[field])
        if actual_value != expected_value:
            raise ValueError(
                    'workflow input %s does not match %s: expected %s, got %s' % (
                            field, entry['profile_id'], expected_value, actual_value))

    if options.group != 'all' and options.group not in profile_groups(profile_map):
        raise ValueError(
                'workflow input group does not match %s: %s' % (
                        entry['profile_id'], options.group))

    outputs = collections.OrderedDict(
            (name, workflow_bool(value)) for name, value in sorted(entry.items()))
    outputs['group'] = options.group
    write_github_values(options.github_output, outputs)

    env_values = collections.OrderedDict((
            ('ARCHOPTS', entry['archopts']),
            ('CI_ARTIFACT_NAME', entry['artifact_name']),
            ('CI_BUILD_ROOT', entry['build_root']),
            ('CI_COLLECT_TIMINGS', entry['collect_timings']),
            ('CI_CONFIGURATION', entry['configuration']),
            ('CI_EXECUTABLE', entry['executable']),
            ('CI_EXECUTABLE_PROJECT', entry['executable_project']),
            ('CI_EXECUTABLE_SUFFIX', entry['executable_suffix']),
            ('CI_FINAL_PROJECTS', entry['final_projects']),
            ('CI_INSTALL_KIND', entry['install_kind']),
            ('CI_MAKE_JOBS', entry['make_jobs']),
            ('CI_MSYSTEM', entry['msystem']),
            ('CI_PACKAGES', entry['packages']),
            ('CI_PROFILE_ID', entry['profile_id']),
            ('CI_PROFILE_PATH', entry['profile_path']),
            ('CI_PYTHON', entry['python']),
            ('CI_RECONCILE_LIST', entry['reconcile_list']),
            ('CI_RUN_ORM', entry['run_orm']),
            ('CI_RUNNER', entry['runner']),
            ('CI_SHELL', entry['shell']),
            ('CI_SOLUTION_DIR', entry['solution_dir']),
            ('OSD', entry['osd']),
            ('OVERRIDE_AR', entry['ar']),
            ('OVERRIDE_CC', entry['cc']),
            ('OVERRIDE_CXX', entry['cxx']),
            ('PRECOMPILE', entry['precompile']),
            ('SUBTARGET', entry['subtarget']),
            ('TARGET', entry['target']),
            ('TOOLS', entry['tools']),
            ('USE_LIBSDL', entry['use_libsdl']),
            ('USE_SDL3', entry['use_sdl3'])))
    write_github_values(options.github_env, env_values)
    return entry['profile_id']


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


def add_profile_inputs(files, selectors, tracked, context):
    profile_relative = '.github/ci-shards/%s.json' % (
            context['profile_map']['profile']['id'])
    if profile_relative not in tracked:
        raise ValueError('checked profile cache input is not tracked: %s' % profile_relative)
    checked_profile_path = tracked[profile_relative]
    checked_profile = load_profile_map(checked_profile_path)
    if checked_profile['profile'] != context['profile_map']['profile']:
        raise ValueError('runtime profile changes checked build configuration')
    schema_path = (
            checked_profile_path.parent / checked_profile['$schema']).resolve()
    schema_relative = schema_path.relative_to(context['source_root']).as_posix()
    workflow = checked_profile['profile']['workflow']
    for relative_path in (schema_relative, workflow):
        if relative_path not in tracked:
            raise ValueError('profile cache input is not tracked: %s' % relative_path)
        files[relative_path] = tracked[relative_path]
        selectors.append(relative_path)


def collect_broad_input_files(tracked, context):
    files = collect_tracked_roots(tracked, BROAD_INPUT_PATHS)
    selectors = list(BROAD_INPUT_PATHS)
    mame_files = collect_tracked_roots(tracked, ('src/mame',))
    files.update(mame_files)
    selectors.append('src/mame')
    add_profile_inputs(files, selectors, tracked, context)
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


def build_identity(profile_map, runner_os, runner_arch, toolchain):
    profile = profile_map['profile']
    return {
            'archive': profile['archive'],
            'build': profile['build'],
            'build_root': profile['build_root'],
            'ci': profile['ci'],
            'collect_timings': profile['collect_timings'],
            'profile': profile['id'],
            'runner_arch': runner_arch,
            'runner_os': runner_os,
            'solution_dir': profile['solution_dir'],
            'toolchain': toolchain}


def read_solution_projects(path):
    with io.open(path, 'r', encoding='utf-8') as solution_makefile:
        project_lines = [
                line.rstrip('\r\n') for line in solution_makefile
                if line.startswith(PROJECTS_PREFIX)]
    if len(project_lines) != 1:
        raise ValueError('%s: expected one PROJECTS definition' % path)
    projects = project_lines[0][len(PROJECTS_PREFIX):].split()
    if not projects:
        raise ValueError('%s: generated solution contains no projects' % path)
    duplicates = sorted(
            project for project, count in collections.Counter(projects).items()
            if count != 1)
    if duplicates:
        raise ValueError('%s: duplicate generated projects: %s' % (
                path, ' '.join(duplicates)))
    return projects


def expected_solution_projects(profile_map):
    sharded = [
            target for shard in all_shards(profile_map)
            for target in shard['targets']]
    return sorted(sharded + profile_map['profile']['deferred_projects'])


def validate_solution_projects(projects, profile_map):
    expected = set(expected_solution_projects(profile_map))
    actual = set(projects)
    missing = sorted(expected - actual)
    unexpected = sorted(actual - expected)
    if missing or unexpected:
        details = []
        if missing:
            details.append('profile projects absent from generated solution: %s' % ' '.join(missing))
        if unexpected:
            details.append('unassigned generated projects: %s' % ' '.join(unexpected))
        raise ValueError('; '.join(details))
    return len(projects)


def read_project_configuration(path, configuration, description=None):
    description = description or path
    marker = 'ifeq ($(config),%s)' % configuration
    variables = {}
    found = False
    active = False
    depth = 0

    for line in read_make_logical_lines(path):
        stripped = line.strip()
        if not active:
            if stripped == marker:
                if found:
                    raise ValueError(
                            '%s: duplicate configuration block for %s' %
                            (description, configuration))
                found = True
                active = True
                depth = 1
            continue

        if MAKE_CONDITIONAL_PATTERN.match(stripped):
            depth += 1
            continue
        if stripped == 'endif':
            depth -= 1
            if depth == 0:
                active = False
            continue
        if depth != 1:
            continue

        match = MAKE_ASSIGNMENT_PATTERN.match(line)
        if not match:
            continue
        name, operation, value = match.groups()
        if operation == '+=':
            previous = variables.get(name, '')
            variables[name] = '%s%s%s' % (
                    previous, ' ' if previous and value else '', value)
        elif operation == '?=':
            variables.setdefault(name, value)
        else:
            variables[name] = value

    if active:
        raise ValueError('%s: unterminated configuration block for %s' % (
                description, configuration))
    if not found:
        raise ValueError('%s: configuration is absent: %s' % (
                description, configuration))
    return variables


def expand_make_variables(value, variables):
    expanded = value
    for unused in range(20):
        def replace(match):
            name = match.group(1) or match.group(2)
            return variables.get(name, match.group(0))

        updated = MAKE_VARIABLE_PATTERN.sub(replace, expanded)
        if updated == expanded:
            break
        expanded = updated
    unresolved = sorted(set(
            match.group(1) or match.group(2)
            for match in MAKE_VARIABLE_PATTERN.finditer(expanded)))
    return expanded, unresolved


def normalize_project_path(token, project_makefile, source_root):
    if not token or '$' in token or '%' in token:
        return None
    path = pathlib.Path(token)
    if not path.is_absolute():
        path = project_makefile.parent / path
    try:
        return path.resolve().relative_to(source_root).as_posix()
    except ValueError:
        return None


def generated_project_kind(output, link_command, archive):
    name = pathlib.PurePosixPath(output.replace('\\', '/')).name if output else ''
    if re.search(r'(^|\s)\$\(AR\)(?:\s|$)', link_command):
        return 'static-library'
    if (
            name.startswith(archive['prefix'])
            and name.endswith(archive['suffix'])):
        return 'static-library'
    if name.endswith(('.dll', '.dylib', '.so')):
        return 'shared-library'
    if '-o $(TARGET)' in link_command or '-o "$(TARGET)"' in link_command:
        return 'executable'
    return 'unknown'


def read_generated_project(project, solution_makefile, profile_map, source_root):
    project_makefile = solution_makefile.parent / ('%s.make' % project)
    relative_makefile = normalize_project_path(
            str(project_makefile), project_makefile, source_root)
    record = {
            'dependencies': [],
            'kind': 'unknown',
            'makefile': relative_makefile or project_makefile.name,
            'metadata_errors': [],
            'name': project,
            'outputs': [],
            'unresolved_dependencies': []}
    try:
        variables = read_project_configuration(
                project_makefile,
                profile_map['profile']['build']['configuration'],
                record['makefile'])
    except (IOError, OSError, UnicodeError, ValueError) as error:
        record['metadata_errors'].append(str(error))
        return record, []

    target, unresolved_target_variables = expand_make_variables(
            variables.get('TARGET', ''), variables)
    target_words = parse_make_words(target, project_makefile) if target else []
    if unresolved_target_variables:
        record['metadata_errors'].append(
                'target contains unresolved make variables: %s' %
                ' '.join(unresolved_target_variables))
    if len(target_words) != 1:
        record['metadata_errors'].append(
                'expected one generated target path, found %d' % len(target_words))
        output = None
    else:
        output = normalize_project_path(
                target_words[0], project_makefile, source_root)
        if output is None:
            record['metadata_errors'].append(
                    'generated target path is outside the source tree or unresolved: %s' %
                    target_words[0])
        else:
            record['outputs'] = [output]

    link_command = variables.get('LINKCMD', '')
    record['kind'] = generated_project_kind(
            output or (target_words[0] if len(target_words) == 1 else ''),
            link_command,
            profile_map['profile']['archive'])
    if record['kind'] == 'unknown':
        record['metadata_errors'].append('generated project kind is unknown')

    libdeps, unresolved_dependency_variables = expand_make_variables(
            variables.get('LIBDEPS', ''), variables)
    if unresolved_dependency_variables:
        record['metadata_errors'].append(
                'library dependencies contain unresolved make variables: %s' %
                ' '.join(unresolved_dependency_variables))
    dependency_paths = []
    for token in parse_make_words(libdeps, project_makefile):
        dependency = normalize_project_path(token, project_makefile, source_root)
        if dependency is None:
            record['unresolved_dependencies'].append(token.replace('\\', '/'))
        else:
            dependency_paths.append(dependency)
    record['metadata_errors'] = sorted(set(record['metadata_errors']))
    record['unresolved_dependencies'] = sorted(set(record['unresolved_dependencies']))
    return record, sorted(set(dependency_paths))


def generated_project_inventory(solution_makefile, profile_map, source_root):
    source_root = source_root.resolve()
    solution_makefile = solution_makefile.resolve()
    projects = read_solution_projects(solution_makefile)
    executable = profile_map['profile']['executable_project']
    if executable not in projects:
        raise ValueError(
                'primary executable project is absent from generated solution: %s' %
                executable)

    records = {}
    dependency_paths = {}
    output_owners = collections.defaultdict(list)
    for project in sorted(projects):
        record, dependencies = read_generated_project(
                project, solution_makefile, profile_map, source_root)
        records[project] = record
        dependency_paths[project] = dependencies
        for output in record['outputs']:
            output_owners[output].append(project)

    for output, owners in sorted(output_owners.items()):
        if len(owners) > 1:
            message = 'generated output is shared by projects %s: %s' % (
                    ' '.join(sorted(owners)), output)
            for project in owners:
                records[project]['metadata_errors'].append(message)

    for project, paths in sorted(dependency_paths.items()):
        dependencies = []
        unresolved = list(records[project]['unresolved_dependencies'])
        for path in paths:
            owners = output_owners.get(path, [])
            if len(owners) == 1:
                dependencies.append(owners[0])
            else:
                unresolved.append(path)
        records[project]['dependencies'] = sorted(set(dependencies))
        records[project]['unresolved_dependencies'] = sorted(set(unresolved))
        records[project]['metadata_errors'] = sorted(
                set(records[project]['metadata_errors']))

    relative_solution = normalize_project_path(
            str(solution_makefile), solution_makefile, source_root)
    if relative_solution is None:
        raise ValueError('solution makefile is outside the source tree: %s' % (
                solution_makefile))
    identity = {
            'configuration': profile_map['profile']['build']['configuration'],
            'profile': profile_map['profile']['id'],
            'project_count': len(records),
            'projects': [records[project] for project in sorted(records)],
            'schema_version': INVENTORY_SCHEMA_VERSION,
            'solution_makefile': relative_solution}
    inventory = dict(identity)
    inventory['inventory_hash'] = canonical_hash(identity)
    return inventory


def generated_project_inventory_identity(inventory):
    return dict(
            (key, value) for key, value in inventory.items()
            if key != 'inventory_hash')


def validate_generated_project_inventory(inventory, profile_map, path):
    if inventory.get('schema_version') != INVENTORY_SCHEMA_VERSION:
        raise ValueError('%s: generated-project inventory schema does not match' % path)
    if inventory.get('profile') != profile_map['profile']['id']:
        raise ValueError('%s: generated-project inventory profile does not match' % path)
    projects = inventory.get('projects')
    if not isinstance(projects, list):
        raise ValueError('%s: generated-project inventory has no project list' % path)
    names = []
    for project in projects:
        if not isinstance(project, dict):
            raise ValueError('%s: generated-project inventory contains an invalid project' % path)
        name = project.get('name')
        if not isinstance(name, str) or not name:
            raise ValueError('%s: generated-project inventory contains an unnamed project' % path)
        names.append(name)
    if names != sorted(set(names)):
        raise ValueError('%s: generated-project inventory names are not sorted and unique' % path)
    if inventory.get('project_count') != len(projects):
        raise ValueError('%s: generated-project inventory count does not match' % path)
    if inventory.get('inventory_hash') != canonical_hash(
            generated_project_inventory_identity(inventory)):
        raise ValueError('%s: generated-project inventory hash does not match' % path)
    return inventory


def checked_project_details(profile_map):
    archive = profile_map['profile']['archive']
    details = {}
    for shard in all_shards(profile_map):
        for target in shard['targets']:
            details[target] = {
                    'checked_group': shard['name'],
                    'kind': 'static-library',
                    'name': target,
                    'outputs': [
                            '%s%s%s' % (
                                    archive['prefix'], target, archive['suffix'])]}
    executable_suffix = profile_map['profile']['executable_suffix']
    for target in profile_map['profile']['deferred_projects']:
        details[target] = {
                'checked_group': 'deferred',
                'kind': 'executable',
                'name': target,
                'outputs': ['%s%s' % (target, executable_suffix)]}
    return details


def generated_target_drift(
        inventory, profile_map, recovery_mode=None, provisional_assignments=None,
        reconciliation_errors=None, runtime_manifest_hash=None):
    provisional_assignments = provisional_assignments or {}
    reconciliation_errors = sorted(set(reconciliation_errors or []))
    actual = dict((project['name'], project) for project in inventory['projects'])
    expected = checked_project_details(profile_map)
    added_names = sorted(set(actual) - set(expected))
    removed_names = sorted(set(expected) - set(actual))
    added = []
    for name in added_names:
        record = dict(actual[name])
        record['provisional_assignment'] = provisional_assignments.get(name)
        added.append(record)
    removed = [expected[name] for name in removed_names]
    has_drift = bool(added or removed)
    if recovery_mode is None:
        recovery_mode = 'pending-reconciliation' if has_drift else 'exact'
    assignments = [
            {'group': provisional_assignments[name], 'project': name}
            for name in sorted(set(added_names) & set(provisional_assignments))]
    if runtime_manifest_hash is None and not has_drift:
        runtime_manifest_hash = profile_map['complete_map_hash']
    identity = {
            'added_projects': added,
            'checked_manifest_hash': profile_map['complete_map_hash'],
            'has_drift': has_drift,
            'inventory_hash': inventory['inventory_hash'],
            'profile': profile_map['profile']['id'],
            'provisional_assignments': assignments,
            'reconciliation_errors': reconciliation_errors,
            'recovery_mode': recovery_mode,
            'removed_projects': removed,
            'runtime_manifest_hash': runtime_manifest_hash,
            'schema_version': DRIFT_SCHEMA_VERSION}
    report = dict(identity)
    report['drift_hash'] = canonical_hash(identity)
    return report


def fallback_marker_identity(marker):
    return dict(
            (key, value) for key, value in marker.items()
            if key != 'marker_hash')


def monolithic_fallback_marker(report):
    identity = {
            'checked_manifest_hash': report['checked_manifest_hash'],
            'drift_hash': report['drift_hash'],
            'inventory_hash': report['inventory_hash'],
            'profile': report['profile'],
            'reasons': sorted(set(report['reconciliation_errors'])),
            'recovery_mode': 'monolithic-fallback',
            'schema_version': FALLBACK_SCHEMA_VERSION}
    marker = dict(identity)
    marker['marker_hash'] = canonical_hash(identity)
    return marker


def validate_monolithic_fallback_marker(marker, profile_map, path):
    expected_fields = {
            'checked_manifest_hash',
            'drift_hash',
            'inventory_hash',
            'marker_hash',
            'profile',
            'reasons',
            'recovery_mode',
            'schema_version'}
    if set(marker) != expected_fields:
        raise ValueError('%s: fallback marker fields do not match' % path)
    if marker.get('schema_version') != FALLBACK_SCHEMA_VERSION:
        raise ValueError('%s: fallback marker schema does not match' % path)
    if marker.get('profile') != profile_map['profile']['id']:
        raise ValueError('%s: fallback marker profile does not match' % path)
    if marker.get('checked_manifest_hash') != profile_map['complete_map_hash']:
        raise ValueError('%s: fallback marker checked manifest does not match' % path)
    if marker.get('recovery_mode') != 'monolithic-fallback':
        raise ValueError('%s: fallback marker recovery mode does not match' % path)
    reasons = marker.get('reasons')
    if (
            not isinstance(reasons, list)
            or not reasons
            or reasons != sorted(set(reasons))
            or any(not isinstance(reason, str) or not reason for reason in reasons)):
        raise ValueError('%s: fallback marker reasons are invalid' % path)
    for field in ('drift_hash', 'inventory_hash', 'marker_hash'):
        value = marker.get(field)
        if not isinstance(value, str) or re.match(r'^[0-9a-f]{64}$', value) is None:
            raise ValueError('%s: fallback marker %s is invalid' % (path, field))
    if marker['marker_hash'] != canonical_hash(fallback_marker_identity(marker)):
        raise ValueError('%s: fallback marker hash does not match' % path)
    return marker


def reconcilable_archive_output(project, profile_map):
    name = project.get('name', '<unnamed>')
    errors = []
    if not isinstance(name, str) or re.match(r'^[A-Za-z0-9_.+-]+$', name) is None:
        errors.append('project name is invalid')
    if project.get('kind') != 'static-library':
        errors.append('project kind is not a static library')
    for field in ('dependencies', 'metadata_errors', 'outputs', 'unresolved_dependencies'):
        if not isinstance(project.get(field), list):
            errors.append('%s metadata is invalid' % field.replace('_', ' '))
    if isinstance(project.get('metadata_errors'), list) and project['metadata_errors']:
        errors.append('generated metadata has errors: %s' % ' | '.join(project['metadata_errors']))
    if isinstance(project.get('dependencies'), list) and project['dependencies']:
        errors.append('project dependencies are not supported: %s' % ' '.join(project['dependencies']))
    if (
            isinstance(project.get('unresolved_dependencies'), list)
            and project['unresolved_dependencies']):
        errors.append('project has unresolved dependencies: %s' % ' '.join(
                project['unresolved_dependencies']))

    outputs = project.get('outputs')
    output = None
    if isinstance(outputs, list):
        if len(outputs) != 1:
            errors.append('expected exactly one generated output')
        else:
            output = outputs[0]
            try:
                validate_relative_path(output, 'generated archive output')
            except ValueError as error:
                errors.append(str(error))
                output = None

    archive = profile_map['profile']['archive']
    expected_output = '%s%s%s' % (archive['prefix'], name, archive['suffix'])
    if output is not None:
        if not is_under(output, profile_map['profile']['build_root']):
            errors.append('generated output is outside the profile build root: %s' % output)
        if pathlib.PurePosixPath(output).name != expected_output:
            errors.append('generated output does not match archive naming: %s' % output)
    if errors:
        raise MonolithicFallbackRequired('cannot reconcile generated target %s: %s' % (
                name, '; '.join(errors)))
    return expected_output


def reconciled_library_shards(profile_map, actual_projects):
    archive = profile_map['profile']['archive']
    result = []
    for checked in library_shards(profile_map):
        targets = sorted(set(checked['targets']) & actual_projects)
        if not targets:
            raise MonolithicFallbackRequired(
                    'cannot reconcile an empty fixed library shard: %s' % checked['name'])
        outputs = [
                '%s%s%s' % (archive['prefix'], target, archive['suffix'])
                for target in targets]
        definition = shard_definition('libraries', targets, outputs)
        result.append({
                'definition_hash': canonical_hash(definition),
                'name': checked['name'],
                'outputs': definition['outputs'],
                'targets': definition['targets']})
    return result


def reconcile_profile_map(inventory, profile_map, profile_path):
    validate_generated_project_inventory(inventory, profile_map, '<generated inventory>')
    report = generated_target_drift(inventory, profile_map)
    if not report['has_drift']:
        return json.loads(json.dumps(profile_map)), {}

    actual = dict((project['name'], project) for project in inventory['projects'])
    actual_projects = set(actual)
    checked = checked_project_details(profile_map)
    added_projects = sorted(actual_projects - set(checked))
    removed_projects = sorted(set(checked) - actual_projects)
    removed_deferred = sorted(
            set(removed_projects) & set(profile_map['profile']['deferred_projects']))
    if removed_deferred:
        raise MonolithicFallbackRequired('cannot reconcile removed deferred projects: %s' % (
                ' '.join(removed_deferred)))

    for name in added_projects:
        reconcilable_archive_output(actual[name], profile_map)

    previous_driver_map = profile_map['driver_map']
    previous_driver_targets = set(previous_driver_map['timing']['targets'])
    elastic_targets = sorted(
            (previous_driver_targets & actual_projects) | set(added_projects))
    requested_shards = previous_driver_map['requested_shards']
    if len(elastic_targets) < requested_shards:
        raise MonolithicFallbackRequired(
                'cannot reconcile %d elastic targets into %d non-empty shards' % (
                        len(elastic_targets), requested_shards))

    updated = json.loads(json.dumps(profile_map))
    updated['library_shards'] = reconciled_library_shards(
            profile_map, actual_projects)
    if set(elastic_targets) != previous_driver_targets:
        previous_timing = previous_driver_map['timing']['targets']
        previous_weights = dict(
                (target, number_value(record['weight'], 'timing weight for %s' % target))
                for target, record in previous_timing.items())
        if not previous_weights:
            raise ValueError('cannot reconcile without a compatible archive timing weight')
        fallback_weight = max(previous_weights.values())
        weights = {}
        timing_records = {}
        for target in elastic_targets:
            if target in previous_timing:
                weights[target] = previous_weights[target]
                timing_records[target] = json.loads(json.dumps(previous_timing[target]))
            else:
                weights[target] = fallback_weight
                timing_records[target] = {
                        'sample_count': 0,
                        'weight': json_seconds(fallback_weight)}

        assignments, loads = partition_targets(
                elastic_targets, weights, requested_shards,
                previous_assignment(previous_driver_map))
        proposed_maximum = max(loads.values())
        previous_maximum = number_value(
                previous_driver_map['predicted_maximum_cpu_seconds'],
                'previous maximum archive weight')
        updated['driver_map'] = build_driver_map(
                assignments,
                weights,
                timing_records,
                json.loads(json.dumps(previous_driver_map['timing']['identity'])),
                json.loads(json.dumps(previous_driver_map['timing']['sources'])),
                previous_maximum,
                proposed_maximum,
                profile_map['profile']['archive'])

    updated['complete_map_hash'] = canonical_hash(profile_map_identity(updated))
    validate_profile_map(updated, str(profile_path))
    validate_solution_projects(sorted(actual_projects), updated)
    assignments = previous_assignment(updated['driver_map'])
    return updated, dict(
            (name, assignments[name]) for name in added_projects)


def render_target_drift_summary(report):
    if not report['has_drift']:
        return ''

    rows = []
    for project in report['added_projects']:
        outputs = '<br>'.join(
                '<code>%s</code>' % html.escape(output)
                for output in project['outputs']) or '<em>unknown</em>'
        assignment = project.get('provisional_assignment')
        if assignment:
            assignment_html = '<code>%s</code>' % html.escape(assignment)
        elif report['recovery_mode'] == 'monolithic-fallback':
            assignment_html = '<em>Monolithic build</em>'
        else:
            assignment_html = '<em>Pending reconciliation</em>'
        rows.append(
                '<tr><td>Added</td><td><code>%s</code></td><td>%s</td>'
                '<td>%s</td><td>%s</td></tr>' % (
                        html.escape(project['name']),
                        html.escape(project['kind']), outputs, assignment_html))
    for project in report['removed_projects']:
        outputs = '<br>'.join(
                '<code>%s</code>' % html.escape(output)
                for output in project['outputs']) or '<em>unknown</em>'
        rows.append(
                '<tr><td>Removed</td><td><code>%s</code></td><td>%s</td>'
                '<td>%s</td><td><code>%s</code></td></tr>' % (
                        html.escape(project['name']),
                        html.escape(project['kind']), outputs,
                        html.escape(project['checked_group'])))

    errors = report.get('reconciliation_errors', [])
    error_summary = ''
    if errors:
        heading = (
                'Monolithic fallback reasons'
                if report['recovery_mode'] == 'monolithic-fallback'
                else 'Reconciliation errors')
        error_summary = '<h2>%s</h2><ul>%s</ul>' % (
                heading,
                ''.join('<li>%s</li>' % html.escape(error) for error in errors))

    build_action = ''
    if report['recovery_mode'] == 'monolithic-fallback':
        build_action = (
                '<p><strong>Build action:</strong> shard compilation was skipped; '
                'the final job is running the original monolithic build and '
                'validation path.</p>')
        maintainer_action = (
                '<p><strong>Maintainer action:</strong> review the generated '
                'project metadata and update the checked profile or shard '
                'classification before the next optimized build.</p>')
    else:
        maintainer_action = (
                '<p><strong>Maintainer action:</strong> use '
                '<code>scripts/build/ci_shards.py rebalance</code> with compatible '
                'timing data, review the result, and update the checked-in profile.'
                '</p>')

    return '\n'.join((
            '<h1>CI shard target manifest drift detected</h1>',
            '<p><strong>Profile:</strong> <code>%s</code></p>' %
            html.escape(report['profile']),
            '<p><strong>Recovery mode:</strong> <code>%s</code></p>' %
            html.escape(report['recovery_mode']),
            '<table>',
            '<thead><tr><th>Change</th><th>Project</th><th>Kind</th>'
            '<th>Outputs</th><th>Assignment</th></tr></thead>',
            '<tbody>%s</tbody>' % ''.join(rows),
            '</table>',
            error_summary,
            build_action,
            maintainer_action,
            ''))


def write_text(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with io.open(path, 'w', encoding='utf-8', newline='\n') as output:
        output.write(value)


def write_generated_project_inventory(options):
    profile_map = load_profile_map(options.profile)
    inventory = generated_project_inventory(
            options.solution_makefile, profile_map, options.source_root)
    report = generated_target_drift(inventory, profile_map)
    write_manifest(options.inventory_output, inventory)
    write_manifest(options.drift_output, report)
    if options.summary_output is not None:
        write_text(options.summary_output, render_target_drift_summary(report))
    write_github_values(options.github_output, {
            'drift': report['has_drift'],
            'drift_hash': report['drift_hash'],
            'inventory_hash': inventory['inventory_hash']})
    return inventory, report


def write_runtime_profile(checked_profile_path, output_path, runtime_profile):
    if output_path.stem != runtime_profile['profile']['id']:
        raise ValueError('%s: runtime profile id does not match file name' % output_path)
    checked_profile = load_profile_map(checked_profile_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    if checked_profile == runtime_profile:
        shutil.copy2(str(checked_profile_path), str(output_path))
    else:
        write_profile_map(output_path, runtime_profile)

    schema_reference = runtime_profile['$schema']
    schema_source = (checked_profile_path.parent / schema_reference).resolve()
    schema_output = (output_path.parent / schema_reference).resolve()
    schema_output.parent.mkdir(parents=True, exist_ok=True)
    if schema_source != schema_output:
        shutil.copy2(str(schema_source), str(schema_output))
    written = load_profile_map(output_path)
    if written != runtime_profile:
        raise ValueError('%s: written runtime profile does not match' % output_path)
    return schema_output


def write_reconciliation_report(options, inventory, report, runtime_profile_path=None):
    write_manifest(options.drift_output, report)
    if options.summary_output is not None:
        write_text(options.summary_output, render_target_drift_summary(report))
    write_github_values(options.github_output, {
            'drift': report['has_drift'],
            'drift_hash': report['drift_hash'],
            'fallback': report['recovery_mode'] == 'monolithic-fallback',
            'inventory_hash': inventory['inventory_hash'],
            'recovery_mode': report['recovery_mode'],
            'runtime_profile': runtime_profile_path or ''})


def remove_runtime_profile(checked_profile_path, runtime_profile_path, profile_map):
    if runtime_profile_path.is_file():
        runtime_profile_path.unlink()
    schema_reference = profile_map['$schema']
    schema_source = (checked_profile_path.parent / schema_reference).resolve()
    schema_output = (runtime_profile_path.parent / schema_reference).resolve()
    if schema_output != schema_source and schema_output.is_file():
        schema_output.unlink()


def write_reconciled_profile(options):
    checked_profile = load_profile_map(options.profile)
    inventory = read_manifest(options.inventory)
    validate_generated_project_inventory(inventory, checked_profile, options.inventory)
    try:
        runtime_profile, assignments = reconcile_profile_map(
                inventory, checked_profile, options.profile)
    except MonolithicFallbackRequired as error:
        report = generated_target_drift(
                inventory,
                checked_profile,
                recovery_mode='monolithic-fallback',
                reconciliation_errors=[str(error)])
        remove_runtime_profile(options.profile, options.output, checked_profile)
        write_manifest(
                options.fallback_marker,
                monolithic_fallback_marker(report))
        write_reconciliation_report(options, inventory, report)
        return None, report

    recovery_mode = (
            'runtime-reconciled'
            if generated_target_drift(inventory, checked_profile)['has_drift']
            else 'exact')
    report = generated_target_drift(
            inventory,
            checked_profile,
            recovery_mode=recovery_mode,
            provisional_assignments=assignments,
            runtime_manifest_hash=runtime_profile['complete_map_hash'])
    if options.fallback_marker.is_file():
        options.fallback_marker.unlink()
    write_runtime_profile(options.profile, options.output, runtime_profile)
    write_reconciliation_report(
            options, inventory, report, options.output.as_posix())
    return runtime_profile, report


def select_projects(group, projects, profile_map):
    validate_solution_projects(projects, profile_map)
    if group == 'all':
        selected = {
                target for shard in all_shards(profile_map)
                for target in shard['targets']}
    elif group == 'final':
        selected = set(profile_map['profile']['final_projects'])
    else:
        selected = set(find_shard(profile_map, group)['targets'])
    return [project for project in projects if project in selected]


def checked_shard_definition(profile_map, group, projects):
    checked = find_shard(profile_map, group)
    pool = 'libraries' if checked in library_shards(profile_map) else DRIVER_POOL
    definition = shard_definition(pool, projects, checked['outputs'])
    if definition['targets'] != checked['targets']:
        raise ValueError('%s: cache definition does not match checked profile map' % group)
    if canonical_hash(definition) != checked['definition_hash']:
        raise ValueError('%s: cache definition hash does not match checked profile map' % group)
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
    profile_path = options.profile.resolve()
    profile_map = load_profile_map(profile_path)
    solution_projects = read_solution_projects(solution_makefile)
    projects = select_projects(options.group, solution_projects, profile_map)
    definition = checked_shard_definition(profile_map, options.group, projects)
    identity = build_identity(
            profile_map,
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
            'profile_map': profile_map,
            'profile_path': profile_path,
            'solution_makefile': solution_makefile,
            'source_root': source_root}


def dependency_cache_prefix(options):
    context = build_context(options)
    return '%s-%s-%s-%s-%s-' % (
            DEPENDENCY_KEY_PREFIX,
            context['profile_map']['profile']['id'],
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
            'dependency_format': manifest.get('dependency_format'),
            'definition_hash': manifest.get('definition_hash'),
            'dependencies': manifest.get('dependencies'),
            'profile': manifest.get('profile'),
            'schema_version': manifest.get('schema_version'),
            'shard': manifest.get('shard')}


def validate_dependency_manifest(
        manifest, context, expected_shape_hash=None, allow_missing_dependencies=False):
    if manifest.get('schema_version') != DEPENDENCY_SCHEMA_VERSION:
        raise ValueError('dependency manifest schema does not match')
    profile = context['profile_map']['profile']
    if manifest.get('profile') != profile['id']:
        raise ValueError('dependency manifest profile does not match')
    dependency_format = profile['build']['dependency_format']
    if manifest.get('dependency_format') != dependency_format:
        raise ValueError('dependency manifest compiler format does not match')
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
        if not allow_missing_dependencies or path.exists() or path.is_symlink():
            require_input_file(relative_path, path)

    dependency_files = manifest.get('dependency_files')
    if not isinstance(dependency_files, list) or not dependency_files:
        raise ValueError('dependency manifest contains no compiler dependency files')
    if dependency_files != sorted(set(dependency_files)):
        raise ValueError('compiler dependency file paths are not sorted and unique')
    for relative_path in dependency_files:
        validate_relative_path(relative_path, 'compiler dependency file')
        if (
                not is_under(relative_path, dependency_root)
                or not pathlib.PurePosixPath(relative_path).match(
                        profile['build']['dependency_glob'])):
            raise ValueError('compiler dependency file is outside its root: %s' % relative_path)

    if manifest.get('dependency_count') != len(dependencies):
        raise ValueError('dependency manifest path count does not match')
    if manifest.get('dependency_file_count') != len(dependency_files):
        raise ValueError('compiler dependency file count does not match')
    if manifest.get('manifest_hash') != canonical_hash(dependency_manifest_identity(manifest)):
        raise ValueError('dependency manifest hash does not match')
    return dependencies


def dependency_manifest_for_key(path, context, required, allow_missing_dependencies=False):
    if path is None or not path.is_file():
        if required:
            raise ValueError('required dependency manifest is missing')
        return None, 'missing'
    try:
        manifest = read_manifest(path)
        validate_dependency_manifest(
                manifest, context,
                allow_missing_dependencies=allow_missing_dependencies)
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
    add_profile_inputs(files, selectors, tracked, context)
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
        files, selectors = collect_broad_input_files(tracked, context)
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
    cache_key = '%s-%s-%s-%s-%s-%s-%s' % (
            CACHE_KEY_PREFIX,
            context['profile_map']['profile']['id'],
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
            'profile': context['profile_map']['profile']['id'],
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
        elif character == ':' and not (
                index > 0
                and line[index - 1].isalpha()
                and (index == 1 or line[index - 2].isspace())
                and (index + 1) < len(line)
                and line[index + 1] in ('/', '\\')):
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


def read_make_logical_lines(path):
    pending = ''
    with io.open(path, 'r', encoding='utf-8') as makefile:
        for physical_line in makefile:
            line = physical_line.rstrip('\r\n')
            if pending:
                line = pending + line.lstrip()
            if line.endswith('\\'):
                pending = line[:-1] + ' '
            else:
                yield line
                pending = ''
    if pending:
        yield pending.rstrip()


def make_rule_separator(line):
    escaped = False
    for index, character in enumerate(line):
        if escaped:
            escaped = False
        elif character == '\\':
            escaped = True
        elif character == ':':
            if (index + 1) >= len(line) or line[index + 1] != '=':
                return index
    return None


def normalize_make_target(token, makefile, source_root):
    if not token or '$' in token or '%' in token:
        return None
    path = pathlib.Path(token)
    if not path.is_absolute():
        path = makefile.parent / path
    try:
        return path.resolve().relative_to(source_root).as_posix()
    except ValueError:
        return None


def find_generated_dependency_targets(context, dependencies):
    needed = set(dependencies)
    matches = {}
    for relative_makefile in context['project_makefiles']:
        makefile = context['source_root'].joinpath(*relative_makefile.split('/'))
        for line in read_make_logical_lines(makefile):
            if not line or line[0].isspace() or line.lstrip().startswith('#'):
                continue
            separator = make_rule_separator(line)
            if separator is None:
                continue
            for target in parse_make_words(line[:separator], makefile):
                relative_target = normalize_make_target(
                        target, makefile, context['source_root'])
                if relative_target in needed and relative_target not in matches:
                    matches[relative_target] = (makefile, target)
    return matches, sorted(needed - set(matches))


def materialize_dependencies(options):
    context = build_context(options)
    dependency_manifest, dependency_status = dependency_manifest_for_key(
            options.dependency_manifest,
            context,
            required=False,
            allow_missing_dependencies=True)
    if dependency_manifest is None:
        return 0, dependency_status

    missing = []
    for relative_path in dependency_manifest['dependencies']:
        path = context['source_root'].joinpath(*relative_path.split('/'))
        if not path.is_file() and not path.is_symlink():
            missing.append(relative_path)
    if not missing:
        return 0, 'current'

    targets, unresolved = find_generated_dependency_targets(context, missing)
    if unresolved:
        sys.stderr.write(
                'Generated dependencies cannot be materialized; using conservative inputs: %s\n' %
                ' '.join(unresolved))
        return 0, 'unresolved'

    by_makefile = collections.defaultdict(list)
    for relative_path in missing:
        makefile, target = targets[relative_path]
        by_makefile[makefile].append((relative_path, target))

    build = context['profile_map']['profile']['build']
    solution_dir = context['solution_makefile'].parent
    for makefile in sorted(by_makefile, key=lambda path: path.as_posix()):
        selected = sorted(by_makefile[makefile])
        command = [
                options.make_program,
                '-j%d' % build['make_jobs'],
                '-C', str(solution_dir),
                '-f', makefile.name,
                'config=%s' % build['configuration']]
        command.extend(target for relative_path, target in selected)
        try:
            subprocess.check_call(command)
        except (OSError, subprocess.CalledProcessError) as error:
            sys.stderr.write(
                    'Generated dependencies could not be rebuilt; using conservative inputs: %s\n' %
                    error)
            return 0, 'failed'

    remaining = []
    for relative_path in missing:
        path = context['source_root'].joinpath(*relative_path.split('/'))
        if not path.is_file() and not path.is_symlink():
            remaining.append(relative_path)
    if remaining:
        sys.stderr.write(
                'Generated dependency rules produced no output; using conservative inputs: %s\n' %
                ' '.join(remaining))
        return 0, 'failed'
    return len(missing), 'materialized'


def read_make_dependencies(path):
    with io.open(path, 'r', encoding='utf-8') as dependency_file:
        text = dependency_file.read()
    dependencies = parse_make_words(first_make_rule(text, path), path)
    if not dependencies:
        raise ValueError('%s: compiler dependency rule contains no prerequisites' % path)
    return dependencies


def read_msvc_dependencies(path):
    dependency = read_json(path, 'MSVC source dependency output')
    data = dependency.get('Data')
    if not isinstance(data, dict):
        raise ValueError('%s: MSVC dependency output contains no Data object' % path)
    result = []
    source = data.get('Source')
    if isinstance(source, str) and source:
        result.append(source)
    includes = data.get('Includes', [])
    if not isinstance(includes, list):
        raise ValueError('%s: MSVC dependency Includes is not an array' % path)
    result.extend(value for value in includes if isinstance(value, str) and value)
    for field in ('ImportedHeaderUnits', 'ImportedModules'):
        entries = data.get(field, [])
        if not isinstance(entries, list):
            raise ValueError('%s: MSVC dependency %s is not an array' % (path, field))
        for entry in entries:
            if isinstance(entry, str) and entry:
                result.append(entry)
            elif isinstance(entry, dict):
                for key in ('BMI', 'Header', 'Source'):
                    value = entry.get(key)
                    if isinstance(value, str) and value:
                        result.append(value)
    if not result:
        raise ValueError('%s: MSVC dependency output contains no source inputs' % path)
    return result


def read_compiler_dependencies(path, dependency_format):
    if dependency_format == 'make':
        return read_make_dependencies(path)
    if dependency_format == 'msvc-json':
        return read_msvc_dependencies(path)
    raise ValueError('unsupported compiler dependency format: %s' % dependency_format)


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


def collect_compiler_dependencies(
        dependency_files, context, dependency_root, dependency_format):
    dependencies = set()
    external = set()
    intermediate = set()
    for dependency_file in dependency_files:
        for token in read_compiler_dependencies(dependency_file, dependency_format):
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

    profile = context['profile_map']['profile']
    dependency_format = profile['build']['dependency_format']
    dependency_files = sorted(
            (path for path in dependency_root.rglob(
                    profile['build']['dependency_glob']) if path.is_file()),
            key=lambda path: path.relative_to(context['source_root']).as_posix())
    if not dependency_files:
        raise ValueError('%s: no compiler dependency files found' % dependency_root)
    relative_files = [path.relative_to(context['source_root']).as_posix() for path in dependency_files]
    dependencies, external_count, intermediate_count = collect_compiler_dependencies(
            dependency_files, context, relative_root, dependency_format)
    if not dependencies:
        raise ValueError('%s: compiler dependency files contain no repository inputs' % dependency_root)

    manifest = {
            'build_hash': context['build_hash'],
            'dependency_format': dependency_format,
            'definition_hash': context['definition_hash'],
            'dependencies': dependencies,
            'dependency_count': len(dependencies),
            'dependency_file_count': len(relative_files),
            'dependency_files': relative_files,
            'dependency_root': relative_root,
            'ignored_external_count': external_count,
            'ignored_intermediate_count': intermediate_count,
            'project_shape_hash': context['project_shape_hash'],
            'profile': profile['id'],
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
            'profile',
            'schema_version')
    mismatches = [field for field in fields if expected.get(field) != actual.get(field)]
    if mismatches:
        raise ValueError('%s: cache manifest mismatch: %s' % (actual_path, ' '.join(mismatches)))


def verify_archives(cache_root, outputs, archive):
    if not cache_root.is_dir():
        raise ValueError('%s: shard cache directory does not exist' % cache_root)

    expected = set(outputs)
    archives = [
            path for path in cache_root.rglob(
                    '%s*%s' % (archive['prefix'], archive['suffix']))
            if path.is_file()]
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


def verify_timings(cache_root, projects):
    expected = {'%s.timing' % project for project in projects}
    paths = [path for path in cache_root.rglob('*.timing') if path.is_file()]
    names = [path.name for path in paths]
    actual = set(names)
    missing = sorted(expected - actual)
    unexpected = sorted(actual - expected)
    duplicates = sorted(
            name for name, count in collections.Counter(names).items()
            if count != 1)
    empty = sorted(path.as_posix() for path in paths if not path.stat().st_size)
    if missing or unexpected or duplicates or empty:
        details = []
        if missing:
            details.append('missing project timings: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected project timings: %s' % ' '.join(unexpected))
        if duplicates:
            details.append('duplicate project timings: %s' % ' '.join(duplicates))
        if empty:
            details.append('empty project timings: %s' % ' '.join(empty))
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
    dependency_glob = context['profile_map']['profile']['build']['dependency_glob']
    dependency_root = cache_root.joinpath(*manifest['dependency_root'].split('/'))
    actual_paths = (
            [path for path in dependency_root.rglob(dependency_glob) if path.is_file()]
            if dependency_root.is_dir() else [])
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
    profile_map = load_profile_map(options.profile)
    solution_projects = read_solution_projects(solution_makefile)
    projects = select_projects(options.group, solution_projects, profile_map)
    expected = read_manifest(options.expected_manifest)
    actual_path = options.cache_root / ('shard-manifest-%s.json' % options.group)
    actual = read_manifest(actual_path)
    verify_manifest(expected, actual, actual_path)

    definition = checked_shard_definition(profile_map, options.group, projects)
    if actual.get('definition') != definition:
        raise ValueError('%s: manifest does not match generated project selection' % actual_path)
    project_makefiles, shape_hash = project_shape(solution_makefile, source_root, projects)
    if actual.get('project_makefiles') != project_makefiles:
        raise ValueError('%s: manifest does not match generated project makefiles' % actual_path)
    if actual.get('project_shape_hash') != shape_hash:
        raise ValueError('%s: manifest does not match generated project shape' % actual_path)

    verify_archives(
            options.cache_root, definition['outputs'],
            profile_map['profile']['archive'])
    if profile_map['profile']['collect_timings']:
        verify_timings(options.cache_root, projects)
    context = {
            'build_hash': expected['build_hash'],
            'definition_hash': expected['definition_hash'],
            'group': options.group,
            'profile_map': profile_map,
            'source_root': source_root}
    dependency_count = verify_dependency_payload(options.cache_root, expected, context)
    return len(projects), dependency_count, actual.get('source_commit', 'unknown')


def copy_repository_file(source_root, source, destination_root):
    source = source.resolve()
    try:
        relative = source.relative_to(source_root)
    except ValueError:
        raise ValueError('%s: staged file is outside the source root' % source)
    destination = destination_root / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(str(source), str(destination))
    return relative.as_posix()


def stage_artifacts(options):
    source_root = options.source_root.resolve()
    solution_makefile = options.solution_makefile.resolve()
    profile_map = load_profile_map(options.profile)
    projects = select_projects(
            options.group, read_solution_projects(solution_makefile), profile_map)
    definition = checked_shard_definition(profile_map, options.group, projects)
    cache_manifest = read_manifest(options.cache_manifest)
    if cache_manifest.get('definition') != definition:
        raise ValueError('cache manifest does not match staged shard definition')

    context = {
            'build_hash': cache_manifest.get('build_hash'),
            'definition_hash': canonical_hash(definition),
            'group': options.group,
            'profile_map': profile_map,
            'source_root': source_root}
    dependency_manifest = read_manifest(options.dependency_manifest)
    validate_dependency_manifest(
            dependency_manifest, context,
            expected_shape_hash=cache_manifest.get('project_shape_hash'))

    build_root = options.build_root.resolve()
    try:
        build_root.relative_to(source_root)
    except ValueError:
        raise ValueError('%s: build root is outside the source root' % build_root)
    if not build_root.is_dir():
        raise ValueError('%s: build root does not exist' % build_root)

    cache_root = options.cache_root.resolve()
    try:
        cache_relative = cache_root.relative_to(source_root)
    except ValueError:
        raise ValueError('%s: cache root is outside the source root' % cache_root)
    if not cache_relative.parts or cache_relative.parts[0] != 'build':
        raise ValueError('%s: cache root must be below build' % cache_root)
    if cache_root.exists():
        shutil.rmtree(str(cache_root))
    cache_root.mkdir(parents=True)

    staged_archives = []
    for output in definition['outputs']:
        matches = [path for path in build_root.rglob(output) if path.is_file()]
        if len(matches) != 1:
            raise ValueError('expected one archive for %s, found %d' % (
                    output, len(matches)))
        staged_archives.append(copy_repository_file(source_root, matches[0], cache_root))

    for relative_path in dependency_manifest['dependency_files']:
        source = source_root.joinpath(*relative_path.split('/'))
        copy_repository_file(source_root, source, cache_root)

    timing_count = 0
    if profile_map['profile']['collect_timings']:
        if options.timing_dir is None:
            raise ValueError('--timing-dir is required for a timing-enabled profile')
        timing_dir = options.timing_dir.resolve()
        for project in projects:
            timing_file = timing_dir / ('%s.timing' % project)
            if not timing_file.is_file():
                raise ValueError('project timing is missing: %s' % timing_file)
            copy_repository_file(source_root, timing_file, cache_root)
            timing_count += 1

    shutil.copy2(
            str(options.cache_manifest),
            str(cache_root / ('shard-manifest-%s.json' % options.group)))
    shutil.copy2(
            str(options.dependency_manifest),
            str(cache_root / ('dependency-manifest-%s.json' % options.group)))
    write_manifest(
            cache_root / ('shard-definition-%s.json' % options.group),
            artifact_shard_definition(profile_map, options.group))
    return len(staged_archives), len(dependency_manifest['dependency_files']), timing_count


def verify_profile_archives(options):
    profile_map = load_profile_map(options.profile)
    projects = read_solution_projects(options.solution_makefile)
    selected = select_projects(options.group, projects, profile_map)
    if options.group == 'all':
        outputs = [output for shard in all_shards(profile_map) for output in shard['outputs']]
    else:
        checked = find_shard(profile_map, options.group)
        outputs = checked['outputs']
        if sorted(selected) != checked['targets']:
            raise ValueError('selected projects do not match shard outputs')
    verify_archives(options.archive_dir, outputs, profile_map['profile']['archive'])
    return len(outputs)


def verify_definitions(options):
    profile_map = load_profile_map(options.profile)
    projects = read_solution_projects(options.solution_makefile)
    validate_solution_projects(projects, profile_map)
    manifest_dir = options.manifest_dir
    if not manifest_dir.is_dir():
        raise ValueError('%s: shard definition directory does not exist' % manifest_dir)
    expected_names = {
            'shard-definition-%s.json' % shard['name']
            for shard in all_shards(profile_map)}
    actual_paths = list(manifest_dir.glob('shard-definition-*.json'))
    actual_names = {path.name for path in actual_paths}
    missing = sorted(expected_names - actual_names)
    unexpected = sorted(actual_names - expected_names)
    if missing or unexpected:
        details = []
        if missing:
            details.append('missing shard definitions: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected shard definitions: %s' % ' '.join(unexpected))
        raise ValueError('; '.join(details))
    target_count = 0
    for shard in all_shards(profile_map):
        path = manifest_dir / ('shard-definition-%s.json' % shard['name'])
        actual = read_json(path, 'shard definition')
        expected = artifact_shard_definition(profile_map, shard['name'])
        if actual != expected:
            raise ValueError('%s: downloaded definition does not match checked profile map' % path)
        target_count += len(shard['targets'])
    return target_count


def verify_fallback_markers(profile_map, manifest_dir, expected_marker_path=None):
    if not manifest_dir.is_dir():
        raise ValueError('%s: fallback marker directory does not exist' % manifest_dir)
    actual_paths = sorted(manifest_dir.glob('fallback-marker-*.json'))
    if expected_marker_path is None:
        if actual_paths:
            raise ValueError('unexpected monolithic fallback markers: %s' % (
                    ' '.join(path.name for path in actual_paths)))
        return 0

    expected_names = {
            'fallback-marker-%s.json' % shard['name']
            for shard in all_shards(profile_map)}
    actual_names = {path.name for path in actual_paths}
    missing = sorted(expected_names - actual_names)
    unexpected = sorted(actual_names - expected_names)
    if missing or unexpected:
        details = []
        if missing:
            details.append('missing fallback markers: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected fallback markers: %s' % ' '.join(unexpected))
        raise ValueError('; '.join(details))

    expected = read_json(expected_marker_path, 'expected monolithic fallback marker')
    validate_monolithic_fallback_marker(expected, profile_map, expected_marker_path)
    for path in actual_paths:
        marker = read_json(path, 'monolithic fallback marker')
        validate_monolithic_fallback_marker(marker, profile_map, path)
        if marker != expected:
            raise ValueError('%s: fallback marker disagrees with final job' % path)
    return len(actual_paths)


def rounded(value):
    return value.quantize(MILLISECOND, rounding=decimal.ROUND_HALF_UP)


def json_seconds(value):
    return float(rounded(value))


def parse_identity(values):
    result = {}
    for value in values:
        if '=' not in value:
            raise ValueError('timing identity must use NAME=VALUE: %s' % value)
        name, field_value = value.split('=', 1)
        if not name or not field_value:
            raise ValueError('timing identity must use non-empty NAME=VALUE fields: %s' % value)
        if name in result:
            raise ValueError('duplicate timing identity field: %s' % name)
        result[name] = field_value
    return result


def read_timing_csv(path, solution_projects, eligible_targets):
    required_fields = {'project', 'cpu_seconds', 'exit_status'}
    result = {}
    seen = set()
    with io.open(path, 'r', encoding='utf-8', newline='') as timing_file:
        reader = csv.DictReader(timing_file)
        if reader.fieldnames is None or not required_fields.issubset(reader.fieldnames):
            raise ValueError('%s: timing CSV is missing required columns' % path)
        for line_number, row in enumerate(reader, 2):
            project = row.get('project', '')
            if not project:
                raise ValueError('%s:%d: timing project is empty' % (path, line_number))
            if project in seen:
                raise ValueError('%s:%d: duplicate timing project: %s' % (
                        path, line_number, project))
            seen.add(project)
            if project not in solution_projects:
                raise ValueError('%s:%d: project is absent from generated solution: %s' % (
                        path, line_number, project))
            try:
                exit_status = int(row['exit_status'])
            except (TypeError, ValueError) as error:
                raise ValueError('%s:%d: invalid timing value: %s' % (
                        path, line_number, error))
            if exit_status != 0:
                continue
            try:
                cpu_seconds = decimal.Decimal(row['cpu_seconds'])
                if not cpu_seconds.is_finite() or cpu_seconds < 0:
                    raise ValueError('CPU time must be non-negative')
                cpu_seconds = rounded(cpu_seconds)
            except (decimal.InvalidOperation, TypeError, ValueError) as error:
                raise ValueError('%s:%d: invalid timing value: %s' % (
                        path, line_number, error))
            if project in eligible_targets:
                result[project] = cpu_seconds
    return result


def median(values):
    ordered = sorted(values)
    middle = len(ordered) // 2
    if len(ordered) % 2:
        return ordered[middle]
    return rounded((ordered[middle - 1] + ordered[middle]) / 2)


def effective_weights(targets, timing_samples):
    successful = [sample for samples in timing_samples.values() for sample in samples]
    if not successful:
        raise ValueError('timing CSVs contain no successful driver samples')
    fallback = max(successful)
    weights = {}
    records = {}
    for target in sorted(targets):
        samples = timing_samples[target]
        weight = median(samples) if samples else fallback
        weights[target] = weight
        records[target] = {
                'sample_count': len(samples),
                'weight': json_seconds(weight)}
    return weights, records


def previous_assignment(driver_map):
    return dict(
            (target, shard['name'])
            for shard in driver_map['shards']
            for target in shard['targets'])


def partition_targets(targets, weights, jobs, previous):
    if jobs < 1 or jobs > len(targets):
        raise ValueError('--jobs must be between 1 and the number of driver targets')
    names = ['driver-%d' % index for index in range(1, jobs + 1)]
    assignments = dict((name, []) for name in names)
    loads = dict((name, decimal.Decimal('0')) for name in names)
    for target in sorted(targets, key=lambda name: (-weights[name], name)):
        minimum = min(loads.values())
        candidates = [name for name in names if loads[name] == minimum]
        preferred = previous.get(target)
        selected = preferred if preferred in candidates else candidates[0]
        assignments[selected].append(target)
        loads[selected] += weights[target]
    for name in names:
        assignments[name].sort()
        loads[name] = rounded(loads[name])
    return assignments, loads


def assignment_loads(assignments, weights):
    return dict(
            (name, rounded(sum(
                    (weights[target] for target in targets),
                    decimal.Decimal('0'))))
            for name, targets in assignments.items())


def driver_assignments(driver_map):
    return dict((shard['name'], tuple(shard['targets'])) for shard in driver_map['shards'])


def compatible_driver_map(driver_map, targets, jobs):
    if driver_map['requested_shards'] != jobs:
        return False
    previous_targets = {
            target for shard in driver_map['shards']
            for target in shard['targets']}
    return previous_targets == set(targets)


def build_driver_map(
        assignments, weights, timing_records, timing_identity, timing_sources,
        previous_maximum, proposed_maximum, archive):
    loads = assignment_loads(assignments, weights)
    shards = []
    for name in sorted(assignments, key=lambda value: int(value.split('-')[1])):
        definition = driver_shard_definition(assignments[name], archive)
        shards.append({
                'definition_hash': canonical_hash(definition),
                'name': name,
                'outputs': definition['outputs'],
                'predicted_cpu_seconds': json_seconds(loads[name]),
                'target_count': len(definition['targets']),
                'targets': definition['targets']})
    driver_map = {
            'complete_map_hash': None,
            'partition_algorithm': PARTITION_ALGORITHM,
            'pool': DRIVER_POOL,
            'predicted_maximum_cpu_seconds': json_seconds(max(loads.values())),
            'previous_maximum_cpu_seconds': (
                    None if previous_maximum is None else json_seconds(previous_maximum)),
            'proposed_maximum_cpu_seconds': json_seconds(proposed_maximum),
            'requested_shards': len(assignments),
            'schema_version': DRIVER_MAP_SCHEMA_VERSION,
            'shards': shards,
            'timing': {
                    'identity': timing_identity,
                    'metric': 'cpu_seconds',
                    'sources': timing_sources,
                    'targets': timing_records}}
    driver_map['complete_map_hash'] = canonical_hash(driver_map_identity(driver_map))
    return validate_driver_map(driver_map, '<generated driver map>', archive)


def write_profile_map(path, profile_map):
    path.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(profile_map, ensure_ascii=True, indent=2, sort_keys=True) + '\n'
    with io.open(path, 'w', encoding='utf-8', newline='\r\n') as output:
        output.write(text)


def rebalance_profile(options):
    if len(options.timing_csv) != len(options.source_run):
        raise ValueError('provide one --source-run for each --timing-csv')
    if len(options.timing_csv) > 3:
        raise ValueError('at most three timing CSVs may be used')

    profile_map = load_profile_map(options.profile)
    solution_projects = read_solution_projects(options.solution_makefile)
    fixed_projects = {
            target for shard in library_shards(profile_map)
            for target in shard['targets']}
    fixed_projects.update(profile_map['profile']['deferred_projects'])
    missing_fixed = sorted(fixed_projects - set(solution_projects))
    if missing_fixed:
        raise ValueError('generated solution is missing fixed profile projects: %s' % (
                ' '.join(missing_fixed)))
    eligible_targets = set(solution_projects) - fixed_projects
    if not eligible_targets:
        raise ValueError('generated solution contains no driver projects')

    timing_samples = dict((target, []) for target in eligible_targets)
    timing_sources = []
    for path, run_id in zip(options.timing_csv, options.source_run):
        samples = read_timing_csv(path, set(solution_projects), eligible_targets)
        for target, value in samples.items():
            timing_samples[target].append(value)
        timing_sources.append({
                'file': path.name,
                'run_id': run_id,
                'sha256': hash_file(path)})

    weights, timing_records = effective_weights(eligible_targets, timing_samples)
    previous_map = profile_map['driver_map']
    previous = previous_assignment(previous_map)
    proposed_assignments, proposed_loads = partition_targets(
            eligible_targets, weights, options.jobs, previous)
    proposed_maximum = max(proposed_loads.values())

    previous_maximum = None
    selected_assignments = proposed_assignments
    retained = False
    if compatible_driver_map(previous_map, eligible_targets, options.jobs):
        previous_assignments = driver_assignments(previous_map)
        previous_loads = assignment_loads(previous_assignments, weights)
        previous_maximum = max(previous_loads.values())
        improvement = (
                (previous_maximum - proposed_maximum) / previous_maximum
                if previous_maximum else decimal.Decimal('0'))
        if not options.force and improvement < REBALANCE_THRESHOLD:
            selected_assignments = previous_assignments
            retained = True

    updated = json.loads(json.dumps(profile_map))
    updated['driver_map'] = build_driver_map(
            selected_assignments,
            weights,
            timing_records,
            parse_identity(options.identity),
            timing_sources,
            previous_maximum,
            proposed_maximum,
            profile_map['profile']['archive'])
    updated['complete_map_hash'] = canonical_hash(profile_map_identity(updated))
    validate_profile_map(updated, str(options.profile))
    write_profile_map(options.output, updated)
    return updated, retained


def main():
    options = parse_args()
    try:
        if options.command == 'matrix':
            sys.stdout.write(json.dumps(
                    matrix_output(options), ensure_ascii=True,
                    separators=(',', ':'), sort_keys=True) + '\n')
        elif options.command == 'github-env':
            profile_id = write_github_profile_environment(options)
            sys.stdout.write('Validated fixed workflow inputs for %s.\n' % profile_id)
        elif options.command == 'validate':
            profile_map = load_profile_map(options.profile)
            projects = read_solution_projects(options.solution_makefile)
            count = validate_solution_projects(projects, profile_map)
            sys.stdout.write(
                    'Validated %d generated projects in %d shards for %s (%s).\n' % (
                            count, len(all_shards(profile_map)),
                            profile_map['profile']['id'],
                            profile_map['complete_map_hash']))
        elif options.command == 'inventory':
            inventory, report = write_generated_project_inventory(options)
            if report['has_drift']:
                sys.stdout.write(
                        'Recorded %d generated projects for %s; '
                        'detected %d added and %d removed targets.\n' % (
                                inventory['project_count'], inventory['profile'],
                                len(report['added_projects']),
                                len(report['removed_projects'])))
            else:
                sys.stdout.write(
                        'Recorded %d generated projects for %s; '
                        'the checked target manifest matches.\n' % (
                                inventory['project_count'], inventory['profile']))
        elif options.command == 'reconcile':
            profile_map, report = write_reconciled_profile(options)
            if report['recovery_mode'] == 'monolithic-fallback':
                sys.stdout.write(
                        'Selected monolithic fallback for %s: %s.\n' % (
                                report['profile'],
                                '; '.join(report['reconciliation_errors'])))
            elif report['has_drift']:
                sys.stdout.write(
                        'Reconciled generated target drift into runtime map %s for %s.\n' % (
                                profile_map['complete_map_hash'],
                                profile_map['profile']['id']))
            else:
                sys.stdout.write(
                        'Copied the exact checked profile for %s.\n' % (
                                profile_map['profile']['id']))
        elif options.command == 'list':
            profile_map = load_profile_map(options.profile)
            projects = read_solution_projects(options.solution_makefile)
            sys.stdout.write('%s\n' % ' '.join(
                    select_projects(options.group, projects, profile_map)))
        elif options.command == 'verify-archives':
            count = verify_profile_archives(options)
            sys.stdout.write('Validated %d archives for %s.\n' % (
                    count, options.group))
        elif options.command == 'write-definition':
            profile_map = load_profile_map(options.profile)
            projects = read_solution_projects(options.solution_makefile)
            select_projects(options.group, projects, profile_map)
            write_manifest(
                    options.output,
                    artifact_shard_definition(profile_map, options.group))
            sys.stdout.write('Wrote shard definition for %s.\n' % options.group)
        elif options.command == 'verify-definitions':
            count = verify_definitions(options)
            sys.stdout.write('Validated %d downloaded targets for %s.\n' % (
                    count, load_profile_map(options.profile)['profile']['id']))
        elif options.command == 'verify-fallback':
            profile_map = load_profile_map(options.profile)
            count = verify_fallback_markers(
                    profile_map, options.manifest_dir, options.expected_marker)
            if options.expected_marker is None:
                sys.stdout.write('Validated sharded build mode for %s.\n' % (
                        profile_map['profile']['id']))
            else:
                sys.stdout.write(
                        'Validated %d identical monolithic fallback markers for %s.\n' % (
                                count, profile_map['profile']['id']))
        elif options.command == 'rebalance':
            profile_map, retained = rebalance_profile(options)
            action = 'Retained' if retained else 'Selected'
            driver_map = profile_map['driver_map']
            sys.stdout.write(
                    '%s %d driver shards; predicted maximum %.3f CPU seconds; map %s.\n' % (
                            action, driver_map['requested_shards'],
                            driver_map['predicted_maximum_cpu_seconds'],
                            profile_map['complete_map_hash']))
            for shard in driver_map['shards']:
                sys.stdout.write('%s: %.3f CPU seconds, %d targets\n' % (
                        shard['name'], shard['predicted_cpu_seconds'],
                        shard['target_count']))
        elif options.command == 'context':
            sys.stdout.write('%s\n' % dependency_cache_prefix(options))
        elif options.command == 'prepare':
            sys.stdout.write('%s\n' % prepare_cache(options))
        elif options.command == 'materialize':
            dependency_count, status = materialize_dependencies(options)
            if status == 'materialized':
                sys.stdout.write('Materialized %d generated dependencies for %s.\n' % (
                        dependency_count, options.group))
            elif status == 'current':
                sys.stdout.write('All dependencies for %s are already materialized.\n' % options.group)
            else:
                sys.stdout.write(
                        'Generated dependency materialization skipped for %s (%s).\n' %
                        (options.group, status))
        elif options.command == 'capture':
            file_count, dependency_count = capture_dependencies(options)
            sys.stdout.write('Captured %d compiler dependency files with %d inputs for %s.\n' % (
                    file_count, dependency_count, options.group))
        elif options.command == 'stage':
            archive_count, dependency_count, timing_count = stage_artifacts(options)
            sys.stdout.write(
                    'Staged %d archives, %d dependency files, and %d timings for %s.\n' % (
                            archive_count, dependency_count, timing_count,
                            options.group))
        elif options.command == 'verify':
            project_count, dependency_count, source_commit = verify_cache(options)
            sys.stdout.write(
                    'Validated %d cached archives and %d compiler dependency files for %s from %s.\n' % (
                            project_count, dependency_count, options.group, source_commit))
        else:
            raise ValueError('unsupported command: %s' % options.command)
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error managing CI shard cache: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
