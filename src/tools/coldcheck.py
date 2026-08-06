#!/usr/bin/env python3

"""ATTR_COLD checker for MAME C++ headers.

Scans .h files for method declarations that should have ATTR_COLD but don't.
Can optionally fix them in-place.

Usage:
    src/tools/coldcheck.py [--fix] [path...]
    src/tools/coldcheck.py --fix src/mame/apple/
    src/tools/coldcheck.py src/devices/machine/6522via.h
    find src -name '*.h' | src/tools/coldcheck.py --fix -

If no paths given, scans src/emu/, src/devices/, src/mame/ by default.
Pass '-' to read file list from stdin.
"""

import argparse
import os
import re
import sys


NAMED_METHODS = (
    'device_start',
    'device_stop',
    'device_reset',
    'device_reset_after_children',
    'device_resolve_objects',
    'device_config_complete',
    'device_pre_save',
    'device_post_load',
    'device_clock_changed',
    'device_add_mconfig',
    'device_rom_region',
    'device_input_ports',
    'device_validity_check',
    'memory_space_config',
    'machine_start',
    'machine_reset',
    'video_start',
    'video_reset',
    'sound_start',
    'sound_reset',
    'driver_start',
    'interface_pre_start',
    'interface_post_start',
    'interface_pre_stop',
    'interface_post_stop',
    'interface_pre_save',
    'interface_post_load',
    'interface_pre_reset',
    'interface_post_reset',
    'interface_config_complete',
    'interface_validity_check',
    'call_unload',
    'get_default_card_software',
    'get_software_list_loader',
    'common_save_state',
    'palette_init',
)

PARAM_SIGNATURES = (
    'address_map &map',
    'address_map& map',
    'machine_config &config',
    'machine_config& config',
    'device_slot_interface &device',
    'device_slot_interface& device',
)

LOAD_METHODS = ('load', 'unload')


def _build_regex():
    named_alt = '|'.join(re.escape(n) for n in sorted(NAMED_METHODS, key=len, reverse=True))

    named_pattern = re.compile(
        r'^'
        r'(?P<indent>\s*)'
        r'(?P<prefix>(?:virtual\s+|static\s+|template\s*<[^>]*>\s+)*)'
        r'(?P<before_name>.*?)'
        r'\b(?P<name>' + named_alt + r')\b'
        r'(?P<params>\([^)]*\))'
        r'(?P<suffix>[^;]*?)'
        r';'
        r'(?P<comment>\s*(?://.*)?)$'
    )

    param_pattern = re.compile(
        r'^'
        r'(?P<indent>\s*)'
        r'(?P<prefix>(?:virtual\s+|static\s+|template\s*<[^>]*>\s+)*)'
        r'(?P<before_name>.*?)'
        r'(?P<name>\w+)'
        r'(?P<params>\([^)]*\))'
        r'(?P<suffix>[^;]*?)'
        r';'
        r'(?P<comment>\s*(?://.*)?)$'
    )

    return named_pattern, param_pattern


NAMED_RE, PARAM_RE = _build_regex()


def _has_attr_cold(line):
    return 'ATTR_COLD' in line


def _is_qualified_call(before_name):
    stripped = before_name.strip()
    return stripped.endswith('::')


def _insert_attr_cold(line, semicolon_pos):
    """Insert ATTR_COLD before ';'."""
    return line[:semicolon_pos] + ' ATTR_COLD;' + line[semicolon_pos + 1:]


def _compute_comment_state(lines):
    """Return list[bool] — True if line starts inside a /* */ comment."""
    result = [False] * len(lines)
    in_comment = False
    for i, line in enumerate(lines):
        result[i] = in_comment
        scan = line.split('//')[0]
        pos = 0
        while True:
            op = scan.find('/*', pos)
            cl = scan.find('*/', pos)
            if op < 0 and cl < 0:
                break
            if cl < 0 or (0 <= op < cl):
                in_comment = not in_comment
                pos = (op if op >= 0 else cl) + 2
            else:
                in_comment = False
                pos = cl + 2
    return result


