#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner

from __future__ import with_statement

import sys
import os
import zlib

if len(sys.argv) < 4:
    print('Usage:')
    print('  complay <source.lay> <output.h> <varname>')
    print('')
    sys.exit(0)

srcfile = sys.argv[1]
dstfile = sys.argv[2]
varname = sys.argv[3]
type = 'UINT8'

try:
    myfile = open(srcfile, 'rb')
except IOError:
    sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
    sys.exit(-1)

byteCount = os.path.getsize(srcfile)
compsize = 0
compressiontype = 1

try:
    dst = open(dstfile,'w')
    dst.write('const %s %s_data[] =\n{\n\t' % ( type, varname))
    offs = 0
    with open(srcfile, "rb") as src:
        while True:
            chunk = src.read(byteCount)	
            if chunk:
                compchunk = bytearray(zlib.compress(chunk, 9))
                compsize = len(compchunk)
                for b in compchunk:
                    dst.write('%d' % b)
                    offs += 1
                    if offs != compsize:
                        dst.write(',')
            else:
                break
            dst.write('\n\t')

    dst.write('\n};\n')

except IOError:
    sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
    sys.exit(-1)

try:
    dst.write('extern const internal_layout %s;\n' % ( varname ))
    dst.write('const internal_layout %s = { \n\t' % ( varname ))
    dst.write('%d,%d,%d,%s_data\n' % ( byteCount, compsize, compressiontype, varname ))
    dst.write('\n};\n')


    dst.close()
except IOError:
    sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
    sys.exit(-1)		

