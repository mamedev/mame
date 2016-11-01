#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner

from __future__ import with_statement

import sys
import os
import re

if len(sys.argv) < 4:
    print('Usage:')
    print('  txt2str <source.typ> <output.h> <varname>')
    sys.exit(0)

terminate = 1
srcfile = sys.argv[1]
dstfile = sys.argv[2]
varname = sys.argv[3]

try:
    myfile = open(srcfile, 'rb')
except IOError:
    sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
    sys.exit(-1)

try:
    dst = open(dstfile,'w')
    dst.write('const char* %s = "\\\n' % varname)
    with open(srcfile, "rb") as src:
        for line in src:
            line = re.sub(r'[\r\n]*$', '', line)
            line = re.sub(r'([\\\"])', r'\\\1', line)
            dst.write(line);
            dst.write("\\n\\\n");
    dst.write('";\n')
    dst.close()
except IOError:
    sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
    sys.exit(-1)