def process_file(filepath, fix=False):
    """Process a single .h file.  Returns list of (line_no, reason, text)."""
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    in_comment = _compute_comment_state(lines)

    findings = []
    modified = False
    new_lines = list(lines)

    for i, line in enumerate(lines):
        stripped = line.strip()

        if not stripped or stripped.startswith('//') or stripped.startswith('#'):
            continue
        if '{' in stripped or '}' in stripped:
            continue
        if in_comment[i]:
            continue
        if _has_attr_cold(stripped):
            continue
        # Pure virtuals — ATTR_COLD is for concrete overrides, not abstract declarations
        if re.search(r'=\s*0\s*;', stripped):
            continue

        # --- Strategy 1: Named method match ---
        m = NAMED_RE.match(stripped)
        if m:
            name = m.group('name')
            before = m.group('before_name')
            suffix = m.group('suffix').strip()

            if _is_qualified_call(before):
                continue

            if name in LOAD_METHODS:
                if 'override' not in suffix and 'virtual' not in m.group('prefix'):
                    continue

            semicolon_pos = line.rfind(';')
            fixed = _insert_attr_cold(line, semicolon_pos)
            findings.append((i + 1, f'named method {name}', line.rstrip()))
            if fix:
                new_lines[i] = fixed
                modified = True
            continue

        # --- Strategy 2: Parameter signature match ---
        m = PARAM_RE.match(stripped)
        if m:
            params = m.group('params')
            for sig in PARAM_SIGNATURES:
                if sig in params:
                    before = m.group('before_name')

                    if _is_qualified_call(before):
                        continue

                    if (not before.strip()
                            and 'virtual' not in m.group('prefix')
                            and 'static' not in m.group('prefix')):
                        continue

                    semicolon_pos = line.rfind(';')
                    fixed = _insert_attr_cold(line, semicolon_pos)
                    findings.append((i + 1, f'param signature ({sig})', line.rstrip()))
                    if fix:
                        new_lines[i] = fixed
                        modified = True
                    break

    if modified:
        with open(filepath, 'w', encoding='utf-8', newline='') as f:
            f.writelines(new_lines)

    return findings


def collect_files(paths, from_stdin=False):
    if from_stdin:
        for line in paths:
            filepath = line.strip()
            if filepath.endswith('.h') and os.path.isfile(filepath):
                yield filepath
        return

    for p in paths:
        if os.path.isfile(p):
            if p.endswith('.h'):
                yield p
        elif os.path.isdir(p):
            for root, dirs, files in os.walk(p):
                for fn in sorted(files):
                    if fn.endswith('.h'):
                        yield os.path.join(root, fn)


def main():
    parser = argparse.ArgumentParser(
        description='Check (and optionally fix) missing ATTR_COLD in MAME headers.'
    )
    parser.add_argument(
        '--fix', action='store_true',
        help='Fix missing ATTR_COLD annotations in-place.'
    )
    parser.add_argument(
        'paths', nargs='*',
        help='Files or directories to scan. Use - to read file list from stdin. '
             'Default: src/emu/ src/devices/ src/mame/'
    )
    args = parser.parse_args()

    if not args.paths:
        search_paths = ['src/emu', 'src/devices', 'src/mame']
        from_stdin = False
    elif '-' in args.paths:
        from_stdin = True
        search_paths = args.paths
    else:
        search_paths = args.paths
        from_stdin = False

    total_findings = 0
    total_files = 0
    files_with_findings = 0

    for filepath in collect_files(search_paths, from_stdin=from_stdin):
        total_files += 1
        findings = process_file(filepath, fix=args.fix)
        if findings:
            files_with_findings += 1
            total_findings += len(findings)
            for line_no, reason, text in findings:
                tag = 'FIXED' if args.fix else 'MISSING'
                print(f'{filepath}:{line_no}: {tag} ({reason})')
                print(f'  {text}')

    action = 'fixed' if args.fix else 'found'
    print(f'\n{total_findings} missing ATTR_COLD {action} in {files_with_findings}/{total_files} files.')

    return 1 if total_findings > 0 and not args.fix else 0


if __name__ == '__main__':
    sys.exit(main())
