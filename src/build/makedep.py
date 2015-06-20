#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Miodrag Milanovic

from __future__ import with_statement

import sys
## to ignore include of emu.h add it always to list
files_included = ['src/emu/emu.h']

include_dirs = ['src/emu/', 'src/mame/', 'src/mess/']

mappings = dict()

components = []

def file_exists(srcfile, folder):
    includes = [ folder ]
    includes.extend(include_dirs)
    for line in includes:
        try:
            fp = open(line + srcfile, 'rb')
            return line + srcfile
        except IOError:
            ignore=1
    return ''

def parse_file(srcfile, folder):
    try:
        fp = open(srcfile, 'rb')
    except IOError:
        sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
        return 1
    in_comment = 0
    linenum = 0
    for line in fp.readlines():
        content = ''
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
            content += c
        content = content.strip()
        if len(content)>0:
            if content.startswith('#include'):
               name = content[8:]
               name = name.replace('"','')
               fullname = file_exists(name, folder)
               if fullname in files_included:
                   continue
               if fullname!='':
                   if fullname in mappings.keys():
                        if not(mappings[fullname] in components):
                            components.append(mappings[fullname])
                            sys.stderr.write("%s\n" % mappings[fullname])
                   files_included.append(fullname)
                   newfolder = fullname.rsplit('/', 1)[0] + '/'
                   parse_file(fullname, newfolder)
               continue
    return 0

def parse_lua_file(srcfile):
    try:
        fp = open(srcfile, 'rb')
    except IOError:
        sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
        return 1
    in_comment = 0
    linenum = 0
    for line in fp.readlines():
        content = line.strip()
        if len(content)>0:
            if content.startswith('--@'):
               name = content[3:]
               mappings[name.rsplit(',', 1)[0]] = name.rsplit(',', 1)[1]
    return 0

if len(sys.argv) < 2:
    print('Usage:')
    print('  makedep <source.c>')
    sys.exit(0)

parse_lua_file('scripts/src/bus.lua')
parse_lua_file('scripts/src/cpu.lua')
parse_lua_file('scripts/src/machine.lua')
parse_lua_file('scripts/src/sound.lua')
parse_lua_file('scripts/src/video.lua')

parse_file(sys.argv[1],'')

