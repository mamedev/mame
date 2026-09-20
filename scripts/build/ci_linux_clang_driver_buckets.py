#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import collections
import io
import pathlib
import sys


PROJECTS_PREFIX = 'PROJECTS := '
EXECUTABLE_PROJECT = 'mame'
THIRDPARTY_PROJECTS = (
        '7z', 'asmjit', 'bgfx', 'bimg', 'bx', 'expat', 'flac', 'jpeg', 'linenoise', 'lua', 'lualibs',
        'portaudio', 'portmidi', 'softfloat3', 'sqlite3', 'utf8proc', 'wdlfft', 'ymfm', 'zlib', 'zstd')
TOOL_PROJECTS = (
        'castool', 'chdman', 'floptool', 'imgtool', 'jedutil', 'ldresample', 'ldverify', 'nltool', 'nlwav',
        'pngcmp', 'regrep', 'romcmp', 'split', 'srcclean', 'testkeys', 'unidasm')

BUCKETS = {
        'driver-1': (
            'misc', 'acorn', 'saitek', 'ausnz', 'homebrew', 'msx', 'dynax', 'amiga', 'olivetti', 'orca',
            'wyse', 'nokia', 'tangerine', 'hitachi', 'mattel', 'tesla', 'mips', 'apollo', 'osborne', 'emusys',
            'quantel', 'ta', 'rare', 'telercas', 'comad', 'modelracing', 'kyocera', 'esprit', 'torch', 'kaypro',
            'wicat', 'positron', 'tigertel', 'apf', 'nakajima', 'ceres', 'novation', 'slicer', 'benesse',
            'unisonic', 'grundy', 'banctec'),
        'driver-2': (
            'atari', 'igs', 'jaleco', 'hp', 'bally', 'ti', 'ibm', 'newcrest', 'maygay', 'heathzenith', 'cave',
            'sun', 'merit', 'ramtek', 'unico', 'virtual', 'alliedleisure', 'informer', 'adp', 'epson', 'yachiyo',
            'olympia', 'alesis', 'recfranco', 'nix', 'tandberg', 'ampex', 'rca', 'conitec', 'omori', 'entex', 'mc',
            'kontron', 'bordun', 'linn', 'videoton', 'tiki', 'camputers', 'chromatics', 'beehive', 'verifone',
            'ccs', 'pitronics', 'force'),
        'driver-3': (
            'sega', 'nec', 'sony', 'sgi', 'vtech', 'vsystem', 'jpm', 'midw8080', 'excellent', 'sfrj', 'amstrad',
            'wing', 'luxor', 'playmark', 'ncd', 'miltonbradley', 'aristocrat', 'siemens', 'att', 'intergraph',
            'brother', 'meadows', 'bondwell', 'umc', 'votrax', 'microsoft', 'epoch', 'midcoin', 'gridcomp', 'osi',
            'rockwell', 'dai', 'mg1', 'dms', 'adc', 'bitcorp', 'compugraphic', 'cromemco', 'gametron', 'isc',
            'mchester', 'drc', 'tms', 'elektron'),
        'driver-4': (
            'skeleton', 'barcrest', 'nichibutsu', 'seta', 'toaplan', 'ddr', 'technos', 'trainer', 'philips',
            'visual', 'funworld', 'atlus', 'matsushita', 'tch', 'efo', 'success', 'oberheim', 'yunsung', 'psikyo',
            'astrocorp', 'multitech', 'nasco', 'lsi', 'valadon', 'natsemi', 'amcoe', 'enterprise', 'f32', 'tryom',
            'morrow', 'zvt', 'huangyeh', 'interton', 'access', 'microcraft', 'cosmo', 'rolm', 'vidbrain', 'cce',
            'colonial', 'mupid', 'unicard', 'zilog', 'saturn'),
        'driver-5': (
            'taito', 'nintendo', 'tvgames', 'sinclair', 'gaelco', 'trs', 'fidelity', 'kaneko', 'irem', 'tatsumi',
            'televideo', 'zaccaria', 'tektronix', 'sanritsu', 'sunelectronics', 'tiger', 'toshiba', 'ericsson',
            'eolith', 'pce', 'fuuki', 'phoenix', 'learsiegler', 'dgrm', 'sunwise', 'hec2hrp', 'next', 'liberty',
            'ces', 'paia', 'ncr', 'swtpc', 'stm', 'chessking', 'tomy', 'fairchild', 'telenova', 'tatung',
            'novadesitec', 'seequa', 'comx', 'kyber', 'unisys', 'amirix', 'teamconcepts', 'ultratec'),
        'driver-6': (
            'namco', 'cinematronics', 'capcom', 'seibu', 'sharp', 'exidy', 'hegenerglaser', 'galaxian',
            'commodore', 'motorola', 'nmk', 'ice', 'act', 'fujitsu', 'altos', 'pacman', 'bmc', 'xerox',
            'sequential', 'qume', 'excalibur', 'citoh', 'dooyong', 'thomson', 'agat', 'sealy', 'vectorgraphic',
            'microterm', 'be', 'olympiaint', 'appliedconcepts', 'yeno', 'eaca', 'poly88', 'gamepark', 'concept',
            'mits', 'omnibyte', 'arcadia', 'cybiko', 'svi', 'memotech', 'cantab', 'matic', 'northstar', 'koei'),
        'driver-7': (
            'konami', 'apple', 'ussr', 'bfm', 'shared', 'roland', 'dec', 'yamaha', 'tecmo', 'psion', 'universal',
            'korg', 'itech', 'sigma', 'igt', 'subsino', 'suna', 'chess', 'stern', 'kawai', 'adds',
            'regnecentralen', 'husky', 'facit', 'coleco', 'elektor', 'funtech', 'mera', 'ampro', 'wang',
            'wavemate', 'hds', 'gottlieb', 'mitsubishi', 'samsung', 'svision', 'openuni', 'dg', 'tab', 'samcoupe',
            'pacific', 'zpa', 'burroughs', 'makerbot', 'ultimachine', 'westinghouse'),
        'driver-8': (
            'pinball', 'dataeast', 'handheld', 'pc', 'casio', 'snk', 'williams', 'novag', 'intel', 'akai',
            'alpha', 'ensoniq', 'rm', 'omron', 'edevices', 'robotron', 'metro', 'sanyo', 'upl', 'mit', 'bandai',
            'tecfri', 'dyna', 'cirsa', 'fairlight', 'canon', 'falco', 'sord', 'moog', 'videogames', 'homelab',
            'promat', 'hominn', 'synertek', 'leapfrog', 'netronics', 'palm', 'nascom', 'booth', 'microkey',
            'plexus', 'sage', 'orla', 'brainchild', 'kurzweil', 'usp')}


