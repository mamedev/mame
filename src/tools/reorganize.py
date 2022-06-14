#!/usr/bin/python

import sys
import os

if len(sys.argv) != 2:
    print("Usage\n%s mame-cmake" % sys.argv[0])
    sys.exit(0)

# Build the lists of files

src = sys.argv[1] + '/src/mame/'

for root, subdirs, files in os.walk(src + 'drivers'):
    assert(len(subdirs) == 0)
    srcd = files

for root, subdirs, files in os.walk(src + 'audio'):
    assert(len(subdirs) == 0)
    srca = files

for root, subdirs, files in os.walk(src + 'video'):
    assert(len(subdirs) == 0)
    srcv = files

for root, subdirs, files in os.walk(src + 'machine'):
    assert(len(subdirs) == 0)
    srcm = files

for root, subdirs, files in os.walk(src + 'includes'):
    assert(len(subdirs) == 0)
    srci = files


# Compute the future name of the files to avoid collisions
present = {}
renaming = {}

def add(l, c):
    global present, renaming
    for f in l:
        if f in present:
            nfx = f.split('.')
            assert(len(nfx) == 2)
            nf = nfx[0] + '_' + c + '.' + nfx[1]
#            print("%s -> %s" % (f, nf))
            assert(nf not in present)
        else:
            nf = f
        present[nf] = True
        renaming[c + '/' + f] = nf
        
add(srci, "i")
add(srcd, "d")
add(srca, "a")
add(srcv, "v")
add(srcm, "m")


# Update the .lua files, lookup the per-file target names

kl = {
    'includes': 'i',
    'drivers': 'd',
    'audio': 'a',
    'video': 'v',
    'machine' : 'm'
}

keys = ['createMAMEProjects', 'createMESSProjects']
fpmap = {'v/konamiic.txt': 'konami' }

bad = False
for scr in ['arcade.lua', 'mess.lua', 'ci.lua', 'dummy.lua', 'nl.lua', 'tiny.lua', 'virtual.lua']:
    fname = sys.argv[1] + '/scripts/target/mame/' + scr
    lines = open(fname, 'r').readlines()
    cp = None
    f = open(fname, 'w')
    for l in lines:
        ls = l.rstrip('\r\n')
        for kk in keys:
            p = ls.find(kk)
            if p == 0 or p == 1:
                p = ls.find('"')+1
                p2 = ls.find('"', p)
                cp = ls[p:p2]
                if cp == 'mameshared':
                    cp = None
        p = ls.find('MAME_DIR .. "src/mame/')
        if p != -1:
            p2 = ls.find('"', p+22)
            fn = ls[p+22:p2]
            fx = fn.split('/')
            k = kl[fx[0]] + '/' + fx[1]
            if cp != None:
                if k in fpmap:
                    print("Duplicated: %s (%s and %s)" % (fn, cp, fpmap[k]))
                    bad = True
                fpmap[k] = cp
                ap = cp
            else:
                if k not in fpmap:
                    bad = True
#                    print("Missing in projects and seen in another .lua: %s" % fn)
                    ap = '?'
                else:
                    ap = fpmap[k]
            ls = ls[:p+22] + ap + '/' + renaming[k] + ls[p2:]
        print(ls, file=f)

for k in renaming.keys():
    if k not in fpmap:
        print("Missing in projects: %s" % k)
        bad = True

if bad:
    sys.exit(1)

# Create the new subdirectories
for d in set(fpmap.values()):
    os.mkdir(src + d)

# Move the files around, patching the include paths as needed

ikl = {}
for p, a in kl.items():
    ikl[a] = p

for bfs, bfd in renaming.items():
    bfsx = bfs.split('/')
    fs = src + ikl[bfsx[0]] + '/' + bfsx[1]
    fdm = fpmap[bfs]
    fd = src + fdm + '/' + bfd
    fdh = open(fd, 'w')
    for l in open(fs):
        p0 = 0
        while l[p0] == ' ' or l[p0] == '\t':
            p0 += 1
        if l[p0:p0+8] == '#include':
            itype = '"'
            si = l.find('"')
            if si != -1:
                si += 1
                ei = l.find('"', si)
                fi = l[si:ei]
                fix = fi.split('/')
                fik = None
                if len(fix) == 2 and fix[0] in kl:
                    fik = kl[fix[0]] + '/' + fix[1]
                elif len(fix) == 1:
                    fik = bfsx[0] + '/' + fi
                if fik != None and fik in fpmap:
                    fim = fpmap[fik]
                    if fim == fdm:
                        nfi = renaming[fik]
                    else:
                        nfi = fim + '/' + renaming[fik]
                    l = l[:si] + nfi + l[ei:]
        fdh.write(l)
    os.remove(fs)

for d in list(kl.keys()):
    os.rmdir(src + d)
