#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import io
import os
import os.path
import re
import sys


def write_output(text):
    try:
        dst.write(text)
    except IOError:
        if dstfile is not None:
            sys.stderr.write('Error writing to output file \'%s\'\n' % dstfile)
            dst.close()
            os.remove(dstfile)
        else:
            sys.stderr.write('Error writing to output\n')
        sys.exit(3)


if __name__ == '__main__':
    if (len(sys.argv) > 4) or (len(sys.argv) < 2):
        print('Usage:')
        print('  file2lines <source.txt> [<output.h> [<varname>]]')
        sys.exit(0 if len(sys.argv) <= 1 else 1)

    srcfile = sys.argv[1]
    dstfile = sys.argv[2] if len(sys.argv) >= 3 else None
    if len(sys.argv) >= 4:
        varname = sys.argv[3]
    else:
        varname = os.path.basename(srcfile)
        base, ext = os.path.splitext(varname)
        if ext.lower() == '.txt':
            varname = base
        varname = 'lines_' + re.sub('[^0-9A-Za-z_]', '_', varname)

    dst = None
    try:
        with io.open(srcfile, 'r', encoding='utf-8') as src:
            if dstfile is not None:
                try:
                    dst = io.open(dstfile, 'w', encoding='utf-8')
                except IOError:
                    sys.stderr.write('Unable to open output file \'%s\'\n' % dstfile)
                    sys.exit(3)
            else:
                dst = sys.stdout
            write_output(u'char const *const %s[] = {\n' % varname)
            for line in src:
                if line[-1] == u'\n':
                    line = line[:-1]
                write_output(u'\t\t"')
                i = 0
                while i < len(line):
                    for j in range(i, len(line) + 1):
                        if j < len(line):
                            ch = line[j]
                            if (ch < u' ') or (ch > u'~') or (ch in u'\"\\'):
                                break
                    if j > i:
                        write_output(line[i:j])
                    if j < len(line):
                        ch = line[j]
                        if ch == u'\a':
                            write_output(u'\\a')
                        elif ch == u'\f':
                            write_output(u'\\f')
                        elif ch == u'\t':
                            write_output(u'\\t')
                        elif ch == u'\v':
                            write_output(u'\\v')
                        elif ch in u'\"\\':
                            write_output(u'\\' + ch)
                        else:
                            ch = ord(ch)
                            if ch < 0x20:
                                write_output(u'\\{0:03o}'.format(ch))
                            elif ch < 0x10000:
                                write_output(u'\\u{0:04X}'.format(ch))
                            else:
                                write_output(u'\\U{0:08X}'.format(ch))
                    i = j + 1
                write_output(u'",\n')
            write_output(u'\t\tnullptr };\n')
    except IOError:
        sys.stderr.write('Error reading input file \'%s\'\n' % srcfile)
        if (dstfile is not None) and (dst is not None):
            dst.close()
            os.remove(dstfile)
        sys.exit(2)
