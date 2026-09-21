#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import collections
import decimal
import hashlib
import io
import json
import pathlib
import sys


PROJECTS_PREFIX = 'PROJECTS := '
EXECUTABLE_PROJECT = 'mame'
THIRDPARTY_PROJECTS = (
        '7z', 'asmjit', 'bgfx', 'bimg', 'bx', 'expat', 'flac', 'jpeg', 'linenoise', 'lua', 'lualibs',
        'portaudio', 'portmidi', 'softfloat3', 'sqlite3', 'utf8proc', 'wdlfft', 'ymfm', 'zlib', 'zstd')
CORE_PROJECTS = (
        'dasm', 'emu', 'formats', 'frontend', 'netlist', 'ocore_sdl', 'osd_sdl', 'precompile', 'qtdbg_sdl',
        'utils')
OPTIONAL_PROJECTS = ('optional',)
TOOL_PROJECTS = (
        'castool', 'chdman', 'floptool', 'imgtool', 'jedutil', 'ldresample', 'ldverify', 'nltool', 'nlwav',
        'pngcmp', 'regrep', 'romcmp', 'split', 'srcclean', 'testkeys', 'unidasm')

LIBRARY_GROUPS = {
        'core': CORE_PROJECTS,
        'optional': OPTIONAL_PROJECTS,
        'thirdparty': THIRDPARTY_PROJECTS}

SHARD_MAP_SCHEMA_VERSION = 1
SHARD_DEFINITION_SCHEMA_VERSION = 1
PARTITION_ALGORITHM = 'lpt-v1'
DRIVER_POOL = 'driver-libs'
DEFAULT_SHARD_MAP_PATH = (
        pathlib.Path(__file__).resolve().parents[2] / '.github' / 'ci-linux-clang-driver-shards.json')
MILLISECOND = decimal.Decimal('0.001')


def canonical_json(value):
    return (json.dumps(value, ensure_ascii=True, separators=(',', ':'), sort_keys=True) + '\n').encode('utf-8')


def canonical_hash(value):
    return hashlib.sha256(canonical_json(value)).hexdigest()


def driver_shard_definition(targets):
    sorted_targets = sorted(targets)
    return {
            'outputs': ['lib%s.a' % target for target in sorted_targets],
            'pool': DRIVER_POOL,
            'schema_version': SHARD_DEFINITION_SCHEMA_VERSION,
            'targets': sorted_targets}


def complete_map_identity(shard_map):
    return dict((key, value) for key, value in shard_map.items() if key != 'complete_map_hash')


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


def read_json(path, description):
    with io.open(path, 'r', encoding='utf-8') as source:
        value = json.load(source)
    if not isinstance(value, dict):
        raise ValueError('%s: expected a JSON object for %s' % (path, description))
    return value


def find_driver_shard(shard_map, name):
    for shard in shard_map['shards']:
        if shard['name'] == name:
            return shard
    raise ValueError('unknown driver shard: %s' % name)


