#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Miodrag Milanovic

from __future__ import with_statement

import sys
## to ignore include of emu.h add it always to list

files_included = ['src/emu/emu.h']

include_dirs = ['src/emu/', 'src/devices/', 'src/mame/', 'src/mess/']

mappings = dict()

deps_files_included = [ ]

deps_include_dirs = ['src/mame/', 'src/mess/']

components = [ ]

drivers = [ ]

def file_exists(root, srcfile, folder, inc_dir):
    includes = [ folder ]
    includes.extend(inc_dir)
    for line in includes:
        try:
            fp = open(root + line + srcfile, 'rb')
            fp.close()
            return line + srcfile
        except IOError:
            pass
    return ''

def add_c_if_exists(root, fullname):
    try:
        fp = open(root + fullname, 'rb')
        fp.close()
        deps_files_included.append(fullname)
    except IOError:
        pass

def add_rest_if_exists(root, srcfile,folder):
    t = srcfile.rsplit('/', 2)
    if t[1]=='includes':
        t[2] = t[2].replace('.h','.c')
        t[1] = 'drivers'     
        add_c_if_exists(root,"/".join(t))
        parse_file_for_deps(root, "/".join(t), folder)
        t[1] = 'machine'     
        add_c_if_exists(root,"/".join(t))
        parse_file_for_deps(root, "/".join(t), folder)
        t[1] = 'video'     
        add_c_if_exists(root,"/".join(t))
        parse_file_for_deps(root, "/".join(t), folder)
        t[1] = 'audio'
        add_c_if_exists(root,"/".join(t))
        parse_file_for_deps(root, "/".join(t), folder)

def parse_file_for_deps(root, srcfile, folder):
    try:
        fp = open(root + srcfile, 'rb')
    except IOError:
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
               fullname = file_exists(root, name, folder,deps_include_dirs)
               if fullname in deps_files_included:
                   continue
               if fullname!='':
                   deps_files_included.append(fullname)
                   add_c_if_exists(root, fullname.replace('.h','.c'))
                   add_rest_if_exists(root, fullname,folder)
                   newfolder = fullname.rsplit('/', 1)[0] + '/'
                   parse_file_for_deps(root, fullname, newfolder)
               continue
    fp.close()
    return 0

def parse_file(root, srcfile, folder):
    try:
        fp = open(root + srcfile, 'rb')
    except IOError:
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
               fullname = file_exists(root, name, folder,include_dirs)
               if fullname in files_included:
                   continue
               if "src/lib/netlist/" in fullname:
                   continue
               if fullname!='':
                   if fullname in mappings.keys():
                        if not(mappings[fullname] in components):
                            components.append(mappings[fullname])
                   files_included.append(fullname)
                   newfolder = fullname.rsplit('/', 1)[0] + '/'
                   parse_file(root, fullname, newfolder)
                   if (fullname.endswith('.h')):
                       parse_file(root, fullname.replace('.h','.c'), newfolder)
               continue
    fp.close()
    return 0

def parse_file_for_drivers(root, srcfile):
    try:
        fp = open(root + srcfile, 'rb')
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
            if content.startswith('COMP') or content.startswith('CONS') or content.startswith('GAME') or content.startswith('SYST')  or content.startswith('GAMEL'):
               name = content[4:]
               drivers.append(name.rsplit(',', 14)[1])
    return 0

def parse_lua_file(srcfile):
    try:
        fp = open(srcfile, 'rb')
    except IOError:
        sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
        return 1
    for line in fp.readlines():
        content = line.strip()
        if len(content)>0:
            if content.startswith('--@'):
               name = content[3:]
               mappings[name.rsplit(',', 1)[0]] = name.rsplit(',', 1)[1]
    return 0

if len(sys.argv) < 5:
    print('Usage:')
    print('  makedep <root> <source.c> <type> <target>')
    sys.exit(0)

root = sys.argv[1] + '/'