def add_common_arguments(parser):
    parser.add_argument('solution_makefile', type=pathlib.Path)
    parser.add_argument('--source-root', type=pathlib.Path, default=pathlib.Path('.'))


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', required=True)

    validate_parser = subparsers.add_parser('validate')
    add_common_arguments(validate_parser)

    list_parser = subparsers.add_parser('list')
    add_common_arguments(list_parser)
    list_parser.add_argument(
            '--group', required=True,
            choices=('base', 'thirdparty', 'tools', 'all') + tuple(BUCKETS))

    archives_parser = subparsers.add_parser('verify-archives')
    add_common_arguments(archives_parser)
    archives_parser.add_argument('--group', required=True, choices=('all',) + tuple(BUCKETS))
    archives_parser.add_argument('--archive-dir', required=True, type=pathlib.Path)

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


def validate_buckets(projects, source_root):
    assignments = [project for bucket in BUCKETS.values() for project in bucket]
    duplicates = sorted(project for project, count in collections.Counter(assignments).items() if count != 1)
    if duplicates:
        raise ValueError('projects assigned to multiple buckets: %s' % ' '.join(duplicates))

    generated_drivers = {
            project for project in projects
            if (source_root / 'src' / 'mame' / project).is_dir()}
    assigned_drivers = set(assignments)
    missing = sorted(generated_drivers - assigned_drivers)
    extra = sorted(assigned_drivers - generated_drivers)
    if missing or extra:
        details = []
        if missing:
            details.append('unassigned generated drivers: %s' % ' '.join(missing))
        if extra:
            details.append('assigned projects absent from generated drivers: %s' % ' '.join(extra))
        raise ValueError('; '.join(details))
    if EXECUTABLE_PROJECT not in projects:
        raise ValueError('generated solution does not contain the %s executable project' % EXECUTABLE_PROJECT)

    generated_projects = set(projects)
    missing_thirdparty = sorted(set(THIRDPARTY_PROJECTS) - generated_projects)
    missing_tools = sorted(set(TOOL_PROJECTS) - generated_projects)
    if missing_thirdparty or missing_tools:
        details = []
        if missing_thirdparty:
            details.append('third-party projects absent from generated solution: %s' % ' '.join(missing_thirdparty))
        if missing_tools:
            details.append('tool projects absent from generated solution: %s' % ' '.join(missing_tools))
        raise ValueError('; '.join(details))
    return generated_drivers


def select_projects(group, projects, generated_drivers):
    if group == 'base':
        excluded = generated_drivers | set(THIRDPARTY_PROJECTS) | set(TOOL_PROJECTS) | {EXECUTABLE_PROJECT}
        return [
                project for project in projects
                if project not in excluded]
    elif group == 'thirdparty':
        return [project for project in projects if project in THIRDPARTY_PROJECTS]
    elif group == 'tools':
        return [project for project in projects if project in TOOL_PROJECTS]
    elif group == 'all':
        return [project for bucket in BUCKETS.values() for project in bucket]
    else:
        return list(BUCKETS[group])


def verify_archives(archive_dir, selected_projects):
    if not archive_dir.is_dir():
        raise ValueError('%s: driver archive directory does not exist' % archive_dir)

    missing = sorted(
            project for project in selected_projects
            if not (archive_dir / ('lib%s.a' % project)).is_file())
    all_drivers = {project for bucket in BUCKETS.values() for project in bucket}
    actual_drivers = {
            path.name[3:-2] for path in archive_dir.glob('lib*.a')
            if path.name[3:-2] in all_drivers}
    unexpected = sorted(actual_drivers - set(selected_projects))
    if missing or unexpected:
        details = []
        if missing:
            details.append('missing driver archives: %s' % ' '.join(missing))
        if unexpected:
            details.append('unexpected driver archives: %s' % ' '.join(unexpected))
        raise ValueError('; '.join(details))


def main():
    options = parse_args()
    try:
        projects = read_solution_projects(options.solution_makefile)
        generated_drivers = validate_buckets(projects, options.source_root)
        if options.command == 'validate':
            sys.stdout.write('Validated %d driver projects in %d buckets.\n' % (
                    len(generated_drivers), len(BUCKETS)))
        else:
            selected_projects = select_projects(options.group, projects, generated_drivers)
            if options.command == 'list':
                sys.stdout.write('%s\n' % ' '.join(selected_projects))
            else:
                verify_archives(options.archive_dir, selected_projects)
                sys.stdout.write('Validated %d driver archives for %s.\n' % (
                        len(selected_projects), options.group))
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error validating CI driver buckets: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
