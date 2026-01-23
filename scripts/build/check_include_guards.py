#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import io
import os
import os.path
import re
import sys


def pathsplit(p):
    result = [ ]
    while p:
        d, n = os.path.split(p)
        if not n:
            result.insert(0, d)
            break
        else:
            result.insert(0, n)
            p = d
    return result


if __name__ == '__main__':
    extpat = re.compile('.+\\.(h|hpp)$')
    substpat = re.compile('[-.]')
    guardpat = re.compile(r'^ *# *ifndef +([^\s]+)(\s+.*)?')
    bad = False
    if len(sys.argv) < 2:
        sys.stderr.write("Error: requires at least one path defined\n")
        sys.exit(2)

    for root in sys.argv[1:]:
        for path, subdirs, files in os.walk(root):
            prefix = 'MAME_' + '_'.join([n.upper() for n in pathsplit(os.path.relpath(path, root))]) + '_'
            for f in files:
                if extpat.match(f):
                    expected = prefix + substpat.sub('_', f.upper())
                    fp = os.path.join(path, f)
                    with io.open(fp, 'r', encoding='utf-8') as fd:
                        for l in fd:
                            m = guardpat.match(l)
                            if m:
                                if m.group(1) != expected:
                                    sys.stderr.write('%s: #include guard does not appear to match expected %s\n' % (fp, expected))
                                    bad = True
                                break
    if bad:
        sys.exit(1)