parse_lua_file(root +'scripts/src/bus.lua')
parse_lua_file(root +'scripts/src/cpu.lua')
parse_lua_file(root +'scripts/src/machine.lua')
parse_lua_file(root +'scripts/src/sound.lua')
parse_lua_file(root +'scripts/src/video.lua')

for filename in sys.argv[2].rsplit(',') :
    deps_files_included.append(filename.replace('\\','/'))
    parse_file_for_deps(root,filename,'')

for filename in deps_files_included:
    parse_file(root,filename,'')

for filename in sys.argv[2].rsplit(',') :
    parse_file_for_drivers(root,filename)


# display output
if sys.argv[3]=='drivers':
    # add a reference to the ___empty driver
    drivers.append("___empty")

    # start with a header
    print('#include "emu.h"\n')
    print('#include "drivenum.h"\n')

    #output the list of externs first
    for drv in sorted(drivers):
        print("GAME_EXTERN(%s);" % drv)
    print("")

    # then output the array
    print("const game_driver * const driver_list::s_drivers_sorted[%d] =" % len(drivers))
    print("{")
    for drv in sorted(drivers):
        print("\t&GAME_NAME(%s)," % drv)
    print("};")
    print("")

    # also output a global count
    print("int driver_list::s_driver_count = %d;\n" % len(drivers))

if sys.argv[3]=='target':
    for line in components:
        sys.stdout.write("%s\n" % line)    
    sys.stdout.write('\n')
    sys.stdout.write('function createProjects_mame_%s(_target, _subtarget)\n' % sys.argv[4])
    sys.stdout.write('    project ("mame_%s")\n' % sys.argv[4])
    sys.stdout.write('    targetsubdir(_target .."_" .. _subtarget)\n')
    sys.stdout.write('    kind (LIBTYPE)\n')
    sys.stdout.write('    uuid (os.uuid("drv-mame-%s"))\n' % sys.argv[4])
    sys.stdout.write('    \n')
    sys.stdout.write('    options {\n')
    sys.stdout.write('        "ForceCPP",\n')
    sys.stdout.write('    }\n')
    sys.stdout.write('    \n')
    sys.stdout.write('    includedirs {\n')
    sys.stdout.write('        MAME_DIR .. "src/osd",\n')
    sys.stdout.write('        MAME_DIR .. "src/emu",\n')
    sys.stdout.write('        MAME_DIR .. "src/devices",\n')
    sys.stdout.write('        MAME_DIR .. "src/mame",\n')
    sys.stdout.write('        MAME_DIR .. "src/mess",\n')
    sys.stdout.write('        MAME_DIR .. "src/lib",\n')
    sys.stdout.write('        MAME_DIR .. "src/lib/util",\n')
    sys.stdout.write('        MAME_DIR .. "3rdparty",\n')
    sys.stdout.write('        GEN_DIR  .. "mame/layout",\n')
    sys.stdout.write('        GEN_DIR  .. "mess/layout",\n')
    sys.stdout.write('    }\n')
    sys.stdout.write('    if _OPTIONS["with-bundled-zlib"] then\n')
    sys.stdout.write('        includedirs {\n')
    sys.stdout.write('            MAME_DIR .. "3rdparty/zlib",\n')
    sys.stdout.write('        }\n')
    sys.stdout.write('    end\n')
    sys.stdout.write('\n')
    sys.stdout.write('    files{\n')
    for line in deps_files_included:
        sys.stdout.write('        MAME_DIR .. "%s",\n' % line)
    sys.stdout.write('    }\n')
    sys.stdout.write('end\n')
    sys.stdout.write('\n')
    sys.stdout.write('function linkProjects_mame_%s(_target, _subtarget)\n' % sys.argv[4])
    sys.stdout.write('    links {\n')
    sys.stdout.write('        "mame_%s",\n' % sys.argv[4])
    sys.stdout.write('    }\n')
    sys.stdout.write('end\n')