def validate_shard_map(shard_map, path='<shard map>'):
    expected_fields = {
            'complete_map_hash',
            'partition_algorithm',
            'pool',
            'predicted_maximum_cpu_seconds',
            'previous_maximum_cpu_seconds',
            'proposed_maximum_cpu_seconds',
            'requested_shards',
            'schema_version',
            'shards',
            'timing'}
    if set(shard_map) != expected_fields:
        raise ValueError('%s: shard map fields do not match schema' % path)
    if shard_map.get('schema_version') != SHARD_MAP_SCHEMA_VERSION:
        raise ValueError('%s: shard map schema does not match' % path)
    if shard_map.get('partition_algorithm') != PARTITION_ALGORITHM:
        raise ValueError('%s: partition algorithm does not match' % path)
    if shard_map.get('pool') != DRIVER_POOL:
        raise ValueError('%s: driver pool does not match' % path)

    requested = shard_map.get('requested_shards')
    shards = shard_map.get('shards')
    if isinstance(requested, bool) or not isinstance(requested, int) or requested < 1:
        raise ValueError('%s: requested shard count is invalid' % path)
    if not isinstance(shards, list) or len(shards) != requested:
        raise ValueError('%s: shard list does not match requested count' % path)

    timing = shard_map.get('timing')
    if not isinstance(timing, dict) or set(timing) != {'identity', 'metric', 'sources', 'targets'}:
        raise ValueError('%s: timing metadata does not match schema' % path)
    if timing.get('metric') != 'cpu_seconds':
        raise ValueError('%s: timing metric must be cpu_seconds' % path)
    identity = timing.get('identity')
    if not isinstance(identity, dict) or not identity:
        raise ValueError('%s: timing identity is missing' % path)
    for key, value in identity.items():
        if not isinstance(key, str) or not key or not isinstance(value, str) or not value:
            raise ValueError('%s: timing identity entries must be non-empty strings' % path)
    sources = timing.get('sources')
    if not isinstance(sources, list) or not sources or len(sources) > 3:
        raise ValueError('%s: timing sources must contain one to three entries' % path)
    for source in sources:
        if not isinstance(source, dict) or set(source) != {'file', 'run_id', 'sha256'}:
            raise ValueError('%s: timing source does not match schema' % path)
        if not all(isinstance(source[field], str) and source[field] for field in ('file', 'run_id')):
            raise ValueError('%s: timing source fields must be non-empty strings' % path)
        digest = source.get('sha256')
        if not isinstance(digest, str) or len(digest) != 64:
            raise ValueError('%s: timing source SHA-256 is invalid' % path)
        try:
            int(digest, 16)
        except ValueError:
            raise ValueError('%s: timing source SHA-256 is invalid' % path)

    timing_targets = timing.get('targets')
    if not isinstance(timing_targets, dict) or not timing_targets:
        raise ValueError('%s: per-target timing weights are missing' % path)
    weights = {}
    for target, record in timing_targets.items():
        if not isinstance(target, str) or not target:
            raise ValueError('%s: timing target name is invalid' % path)
        if not isinstance(record, dict) or set(record) != {'sample_count', 'weight'}:
            raise ValueError('%s: timing record for %s does not match schema' % (path, target))
        count = record.get('sample_count')
        if isinstance(count, bool) or not isinstance(count, int) or not 0 <= count <= 3:
            raise ValueError('%s: timing sample count for %s is invalid' % (path, target))
        weights[target] = number_value(record.get('weight'), 'timing weight for %s' % target)

    all_targets = []
    predicted_weights = []
    expected_names = ['driver-%d' % index for index in range(1, requested + 1)]
    actual_names = []
    for shard in shards:
        if not isinstance(shard, dict):
            raise ValueError('%s: shard entry is not an object' % path)
        expected_shard_fields = {
                'definition_hash', 'name', 'outputs', 'predicted_cpu_seconds',
                'target_count', 'targets'}
        if set(shard) != expected_shard_fields:
            raise ValueError('%s: shard fields do not match schema' % path)
        name = shard.get('name')
        actual_names.append(name)
        targets = shard.get('targets')
        if (
                not isinstance(targets, list)
                or not targets
                or any(not isinstance(target, str) or not target for target in targets)
                or targets != sorted(set(targets))):
            raise ValueError('%s: targets for %s are not sorted and unique' % (path, name))
        if shard.get('target_count') != len(targets):
            raise ValueError('%s: target count for %s does not match' % (path, name))
        definition = driver_shard_definition(targets)
        if shard.get('outputs') != definition['outputs']:
            raise ValueError('%s: outputs for %s do not match targets' % (path, name))
        if shard.get('definition_hash') != canonical_hash(definition):
            raise ValueError('%s: definition hash for %s does not match' % (path, name))
        missing_weights = sorted(set(targets) - set(weights))
        if missing_weights:
            raise ValueError('%s: %s has targets without timing weights: %s' % (
                    path, name, ' '.join(missing_weights)))
        predicted = number_value(
                shard.get('predicted_cpu_seconds'), 'predicted weight for %s' % name)
        calculated = sum((weights[target] for target in targets), decimal.Decimal('0')).quantize(MILLISECOND)
        if predicted != calculated:
            raise ValueError('%s: predicted weight for %s does not match targets' % (path, name))
        predicted_weights.append(predicted)
        all_targets.extend(targets)

    if actual_names != expected_names:
        raise ValueError('%s: shard names are not canonical' % path)
    duplicates = sorted(
            target for target, count in collections.Counter(all_targets).items() if count != 1)
    if duplicates:
        raise ValueError('%s: targets assigned more than once: %s' % (path, ' '.join(duplicates)))
    if sorted(all_targets) != sorted(timing_targets):
        raise ValueError('%s: assigned targets do not match timing targets' % path)

    previous = number_value(
            shard_map.get('previous_maximum_cpu_seconds'),
            'previous maximum weight',
            allow_none=True)
    proposed = number_value(shard_map.get('proposed_maximum_cpu_seconds'), 'proposed maximum weight')
    predicted = number_value(shard_map.get('predicted_maximum_cpu_seconds'), 'predicted maximum weight')
    if predicted != max(predicted_weights):
        raise ValueError('%s: predicted maximum does not match shard weights' % path)
    if predicted != proposed and predicted != previous:
        raise ValueError('%s: selected map is neither the proposed nor previous partition' % path)

    expected_hash = canonical_hash(complete_map_identity(shard_map))
    if shard_map.get('complete_map_hash') != expected_hash:
        raise ValueError('%s: complete map hash does not match' % path)
    return shard_map


