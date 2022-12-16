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
import datetime


# workaround for version incompatibility
if sys.version_info > (3, ):
    long = int

# globals

last_src = ""

def process_entry(srcfile, name, params):
    global last_src
    if (last_src != srcfile):
        last_src = srcfile
        print("// ---------------------------------------------------------------------")
        print("// Source: {}".format(srcfile))
        print("// ---------------------------------------------------------------------")
        print("")
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
        m = re.match(r"NETLIST_START\((\w+)\)", ls)
        if m != None:
            print("\tEXTERNAL_LIB_ENTRY("+ m.group(1) + ")")
    src.close()

if __name__ == '__main__':
    if (len(sys.argv) == 0):
        print('Usage:')
        print('  create_devinc files ...')
        sys.exit(0)
    files_sorted = [];
    for argno in range(1, len(sys.argv)):
        files_sorted.append(sys.argv[argno])
    files_sorted.sort();
    print("// license:CC0")
    print("// copyright-holders:Couriersud")
    print("")
    now = datetime.datetime.now()
    print("// File programmatically created " + now.strftime("%c"))
    print("")
    print("#include \"devices/net_lib.h\"")
    print("")
    print("NETLIST_START(modules_lib)")
    print("{")
    print("")
    for entry in files_sorted:
        process_file(entry)
    print("")
    print("}")
