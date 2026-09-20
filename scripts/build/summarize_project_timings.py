#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import csv
import io
import pathlib
import sys


COMMON_PROJECTS = frozenset((
        'emu',
        'formats',
        'frontend',
        'netlist',
        'precompile',
        'softfloat3',
        'utils',
        'wdlfft',
        'ymfm'))

DEVICE_PROJECTS = frozenset((
        'dasm',
        'optional'))


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input-dir', type=pathlib.Path, required=True, help='directory containing project timing files')
    parser.add_argument('--csv', type=pathlib.Path, required=True, help='output CSV path')
    parser.add_argument('--markdown', type=pathlib.Path, required=True, help='output Markdown summary path')
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'), help='MAME source root')
    parser.add_argument('--driver-limit', type=int, default=50, help='maximum driver projects in the Markdown summary')
    return parser.parse_args()


def project_category(project, source_root):
    if (source_root / 'src' / 'mame' / project).is_dir():
        return 'driver'
    elif project in COMMON_PROJECTS:
        return 'common'
    elif project in DEVICE_PROJECTS:
        return 'device'
    else:
        return 'other'


def read_project_timing(path, source_root):
    elapsed = 0.0
    user = 0.0
    system = 0.0
    max_rss = 0
    exit_status = 0
    invocations = 0

    with io.open(path, 'r', encoding='utf-8') as timing_file:
        for line_number, line in enumerate(timing_file, 1):
            fields = line.split()
            if not fields:
                continue
            if len(fields) != 5:
                raise ValueError('%s:%d: expected five timing fields' % (path, line_number))
            try:
                record_elapsed = float(fields[0])
                record_user = float(fields[1])
                record_system = float(fields[2])
                record_max_rss = int(fields[3])
                record_exit_status = int(fields[4])
            except ValueError as error:
                raise ValueError('%s:%d: invalid timing value: %s' % (path, line_number, error))

            elapsed += record_elapsed
            user += record_user
            system += record_system
            max_rss = max(max_rss, record_max_rss)
            exit_status = max(exit_status, record_exit_status)
            invocations += 1

    if not invocations:
        raise ValueError('%s: no timing records found' % path)

    project = path.stem
    return {
            'project': project,
            'category': project_category(project, source_root),
            'invocations': invocations,
            'elapsed_seconds': elapsed,
            'user_seconds': user,
            'system_seconds': system,
            'cpu_seconds': user + system,
            'max_rss_kb': max_rss,
            'exit_status': exit_status}


def load_timings(input_dir, source_root):
    paths = sorted(input_dir.glob('*.timing'))
    if not paths:
        raise ValueError('%s: no project timing files found' % input_dir)
    result = [read_project_timing(path, source_root) for path in paths]
    result.sort(key=lambda row: (-row['cpu_seconds'], -row['elapsed_seconds'], row['project']))
    return result


def write_csv(path, rows):
    fieldnames = (
            'project',
            'category',
            'invocations',
            'elapsed_seconds',
            'user_seconds',
            'system_seconds',
            'cpu_seconds',
            'max_rss_kb',
            'exit_status')
    with io.open(path, 'w', encoding='utf-8', newline='') as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            formatted = dict(row)
            for field in ('elapsed_seconds', 'user_seconds', 'system_seconds', 'cpu_seconds'):
                formatted[field] = '%.3f' % row[field]
            writer.writerow(formatted)


def write_table(output, rows):
    output.write('| Project | Type | Runs | Elapsed (s) | CPU (s) | Max RSS (MiB) |\n')
    output.write('| --- | --- | ---: | ---: | ---: | ---: |\n')
    for row in rows:
        output.write('| %s | %s | %d | %.3f | %.3f | %.1f |\n' % (
                row['project'],
                row['category'],
                row['invocations'],
                row['elapsed_seconds'],
                row['cpu_seconds'],
                row['max_rss_kb'] / 1024.0))


def write_markdown(path, rows, driver_limit):
    library_rows = [row for row in rows if row['category'] in ('common', 'device')]
    driver_rows = [row for row in rows if row['category'] == 'driver']
    other_rows = [row for row in rows if row['category'] == 'other']

    with io.open(path, 'w', encoding='utf-8', newline='\n') as output:
        output.write('# MAME generated project timings\n\n')
        output.write('Measured %d generated projects. Projects run in parallel, so elapsed '
                'values are not additive. CPU time is user plus system time and is the '
                'preferred initial weight for balancing build shards.\n\n' % len(rows))

        output.write('## Common and device projects\n\n')
        write_table(output, library_rows)

        output.write('\n## Slowest driver projects\n\n')
        write_table(output, driver_rows[:driver_limit])

        output.write('\n## Slowest other projects\n\n')
        write_table(output, other_rows[:20])


def main():
    options = parse_args()
    try:
        rows = load_timings(options.input_dir, options.source_root)
        write_csv(options.csv, rows)
        write_markdown(options.markdown, rows, options.driver_limit)
    except (IOError, OSError, ValueError) as error:
        sys.stderr.write('Error collecting project timings: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
