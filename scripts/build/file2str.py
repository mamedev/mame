#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner

from __future__ import with_statement

import sys
import os

if len(sys.argv) < 4:
    print('Usage:')
    print('  file2str <source.lay> <output.h> <varname> [<type>]')
    print('')
    print('The default <type> is char, with an assumed NULL terminator')
    sys.exit(0)

terminate = 1
srcfile = sys.argv[1]
dstfile = sys.argv[2]
varname = sys.argv[3]

if len(sys.argv) >= 5:
    type = sys.argv[4]
    terminate = 0
else:
    type = 'char'

try:
    myfile = open(srcfile, 'rb')
except IOError:
    sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
    sys.exit(-1)

byteCount = os.path.getsize(srcfile)
try:
    dst = open(dstfile,'w')
    dst.write('extern const %s %s[];\n' % ( type, varname ))
    dst.write('const %s %s[] =\n{\n\t' % ( type, varname))
    offs = 0
    with open(srcfile, "rb") as src:
        while True:
            chunk = src.read(16)
            if chunk:
                for b in chunk:
                    # For Python 2.x compatibility.
                    if isinstance(b, str):
                        b = ord(b)
                    dst.write('0x%02x' % b)
                    offs += 1
                    if offs != byteCount:
                        dst.write(',')
            else:
                break
            if offs != byteCount:
                dst.write('\n\t')
    if terminate == 1:
        dst.write(',0x00')
    dst.write('\n};\n')
    dst.close()
except IOError:
    sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
    sys.exit(-1)
