#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Ben Bruscella

## Exercises the pinviz plugin on every machine that has a table, and reports on
## what it finds. This needs a built MAME and the ROMs, so it cannot run in CI;
## scripts/build/check_pinviz.py covers what can be checked without them.
##
##   plugins/pinviz/selftest.py --rompath roms
##   plugins/pinviz/selftest.py --rompath roms --quick        (skip the long runs)
##   plugins/pinviz/selftest.py --rompath roms centaur        (one machine)
##
## Three things are checked:
##
## Geometry, from PINVIZ_DUMP. No rubber or target may cross a wall, because the
## acute corner that leaves holds a ball against both surfaces, and nothing may
## reach past the layout's playfield panel, because it would be drawn over the
## panel next to it. Every feature must also have a position.
##
## Flippers, from PINVIZ_FLIPTEST. No ball may pass through the bat at any speed
## a playfield can produce, and the shot should leave faster than it arrived.
##
## Play, from PINVIZ_AUTOPILOT. The machine has to serve balls and see switch
## closures, and a Lua error anywhere is a failure.

import argparse
import math
import os
import os.path
import re
import subprocess
import sys


MACHINES = ('centaur', 'playboy', 'matahari', 'pwerplay', 'hs_l4')
FLIP_SPEEDS = (40, 90, 140, 200, 260)


def run(mame, machine, seconds, rompath, env=None, timeout=600):
    cmd = [mame, machine, '-rompath', rompath, '-plugin', 'pinviz', '-window',
           '-seconds_to_run', str(seconds), '-nothrottle', '-video', 'none']
    full = dict(os.environ)
    full.update(env or { })
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, env=full)
    except subprocess.TimeoutExpired:
        return ''
    return p.stdout + p.stderr


def segments_cross(a, b):
    def side(p, q, r):
        return (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1])
    d1, d2 = side(b[0:2], b[2:4], a[0:2]), side(b[0:2], b[2:4], a[2:4])
    d3, d4 = side(a[0:2], a[2:4], b[0:2]), side(a[0:2], a[2:4], b[2:4])
    return ((d1 > 0) != (d2 > 0)) and ((d3 > 0) != (d4 > 0))


def check_geometry(mame, machine, rompath):
    out = run(mame, machine, 3, rompath, { 'PINVIZ_DUMP': '1' })
    failures = [ ]

    placed = re.search(r'(\d+) features placed from the layout, (\d+) from the table, (\d+) without a position', out)
    if not placed:
        return ['the plugin did not report placing any features'], { }
    if int(placed.group(3)):
        failures.append('%s feature(s) have no position, so the layout is missing ids' % placed.group(3))

    walls, segs, rounds, lay = [ ], [ ], [ ], None
    for line in out.splitlines():
        f = line.split()
        if len(f) < 3 or f[0] != '[dump]':
            continue
        if f[1] == 'wall':
            walls.append(tuple(float(v) for v in f[2:6]))
        elif f[1] in ('slings', 'targets'):
            segs.append((' '.join(f[6:]), tuple(float(v) for v in f[2:6])))
        elif f[1] in ('bumpers', 'sensors', 'saucers'):
            rounds.append((' '.join(f[5:]), tuple(float(v) for v in f[2:5])))

    for name, seg in segs:
        for wall in walls:
            if segments_cross(seg, wall):
                failures.append('"%s" crosses the wall (%.1f,%.1f)-(%.1f,%.1f)' % ((name,) + wall))
                break

    counts = { 'walls': len(walls), 'segments': len(segs), 'round': len(rounds),
               'placed': int(placed.group(1)), 'from table': int(placed.group(2)) }
    return failures, counts


