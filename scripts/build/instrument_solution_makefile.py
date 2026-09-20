#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import argparse
import io
import pathlib
import sys


PROJECTS_PREFIX = 'PROJECTS := '
BUILD_RECIPE_PREFIX = '\t@${MAKE} --no-print-directory -C '
TIMED_RECIPE_PREFIX = '\t@$(PROJECT_TIMER) ${MAKE} --no-print-directory -C '
TIMER_VARIABLE = "  PROJECT_TIMER = /usr/bin/time -a -f '%e %U %S %M %x' -o \"$(MAME_PROJECT_TIMING_DIR)/$@.timing\""


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('makefile', type=pathlib.Path, help='generated solution Makefile to instrument')
    return parser.parse_args()


def instrument_makefile(path):
    data = path.read_bytes()
    newline = '\r\n' if b'\r\n' in data else '\n'
    text = data.decode('utf-8')
    lines = text.splitlines()

    project_lines = [line for line in lines if line.startswith(PROJECTS_PREFIX)]
    if len(project_lines) != 1:
        raise ValueError('%s: expected one PROJECTS definition' % path)
    projects = project_lines[0][len(PROJECTS_PREFIX):].split()
    if not projects:
        raise ValueError('%s: generated solution contains no projects' % path)

    try:
        clean_index = lines.index('clean:')
    except ValueError:
        raise ValueError('%s: clean target not found' % path)

    timed_count = sum(line.startswith(TIMED_RECIPE_PREFIX) for line in lines[:clean_index])
    if TIMER_VARIABLE in lines:
        if timed_count != len(projects):
            raise ValueError('%s: project timing instrumentation is incomplete' % path)
        return False
    elif timed_count:
        raise ValueError('%s: timed recipes found without timer definition' % path)

    recipe_count = 0
    for index in range(clean_index):
        if lines[index].startswith(BUILD_RECIPE_PREFIX):
            lines[index] = TIMED_RECIPE_PREFIX + lines[index][len(BUILD_RECIPE_PREFIX):]
            recipe_count += 1
    if recipe_count != len(projects):
        raise ValueError('%s: found %d project recipes for %d projects' % (path, recipe_count, len(projects)))

    try:
        export_index = lines.index('export config')
    except ValueError:
        raise ValueError('%s: exported configuration not found' % path)
    insert_index = export_index + 1
    if (insert_index < len(lines)) and not lines[insert_index]:
        insert_index += 1
    lines[insert_index:insert_index] = [
            'ifneq (,$(MAME_PROJECT_TIMING_DIR))',
            TIMER_VARIABLE,
            'endif',
            '']

    output = newline.join(lines)
    if text.endswith(('\r', '\n')):
        output += newline
    with io.open(path, 'w', encoding='utf-8', newline='') as makefile:
        makefile.write(output)
    return True


def main():
    options = parse_args()
    try:
        instrument_makefile(options.makefile)
    except (IOError, OSError, UnicodeError, ValueError) as error:
        sys.stderr.write('Error instrumenting solution Makefile: %s\n' % error)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
