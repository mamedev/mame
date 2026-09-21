#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import csv
import decimal
import hashlib
import io
import json
import pathlib
import sys

import ci_linux_clang_driver_buckets as shard_config


MILLISECOND = decimal.Decimal('0.001')
REBALANCE_THRESHOLD = decimal.Decimal('0.05')


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))
    parser.add_argument('--timing-csv', action='append', required=True, type=pathlib.Path)
    parser.add_argument('--source-run', action='append', required=True)
    parser.add_argument('--identity', action='append', required=True, metavar='NAME=VALUE')
    parser.add_argument('--jobs', required=True, type=int)
    parser.add_argument('--output', required=True, type=pathlib.Path)
    parser.add_argument('--previous-map', type=pathlib.Path)
    parser.add_argument('--force', action='store_true')
    return parser.parse_args()


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


def hash_file(path):
    digest = hashlib.sha256()
    with open(str(path), 'rb') as source:
        while True:
            block = source.read(1024 * 1024)
            if not block:
                break
            digest.update(block)
    return digest.hexdigest()


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
                raise ValueError('%s:%d: duplicate timing project: %s' % (path, line_number, project))
            seen.add(project)
            if project not in solution_projects:
                raise ValueError('%s:%d: project is absent from generated solution: %s' % (
                        path, line_number, project))
            try:
                exit_status = int(row['exit_status'])
            except (TypeError, ValueError) as error:
                raise ValueError('%s:%d: invalid timing value: %s' % (path, line_number, error))
            if exit_status != 0:
                continue
            try:
                cpu_seconds = decimal.Decimal(row['cpu_seconds'])
                if not cpu_seconds.is_finite() or cpu_seconds < 0:
                    raise ValueError('CPU time must be non-negative')
                cpu_seconds = rounded(cpu_seconds)
            except (decimal.InvalidOperation, TypeError, ValueError) as error:
                raise ValueError('%s:%d: invalid timing value: %s' % (path, line_number, error))
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


def previous_assignment(shard_map):
    if shard_map is None:
        return {}
    return dict(
            (target, shard['name'])
            for shard in shard_map['shards']
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
            (name, rounded(sum((weights[target] for target in targets), decimal.Decimal('0'))))
            for name, targets in assignments.items())


def compatible_previous_map(shard_map, targets, jobs):
    if shard_map is None or shard_map['requested_shards'] != jobs:
        return False
    previous_targets = {
            target for shard in shard_map['shards'] for target in shard['targets']}
    return previous_targets == set(targets)


def build_shard_map(
        assignments, weights, timing_records, timing_identity, timing_sources,
        previous_maximum, proposed_maximum):
    loads = assignment_loads(assignments, weights)
    shards = []
    for name in sorted(assignments, key=lambda value: int(value.split('-')[1])):
        definition = shard_config.driver_shard_definition(assignments[name])
        shards.append({
                'definition_hash': shard_config.canonical_hash(definition),
                'name': name,
                'outputs': definition['outputs'],
                'predicted_cpu_seconds': json_seconds(loads[name]),
                'target_count': len(definition['targets']),
                'targets': definition['targets']})
    shard_map = {
            'complete_map_hash': None,
            'partition_algorithm': shard_config.PARTITION_ALGORITHM,
            'pool': shard_config.DRIVER_POOL,
            'predicted_maximum_cpu_seconds': json_seconds(max(loads.values())),
            'previous_maximum_cpu_seconds': (
                    None if previous_maximum is None else json_seconds(previous_maximum)),
            'proposed_maximum_cpu_seconds': json_seconds(proposed_maximum),
            'requested_shards': len(assignments),
            'schema_version': shard_config.SHARD_MAP_SCHEMA_VERSION,
            'shards': shards,
            'timing': {
                    'identity': timing_identity,
                    'metric': 'cpu_seconds',
                    'sources': timing_sources,
                    'targets': timing_records}}
    shard_map['complete_map_hash'] = shard_config.canonical_hash(
            shard_config.complete_map_identity(shard_map))
    return shard_config.validate_shard_map(shard_map)


def write_shard_map(path, shard_map):
    path.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(shard_map, ensure_ascii=True, indent=2, sort_keys=True) + '\n'
    with io.open(path, 'w', encoding='utf-8', newline='\r\n') as output:
        output.write(text)


def rebalance(options):
    if len(options.timing_csv) != len(options.source_run):
        raise ValueError('provide one --source-run for each --timing-csv')
    if len(options.timing_csv) > 3:
        raise ValueError('at most three timing CSVs may be used')

    source_root = options.source_root.resolve()
    projects = shard_config.read_solution_projects(options.solution_makefile)
    generated_drivers = shard_config.solution_driver_projects(projects, source_root)
    timing_samples = dict((target, []) for target in generated_drivers)
    timing_sources = []
    solution_projects = set(projects)
    for path, run_id in zip(options.timing_csv, options.source_run):
        samples = read_timing_csv(path, solution_projects, generated_drivers)
        for target, value in samples.items():
            timing_samples[target].append(value)
        timing_sources.append({
                'file': path.name,
                'run_id': run_id,
                'sha256': hash_file(path)})

    weights, timing_records = effective_weights(generated_drivers, timing_samples)
    previous_map = (
            shard_config.load_shard_map(options.previous_map)
            if options.previous_map is not None else None)
    previous = previous_assignment(previous_map)
    proposed_assignments, proposed_loads = partition_targets(
            generated_drivers, weights, options.jobs, previous)
    proposed_maximum = max(proposed_loads.values())

    previous_maximum = None
    selected_assignments = proposed_assignments
    retained = False
    if compatible_previous_map(previous_map, generated_drivers, options.jobs):
        previous_assignments = shard_config.driver_buckets(previous_map)
        previous_loads = assignment_loads(previous_assignments, weights)
        previous_maximum = max(previous_loads.values())
        improvement = (
                (previous_maximum - proposed_maximum) / previous_maximum
                if previous_maximum else decimal.Decimal('0'))
        if not options.force and improvement < REBALANCE_THRESHOLD:
            selected_assignments = previous_assignments
            retained = True

    shard_map = build_shard_map(
            selected_assignments,
            weights,
            timing_records,
            parse_identity(options.identity),
            timing_sources,
            previous_maximum,
            proposed_maximum)
    write_shard_map(options.output, shard_map)
    return shard_map, retained


def main():
    options = parse_args()
    try:
        shard_map, retained = rebalance(options)
        action = 'Retained' if retained else 'Selected'
        sys.stdout.write('%s %d driver shards; predicted maximum %.3f CPU seconds; map %s.\n' % (
                action,
                shard_map['requested_shards'],
                shard_map['predicted_maximum_cpu_seconds'],
                shard_map['complete_map_hash']))
        for shard in shard_map['shards']:
            sys.stdout.write('%s: %.3f CPU seconds, %d targets\n' % (
                    shard['name'], shard['predicted_cpu_seconds'], shard['target_count']))
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error rebalancing CI driver shards: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