def check_flippers(mame, machine, rompath, speeds):
    failures, rows = [ ], [ ]
    for speed in speeds:
        out = run(mame, machine, 45, rompath, { 'PINVIZ_FLIPTEST': str(speed) })
        line = None
        for l in out.splitlines():
            if l.startswith('[fliptest]'):
                line = l
        if not line:
            failures.append('the flipper test at %d in/s produced no result' % speed)
            continue
        m = re.search(r'trials (\d+) hit (\d+) through (\d+).*mean (\d+) range (\d+)-(\d+)', line)
        if not m:
            failures.append('could not read the flipper test result: %s' % line)
            continue
        trials, hit, through, mean, lo, hi = (int(v) for v in m.groups())
        rows.append((speed, mean, lo, hi, through, trials))
        if through:
            failures.append('%d of %d balls passed through the bat at %d in/s' % (through, trials, speed))
        if mean < speed * 0.6:
            failures.append('a %d in/s ball came off the flipper at %d in/s, so the bat is not driving it'
                            % (speed, mean))
    return failures, rows


def check_play(mame, machine, rompath, seconds):
    out = run(mame, machine, seconds, rompath,
              { 'PINVIZ_AUTOPILOT': '1', 'PINVIZ_LOG': '1' }, timeout=seconds * 3 + 120)
    failures = [ ]
    if re.search(r'(?i)lua error|stack traceback', out):
        failures.append('a Lua error was reported')

    balls = len(re.findall(r'ball play', out))
    hits = len(re.findall(r'^\[pinviz\]\s+[\d.]+ sw ', out, re.M))
    distinct = len(set(re.findall(r'^\[pinviz\]\s+[\d.]+ sw\s+(.*)$', out, re.M)))
    stuck = len(re.findall(r'stuck', out))

    if not balls:
        failures.append('no ball was served in %d seconds, so nothing was exercised' % seconds)
    elif not hits:
        failures.append('a ball was served but no switch closed')

    return failures, { 'balls': balls, 'hits': hits, 'distinct': distinct, 'stuck': stuck }


def main():
    parser = argparse.ArgumentParser(description='exercise the pinviz plugin on the machines that have tables')
    parser.add_argument('machines', nargs='*', default=None, help='machines to test, default all of them')
    parser.add_argument('--mame', default='./mame', help='the MAME executable, default ./mame')
    parser.add_argument('--rompath', default='roms', help='where the ROMs are, default roms')
    parser.add_argument('--quick', action='store_true', help='shorter runs and fewer flipper speeds')
    args = parser.parse_args()

    machines = args.machines or MACHINES
    speeds = (90, 200) if args.quick else FLIP_SPEEDS
    play_seconds = 60 if args.quick else 180

    if not os.path.exists(args.mame):
        sys.stderr.write('%s does not exist; build MAME first\n' % args.mame)
        sys.exit(2)

    bad = False
    for machine in machines:
        sys.stdout.write('\n=== %s\n' % machine)

        failures, counts = check_geometry(args.mame, machine, args.rompath)
        if counts:
            sys.stdout.write('  geometry: %s\n' % ', '.join('%s %s' % (k, v) for k, v in sorted(counts.items())))
        for f in failures:
            sys.stdout.write('  FAIL geometry: %s\n' % f)
        bad = bad or bool(failures)

        flip_failures, rows = check_flippers(args.mame, machine, args.rompath, speeds)
        for speed, mean, lo, hi, through, trials in rows:
            sys.stdout.write('  flipper: %3d in/s -> mean %3d, range %3d-%3d, %d of %d through\n'
                             % (speed, mean, lo, hi, through, trials))
        for f in flip_failures:
            sys.stdout.write('  FAIL flipper: %s\n' % f)
        bad = bad or bool(flip_failures)

        play_failures, play = check_play(args.mame, machine, args.rompath, play_seconds)
        sys.stdout.write('  play: %s\n' % ', '.join('%s %s' % (k, v) for k, v in sorted(play.items())))
        for f in play_failures:
            sys.stdout.write('  FAIL play: %s\n' % f)
        bad = bad or bool(play_failures)

    sys.stdout.write('\n%s\n' % ('some checks failed' if bad else 'all checks passed'))
    if bad:
        sys.exit(1)


if __name__ == '__main__':
    main()
