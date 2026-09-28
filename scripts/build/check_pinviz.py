#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Ben Bruscella

## Checks the pinviz plugin's tables against the layouts they take their feature
## positions from. A table names a switch, the layout marks that switch with an
## id of the form "pinviz:<port>:<mask>", and the plugin reads the position out of
## the layout. Nothing at build or run time ties the two together, so an id that
## is renamed or dropped only shows up as a feature quietly missing from the
## table, which is easy to miss and hard to spot later.
##
## No machine is created and no ROM is needed, so this runs anywhere.

import glob
import io
import os
import os.path
import re
import sys


## the attribute order and the spacing between the tag and its bounds both vary
## between layouts, so be generous about what sits in between
ID_IN_LAYOUT = re.compile(r'id="(pinviz:[^"]+)"[^>]*>\s*<bounds\s+x="(-?[\d.]+)"\s+y="(-?[\d.]+)"\s+width="([\d.]+)"\s+height="([\d.]+)"')
SWITCH_IN_TABLE = re.compile(r"switch\s*=\s*\{\s*':(\w+)'\s*,\s*(0x[0-9a-fA-F]+)\s*\}(?:\s*,\s*lay\s*=\s*':(\w+)')?")
GAME_WITH_LAYOUT = re.compile(r'GAMEL?\(\s*\d+\s*,\s*(\w+)\s*,.*\blayout_(\w+)\s*\)')

## a table line may say that a feature deliberately carries its own coordinates
OPT_OUT = 'no layout id'


def read(path):
    with io.open(path, 'r', encoding='utf-8') as fd:
        return fd.read()


def playfield_of(layout):
    ## only the playfield panel places features; the backbox and the test panel
    ## carry ids of their own in some layouts and must not be matched here
    parts = layout.split('<collection name="Playfield">')
    if len(parts) < 2:
        return None
    return parts[1].split('</collection>')[0]


def layouts_for_machines(root):
    result = { }
    for src in glob.glob(os.path.join(root, 'src', 'mame', 'pinball', '*.cpp')):
        for line in read(src).splitlines():
            m = GAME_WITH_LAYOUT.search(line)
            if m:
                result[m.group(1)] = m.group(2)
    return result


def check_table(root, table, layout_name, reports):
    bad = False
    path = os.path.join(root, 'src', 'mame', 'layout', layout_name + '.lay')
    name = os.path.basename(table)
    if not os.path.exists(path):
        sys.stderr.write('%s: no layout %s\n' % (name, path))
        return True

    playfield = playfield_of(read(path))
    if playfield is None:
        sys.stderr.write('%s: %s.lay has no Playfield collection\n' % (name, layout_name))
        return True

    ids = { }
    duplicates = set()
    for m in ID_IN_LAYOUT.finditer(playfield):
        if m.group(1) in ids:
            duplicates.add(m.group(1))
        ids[m.group(1)] = tuple(float(v) for v in m.groups()[1:])

    for dup in sorted(duplicates):
        sys.stderr.write('%s.lay: %s appears more than once, so its position is ambiguous\n' % (layout_name, dup))
        bad = True

    for marker in ('pinviz:pfx', 'pinviz:pfy'):
        if marker not in ids:
            sys.stderr.write('%s.lay: no %s, so the plugin cannot find the playfield extent\n' % (layout_name, marker))
            bad = True

    used = set()
    for line in read(table).splitlines():
        ## a line can carry several features, as the shorter sensor and target lists do
        for m in SWITCH_IN_TABLE.finditer(line):
            key = 'pinviz:%s:%s' % (m.group(1), m.group(2))
            if m.group(3):
                key += ':' + m.group(3)
            if key in ids:
                used.add(key)
            elif OPT_OUT not in line:
                sys.stderr.write('%s: %s is not in %s.lay, so this feature has no position\n' % (name, key, layout_name))
                bad = True

    spare = set(k for k in ids if not k.startswith('pinviz:pf')) - used
    if spare:
        reports.append('%s.lay: %d id(s) no table uses: %s' % (layout_name, len(spare), ', '.join(sorted(spare))))

    return bad


def main():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    tables = sorted(glob.glob(os.path.join(root, 'plugins', 'pinviz', 'tables', '*.lua')))
    machines = layouts_for_machines(root)

    bad = False
    reports = [ ]
    checked = 0
    for table in tables:
        machine = os.path.splitext(os.path.basename(table))[0]
        if machine not in machines:
            ## shared code rather than a machine's table, such as the common body
            continue
        checked += 1
        if check_table(root, table, machines[machine], reports):
            bad = True

    for line in reports:
        sys.stdout.write('%s\n' % line)
    sys.stdout.write('pinviz: checked %d table(s) against their layouts\n' % checked)

    if bad:
        sys.exit(1)


if __name__ == '__main__':
    main()