def load_shard_map(path=DEFAULT_SHARD_MAP_PATH):
    return validate_shard_map(read_json(path, 'driver shard map'), path)


def driver_groups(shard_map=None):
    if shard_map is None:
        shard_map = load_shard_map()
    return tuple(shard['name'] for shard in shard_map['shards'])


def driver_buckets(shard_map=None):
    if shard_map is None:
        shard_map = load_shard_map()
    return dict((shard['name'], tuple(shard['targets'])) for shard in shard_map['shards'])


def driver_shard_artifact_definition(shard_map, name):
    shard = find_driver_shard(shard_map, name)
    return {
            'complete_map_hash': shard_map['complete_map_hash'],
            'definition': driver_shard_definition(shard['targets']),
            'definition_hash': shard['definition_hash'],
            'partition_algorithm': shard_map['partition_algorithm'],
            'shard': name}


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(str(path), 'wb') as output:
        output.write(canonical_json(value))


def add_common_arguments(parser):
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))
    parser.add_argument('--shard-map', type=pathlib.Path, default=DEFAULT_SHARD_MAP_PATH)


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', required=True)

    validate_parser = subparsers.add_parser('validate')
    add_common_arguments(validate_parser)

    list_parser = subparsers.add_parser('list')
    add_common_arguments(list_parser)
    list_parser.add_argument('--group', required=True)

    archives_parser = subparsers.add_parser('verify-archives')
    add_common_arguments(archives_parser)
    archives_parser.add_argument('--group', required=True)
    archives_parser.add_argument('--archive-dir', required=True, type=pathlib.Path)

    definition_parser = subparsers.add_parser('write-definition')
    add_common_arguments(definition_parser)
    definition_parser.add_argument('--group', required=True)
    definition_parser.add_argument('--output', required=True, type=pathlib.Path)

    definitions_parser = subparsers.add_parser('verify-definitions')
    add_common_arguments(definitions_parser)
    definitions_parser.add_argument('--manifest-dir', required=True, type=pathlib.Path)

    return parser.parse_args()


def read_solution_projects(path):
    with io.open(path, 'r', encoding='utf-8') as solution_makefile:
        project_lines = [line.rstrip('\r\n') for line in solution_makefile if line.startswith(PROJECTS_PREFIX)]
    if len(project_lines) != 1:
        raise ValueError('%s: expected one PROJECTS definition' % path)
    projects = project_lines[0][len(PROJECTS_PREFIX):].split()
    if not projects:
        raise ValueError('%s: generated solution contains no projects' % path)
    duplicates = sorted(project for project, count in collections.Counter(projects).items() if count != 1)
    if duplicates:
        raise ValueError('%s: duplicate generated projects: %s' % (path, ' '.join(duplicates)))
    return projects


def solution_driver_projects(projects, source_root):
    generated_drivers = {
            project for project in projects
            if (source_root / 'src' / 'mame' / project).is_dir()}
    if EXECUTABLE_PROJECT not in projects:
        raise ValueError('generated solution does not contain the %s executable project' % EXECUTABLE_PROJECT)

    library_assignments = [project for group in LIBRARY_GROUPS.values() for project in group]
    duplicates = sorted(
            project for project, count in collections.Counter(library_assignments).items() if count != 1)
    if duplicates:
        raise ValueError('projects assigned to multiple library groups: %s' % ' '.join(duplicates))

    generated_projects = set(projects)
    missing_libraries = sorted(set(library_assignments) - generated_projects)
    missing_tools = sorted(set(TOOL_PROJECTS) - generated_projects)
    classified_projects = generated_drivers | set(library_assignments) | set(TOOL_PROJECTS) | {EXECUTABLE_PROJECT}
    unassigned_projects = sorted(generated_projects - classified_projects)
    if missing_libraries or missing_tools or unassigned_projects:
        details = []
        if missing_libraries:
            details.append('library projects absent from generated solution: %s' % ' '.join(missing_libraries))
        if missing_tools:
            details.append('tool projects absent from generated solution: %s' % ' '.join(missing_tools))
        if unassigned_projects:
            details.append('unassigned generated projects: %s' % ' '.join(unassigned_projects))
        raise ValueError('; '.join(details))
    return generated_drivers


def validate_buckets(projects, source_root, shard_map=None):
    if shard_map is None:
        shard_map = load_shard_map()
    generated_drivers = solution_driver_projects(projects, source_root)
    assignments = [project for bucket in driver_buckets(shard_map).values() for project in bucket]
    duplicates = sorted(project for project, count in collections.Counter(assignments).items() if count != 1)
    assigned_drivers = set(assignments)
    missing = sorted(generated_drivers - assigned_drivers)
    extra = sorted(assigned_drivers - generated_drivers)
    if duplicates or missing or extra:
        details = []
        if duplicates:
            details.append('projects assigned to multiple shards: %s' % ' '.join(duplicates))
        if missing:
            details.append('unassigned generated drivers: %s' % ' '.join(missing))
        if extra:
            details.append('assigned projects absent from generated drivers: %s' % ' '.join(extra))
        raise ValueError('; '.join(details))
    return generated_drivers


