#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner

from __future__ import with_statement

import sys

drivlist = []
sourcelist = []
filter_addlist = []
filter_removelist = []

def parse_file(srcfile):
    try:
        fp = open(srcfile, 'rt')
    except IOError:
        sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
        return 1
    in_comment = 0
    linenum = 0
    curr_source = ''
    for line in fp.readlines():
        drivname = ''
        linenum+=1
        srcptr = 0
        while srcptr < len(line):
            c = line[srcptr]
            srcptr+=1
            if c==13 or c==10:
                if c==13 and line[srcptr]==10:
                    srcptr+=1
                continue
            if c==' ' or c==9:
                continue
            if in_comment==1 and c=='*' and line[srcptr]=='/' :
                srcptr+=1
                in_comment = 0
                continue
            if in_comment:
                continue
            if c=='/' and line[srcptr]=='*' :
                srcptr+=1
                in_comment = 1
                continue
            if c=='/' and line[srcptr]=='/' :
                break
            drivname += c
        drivname = drivname.strip()
        if len(drivname)>0:
            if drivname[0]=='#':
               sys.stderr.write("Importing drivers from '%s'\n" % drivname[1:])
               parse_file(drivname[1:])
               continue
            if drivname[0]=='@':
               curr_source= drivname[8:]
               continue
            if not all(((c >='a' and c<='z') or (c>='0' and c<='9') or c=='_') for c in drivname):
               sys.stderr.write("%s:%d - Invalid character in driver \"%s\"\n" % (srcfile,  linenum,  drivname))
               return 1
            else:
               if (curr_source == '') or (len(sourcelist)==0) or (curr_source in sourcelist):
                   drivlist.append(drivname)
    return 0

def parse_filter_file(srcfile):
    try:
        fp = open(srcfile, 'rt')
    except IOError:
        sys.stderr.write("Unable to open filter file '%s'\n" % srcfile)
        return 1
    in_comment = 0
    linenum = 0
    for line in fp.readlines():
        sourcename = ''
        linenum+=1
        srcptr = 0
        while srcptr < len(line):
            c = line[srcptr]
            srcptr+=1
            if c==13 or c==10:
                if c==13 and line[srcptr]==10:
                    srcptr+=1
                continue
            if c==' ' or c==9:
                continue
            if in_comment==1 and c=='*' and line[srcptr]=='/' :
                srcptr+=1
                in_comment = 0
                continue
            if in_comment:
                continue
            if c=='/' and line[srcptr]=='*' :
                srcptr+=1
                in_comment = 1
                continue
            if c=='/' and line[srcptr]=='/' :
                break
            sourcename += c
        sourcename = sourcename.strip()
        if len(sourcename)>0:
            if sourcename[0]=='#':
               sys.stderr.write("Importing drivers from '%s'\n" % sourcename[1:])
               parse_filter_file(sourcename[1:])
               continue
            if sourcename[0]=='+':
               filter_addlist.append(sourcename[1:])
               continue
            if sourcename[0]=='-':
               filter_removelist.append(sourcename[1:])
               continue
            if not all(((c >='a' and c<='z') or (c>='0' and c<='9') or c=='_' or c=='.' or c=='-') for c in sourcename):
               sys.stderr.write("%s:%d - Invalid character in driver \"%s\"\n" % (srcfile,  linenum,  sourcename))
               return 1
            else:               
               sourcelist.append(sourcename)
    return 0


if len(sys.argv) < 2 or len(sys.argv) > 3:
    print('Usage:')
    print('  makelist <source.lst> [<filter.flt>]')
    sys.exit(0)

if len(sys.argv) == 3:
    if parse_filter_file(sys.argv[2]) :
        sys.exit(1)
    sys.stderr.write("%d source file(s) found\n" % len(sourcelist))

if parse_file(sys.argv[1]) :
    sys.exit(1)

# output a count
if len(drivlist)==0 :
    sys.stderr.write("No drivers found\n")
    sys.exit(1)

for x in filter_addlist:
    drivlist.append(x)

drivlist = [x for x in drivlist if (x not in filter_removelist)]

sys.stderr.write("%d driver(s) found\n" % len(drivlist))

# add a reference to the ___empty driver
drivlist.append("___empty")

# start with a header
print('#include "emu.h"\n')
print('#include "drivenum.h"\n')

#output the list of externs first
for drv in sorted(drivlist):
    print("GAME_EXTERN(%s);" % drv)
print("")

# then output the array
print("const game_driver * const driver_list::s_drivers_sorted[%d] =" % len(drivlist))
print("{")
for drv in sorted(drivlist):
    print("\t&GAME_NAME(%s)," % drv)
print("};")
print("")

# also output a global count
print("int driver_list::s_driver_count = %d;\n" % len(drivlist))
