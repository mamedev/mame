#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Olivier Galibert

from __future__ import print_function

import os
import os.path
import re
import sys
import zlib
import vlayout

validators = {}
validators['.lay'] = vlayout.validate


if len(sys.argv) < 4:
    print('Usage:')
    print('  %s name dest.cpp source-dir[:subdir-name] [source-dir[:..]...]')
    sys.exit(0 if len(sys.argv) <= 1 else 1)

name = sys.argv[1]
outfn = sys.argv[2]

# First is list of directories, second is list of files
hierarchy = ([], [])
blobs = []
blobusize = []
bloboffset = []

for fileid in range(3, len(sys.argv)):
    source = sys.argv[fileid].split(':')
    base = hierarchy
    if len(source) == 2:
        steps = source[1].split('/')
        for s in steps:
            e = [d for d in base[0] if d[0] == s]
            if len(e) != 0:
                base = e[0][1]
            else:
                nb = ([], [])
                base[0].append((s, nb))
                base = nb
        
    lookup = {}
    lookup[source[0]] = base

    for root, subdirs, files in os.walk(source[0]):
        assert(root in lookup)
        base = lookup[root]
        curdirs = [d[0] for d in base[0]]
        newdirs = [d for d in subdirs if d not in curdirs]
        for d in newdirs:
            b = ([], [])
            base[0].append((d, b))
            lookup[os.path.join(root, d)] = b
        for f in files:
            data = None
            fpath = os.path.join(root, f)
            ext = os.path.splitext(f)[1]
            if ext in validators:
                data = validators[ext](fpath)
            else:
                data = open(fpath, 'rb').read()
            base[1].append((f, len(blobs)))
            blobs.append(zlib.compress(data, 9))
            blobusize.append(len(data))

directories = []
dcount = 0
fcount = 0

def hier_sort(e, p):
    global dcount
    global fcount
    global directories
    if p != -1:
        directories[p][2] = dcount
    dcount += len(e[0])
    fcount += len(e[1])
    e[0].sort(key = lambda x: x[0])
    e[1].sort(key = lambda x: x[0])
    dbase = len(directories)
    for ee in e[0]:
        directories.append([ee, p, -1, -1])
    for i in range(0, len(e[0])):
        hier_sort(e[0][i][1], dbase+i)

hier_sort(hierarchy, -1)

tsize = 0
for b in blobs:
    size = len(b)
    bloboffset.append(tsize)
    tsize += size

f = open(outfn, 'w')

print('// Generated file, editing is futile', file=f)
print('', file=f)
print('#include "intfs.h"', file=f)
print('', file=f)
print('extern const intfs::dir_entry root_%s;' % name, file=f)
print('', file=f)
print('static const unsigned char data_%s[%d] = {' % (name, tsize), file=f)

bpos = 0
if sys.version_info < (3, 0):
    for b in blobs:
        for c in b:
            f.write('%d,' % ord(c))
            bpos += 1
            if (bpos & 31) == 31:
                print('', file=f)
else:
    for b in blobs:
        for c in b:
            f.write('%d,' % c)
            bpos += 1
            if (bpos & 31) == 31:
                print('', file=f)

print('};', file=f)
print('', file=f)

print('static const intfs::file_entry file_%s[%d] = {' % (name, fcount), file=f)

def write_files(f, e):
    global name
    for ent in e:
        ee = ent[1]
        print('\t{ "%s", data_%s+%u, %u, %u },' % (ent[0], name, bloboffset[ee], len(blobs[ee]), blobusize[ee]), file=f)

write_files(f, hierarchy[1])
fpos = len(hierarchy[1])
for d in directories:
    d[3] = fpos
    fpos += len(d[0][1][1])
    write_files(f, d[0][1][1])

print('};', file=f)
print('', file=f)

def write_dir(f, d):
    global name
    parent = d[1]
    print('\t{ "%s", %s, %s, %s, %u, %u }%s' %
          (d[0][0],
           ("dir_%s+%d" % (name, parent) if parent >= 0 else "&root_%s" % name if parent == -1 else "nullptr"),
           ("dir_%s+%d" % (name, d[2]) if len(d[0][1][0]) > 0 else "nullptr"),
           ("file_%s+%d" % (name, d[3]) if len(d[0][1][1]) > 0 else "nullptr"),
           len(d[0][1][0]),
           len(d[0][1][1]),
           '' if parent == -2 else ','), file=f)


if dcount > 0:
   print('static const intfs::dir_entry dir_%s[%d] = {' % (name, dcount), file=f)
   for d in directories:
       write_dir(f, d)
   print('};', file=f)
   print('', file=f)

print('const intfs::dir_entry root_%s =' % name, file=f)
write_dir(f, [('.', hierarchy), -2, 0, 0])
print(';', file=f)