def select_projects(group, projects, generated_drivers, shard_map=None):
    if shard_map is None:
        shard_map = load_shard_map()
    buckets = driver_buckets(shard_map)
    if group in LIBRARY_GROUPS:
        selected_projects = set(LIBRARY_GROUPS[group])
        return [project for project in projects if project in selected_projects]
    if group == 'tools':
        return [project for project in projects if project in TOOL_PROJECTS]
    if group == 'all':
        return [project for name in driver_groups(shard_map) for project in buckets[name]]
    if group in buckets:
        return list(buckets[group])
    raise ValueError('unknown project group: %s' % group)


def verify_archives(archive_dir, selected_projects, generated_drivers):
    if not archive_dir.is_dir():
        raise ValueError('%s: driver archive directory does not exist' % archive_dir)

    missing = sorted(
            project for project in selected_projects
            if not (archive_dir / ('lib%s.a' % project)).is_file())
    actual_drivers = {
            path.name[3:-2] for path in archive_dir.glob('lib*.a')
            if path.name[3:-2] in generated_drivers}
    unexpected = sorted(actual_drivers - set(selected_projects))
    if missing or unexpected:
        details = []
        if missing:
            details.append('missing driver archives: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected driver archives: %s' % ' '.join(unexpected))
        raise ValueError('; '.join(details))


def verify_definitions(manifest_dir, shard_map, generated_drivers):
    if not manifest_dir.is_dir():
        raise ValueError('%s: shard definition directory does not exist' % manifest_dir)
    expected_names = {
            'driver-shard-definition-%s.json' % name for name in driver_groups(shard_map)}
    actual_paths = list(manifest_dir.glob('driver-shard-definition-*.json'))
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

    targets = []
    for name in driver_groups(shard_map):
        path = manifest_dir / ('driver-shard-definition-%s.json' % name)
        actual = read_json(path, 'driver shard definition')
        expected = driver_shard_artifact_definition(shard_map, name)
        if actual != expected:
            raise ValueError('%s: downloaded shard definition does not match checked map' % path)
        targets.extend(actual['definition']['targets'])

    duplicates = sorted(target for target, count in collections.Counter(targets).items() if count != 1)
    missing_targets = sorted(set(generated_drivers) - set(targets))
    extra_targets = sorted(set(targets) - set(generated_drivers))
    if duplicates or missing_targets or extra_targets:
        details = []
        if duplicates:
            details.append('duplicate downloaded targets: %s' % ' '.join(duplicates))
        if missing_targets:
            details.append('missing downloaded targets: %s' % ' '.join(missing_targets))
        if extra_targets:
            details.append('unexpected downloaded targets: %s' % ' '.join(extra_targets))
        raise ValueError('; '.join(details))
    return len(targets)


def main():
    options = parse_args()
    try:
        shard_map = load_shard_map(options.shard_map)
        projects = read_solution_projects(options.solution_makefile)
        generated_drivers = validate_buckets(projects, options.source_root, shard_map)
        if options.command == 'validate':
            sys.stdout.write(
                    'Validated %d driver projects in %d shards and %d library projects in %d groups '
                    'using map %s.\n' % (
                            len(generated_drivers), len(shard_map['shards']),
                            sum(len(group) for group in LIBRARY_GROUPS.values()), len(LIBRARY_GROUPS),
                            shard_map['complete_map_hash']))
        elif options.command == 'write-definition':
            if options.group not in driver_groups(shard_map):
                raise ValueError('shard definition requested for non-driver group: %s' % options.group)
            write_json(options.output, driver_shard_artifact_definition(shard_map, options.group))
            sys.stdout.write('Wrote driver shard definition for %s.\n' % options.group)
        elif options.command == 'verify-definitions':
            count = verify_definitions(options.manifest_dir, shard_map, generated_drivers)
            sys.stdout.write('Validated %d downloaded driver targets against map %s.\n' % (
                    count, shard_map['complete_map_hash']))
        else:
            selected_projects = select_projects(options.group, projects, generated_drivers, shard_map)
            if options.command == 'list':
                sys.stdout.write('%s\n' % ' '.join(selected_projects))
            else:
                verify_archives(options.archive_dir, selected_projects, generated_drivers)
                sys.stdout.write('Validated %d driver archives for %s.\n' % (
                        len(selected_projects), options.group))
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error validating CI driver shards: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
