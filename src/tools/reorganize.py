#!/usr/bin/python

import argparse
import glob
import os
import os.path
import re
import subprocess
import sys


def parse_command():
    parser = argparse.ArgumentParser()
    parser.add_argument('srcroot', metavar='<srcroot>', help='MAME source root directory')
    return parser.parse_args()


def resolve_conflicts(root, *parts):
    def walk(subdir):
        for path, subdirs, files in os.walk(os.path.join(root, subdir)):
            if len(subdirs):
                raise Exception('Found %d unwanted subdirectories in %s' % (len(subdirs), path))
            return files

    present = set()
    result = {}
    for part in parts:
        c = part[0]
        for file in walk(part):
            if file in present:
                base, ext = os.path.splitext(file)
                if not ext:
                    raise Exception('File %s has no extension' % (file, ))
                nf = base + '_' + c + ext
                #print('%s -> %s' % (os.path.join(part, file), nf))
                if nf in present:
                    raise Exception('File %s still conflicted after renaming from %s' % (nf, file))
            else:
                nf = file
            present.add(nf)
            result[part + '/' + file] = nf

    return result


def update_projects(root, renaming, *scripts):
    keys = ('createMAMEProjects', 'createMESSProjects')
    result = { 'video/konamiic.txt': 'konami' }

    bad = False

    for script in scripts:
        fname = os.path.join(root, 'scripts', 'target', 'mame', script)
        with open(fname, 'r', encoding='utf-8') as infile:
            lines = infile.readlines()
        cp = None
        with open(fname, 'w', encoding='utf-8') as f:
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
                    p2 = ls.find('"', p + 22)
                    fn = ls[p+22:p2]
                    if cp is not None:
                        if fn in result:
                            print("Duplicated: %s (%s and %s)" % (fn, cp, result[fn]))
                            bad = True
                        result[fn] = cp
                        ap = cp
                    else:
                        if fn not in result:
                            bad = True
                            #print("Missing in projects and seen in another .lua: %s" % fn)
                            ap = '?'
                        else:
                            ap = result[fn]
                    ls = ls[:p+22] + ap + '/' + renaming[fn] + ls[p2:]
                print(ls, file=f)

    for k in renaming:
        if k not in result:
            print('Missing in projects: %s' % (k, ))
            bad = True

    if bad:
        raise Exception('Error updating project scripts')

    return result


def update_driver_list(fname, renaming, projectmap):
    with open(fname, 'r', encoding='utf-8') as infile:
        lines = infile.readlines()
    with open(fname, 'w', encoding='utf-8') as outfile:
        for l in lines:
            ls = l.rstrip('\r\n')
            match = re.match('(.*@source:)([-_0-9a-z]+\\.cpp)\\b(\s*)(.*)', ls)
            if match:
                f = 'drivers/' + match.group(2)
                r = projectmap[f] + '/' + renaming.get(f, match.group(2))
                if match.group(3):
                    w = len(match.group(2)) + len(match.group(3))
                    if len(r) < w:
                        r = r + (' ' * (w - len(r)))
                    else:
                        r = r + ' '
                print(match.group(1) + r + match.group(4), file=outfile)
            else:
                print(ls, file=outfile)


def update_driver_filter(fname, renaming, projectmap):
    with open(fname, 'r', encoding='utf-8') as infile:
        lines = infile.readlines()
    with open(fname, 'w', encoding='utf-8') as outfile:
        for l in lines:
            ls = l.rstrip('\r\n')
            match = re.match('(.*)(?<![-_0-9a-z])([-_0-9a-z]+\\.cpp)\\b(.*)', ls)
            if match:
                f = 'drivers/' + match.group(2)
                b = renaming.get(f, match.group(2))
                print(match.group(1) + projectmap[f] + '/' + b + match.group(3), file=outfile)
            else:
                print(ls, file=outfile)


def relocate_source(root, filename, destbase, renaming, projectmap):
    fsx = filename.split('/')
    destproject = projectmap[filename]
    sourcepath = os.path.join(root, 'src', 'mame', fsx[0], fsx[1])
    destpath = os.path.join(root, 'src', 'mame', destproject, destbase)
    with open(sourcepath, 'r', encoding='utf-8') as infile:
        lines = infile.readlines()
    subprocess.run(['git', '-C', root, 'mv', os.path.join('src', 'mame', fsx[0], fsx[1]), os.path.join('src', 'mame', destproject, destbase)])

    # Patch the include paths as needed
    with open(destpath, 'w', encoding='utf-8') as fdh:
        for l in lines:
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
                    if len(fix) == 2:
                        fik = fi
                    elif len(fix) == 1:
                        fik = fsx[0] + '/' + fi
                    if (fik is not None) and (fik in projectmap):
                        fim = fpmap[fik]
                        if fim == destproject:
                            nfi = renaming[fik]
                        else:
                            nfi = fim + '/' + renaming[fik]
                        l = l[:si] + nfi + l[ei:]
            fdh.write(l)


if __name__ == '__main__':
    options = parse_command()
    src = os.path.join(options.srcroot, 'src', 'mame')
    areas = ('includes', 'drivers', 'audio', 'video', 'machine')

    # Compute the future name of the files to avoid collisions
    renaming = resolve_conflicts(src, *areas)

    # Update the .lua files, lookup the per-file target names
    fpmap = update_projects(options.srcroot, renaming, 'arcade.lua', 'mess.lua', 'ci.lua', 'dummy.lua', 'nl.lua', 'tiny.lua', 'virtual.lua')

    # update .lst list files
    for flt in glob.glob(os.path.join(src, '*.lst')):
        update_driver_list(flt, renaming, fpmap)

    # update .flt filter files
    for flt in glob.glob(os.path.join(src, '*.flt')):
        update_driver_filter(flt, renaming, fpmap)

    # Create the new subdirectories
    for d in fpmap.values():
        os.makedirs(os.path.join(src, d), exist_ok=True)

    # Move the files around
    for fs, bfd in renaming.items():
        relocate_source(options.srcroot, fs, bfd, renaming, fpmap)

    for d in areas:
        os.rmdir(os.path.join(src, d))
