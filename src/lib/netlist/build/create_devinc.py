#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Couriersud

import os
import os.path
import re
import sys
import xml.sax
import xml.sax.saxutils
import zlib


# workaround for version incompatibility
if sys.version_info > (3, ):
    long = int

# globals

last_src = ""

def process_srcfile(srcfile):
    global last_src
    if (last_src != srcfile):
        last_src = srcfile
        print("// ---------------------------------------------------------------------")
        print("// Source: {}".format(srcfile))
        print("// ---------------------------------------------------------------------")
        print("")

def process_entry_external(srcfile, name):
    process_srcfile(srcfile)
    print("NETLIST_EXTERNAL({})".format(name))

def process_entry(srcfile, name, params):
    process_srcfile(srcfile)
    p = re.sub("\+","",params)
    ps = p.split(",")
    pusage = ""
    pauto = ""
    for x in ps:
        if x[0:1] == "@":
            pauto = pauto + ", " + x[1:]
        else:
            pusage = pusage + ", " + x
    print("// usage       : {}(name{})".format(name, pusage))
    if len(pauto) > 0:
        print("// auto connect: {}".format(pauto[2:]))
    print("#define {}(...)                                                   \\".format(name))
    print("\tNET_REGISTER_DEVEXT({}, __VA_ARGS__)".format(name))
    print("")


def process_file(srcfile):
    src = open(srcfile,'r')
    lines = src.readlines()
    for line in lines:
        ls = re.sub("\s+","",line.strip())
        ls = re.sub("^\s*//.*","",ls)
        ls = re.sub("\"","",ls)
        m = re.match(r"NETLIB_DEVICE_IMPL\((\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
        #print("Line{}: {}".format(count, line.strip()))
        if m is not None:
            process_entry(srcfile, m.group(2), m.group(3))
        else:
            m = re.match(r"NETLIB_DEVICE_IMPL_ALIAS\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
            if m is not None:
                process_entry(srcfile, m.group(3), m.group(4))
            else:
                m = re.match(r"NETLIB_DEVICE_IMPL_NS\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
                if m is not None:
                    process_entry(srcfile, m.group(3), m.group(4))
                else:
                    m = re.match(r"LOCAL_LIB_ENTRY\((\w+)\)", ls)
                    if m is not None:
                        process_entry(srcfile, m.group(1), "")
                    else:
                        m = re.match(r"(static)*TRUTHTABLE_START\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
                        if m is not None:
                            process_entry(srcfile, m.group(2), m.group(5))
                        else:
                            #m = re.match(r"EXTERNAL_SOURCE\((\w+)\)", ls)
                            m = re.match(r"NETLIST_START\((\w+)\)", ls)
                            if m is not None:
                                process_entry_external(srcfile, m.group(1))

    src.close()

def file_header():
    print("// license:BSD-3-Clause")
    print("// copyright-holders:Couriersud")
    print("")
    print("#ifndef NLD_DEVINC_H")
    print("#define NLD_DEVINC_H")
    print("")
    print("#ifndef __PLIB_PREPROCESSOR__\n")
    print("")
    print("#include \"../nl_setup.h\"")
    print("")

def file_footer():
    print("#endif // __PLIB_PREPROCESSOR__\n")
    print("#endif // NLD_DEVINC_H\n")

if __name__ == '__main__':
    if (len(sys.argv) == 0):
        print('Usage:')
        print('  create_devinc files ...')
        sys.exit(0)
    files_sorted = []
    for argno in range(1, len(sys.argv)):
        files_sorted.append(sys.argv[argno])
    files_sorted.sort()
    file_header()
    for entry in files_sorted:
        process_file(entry)
    file_footer()

