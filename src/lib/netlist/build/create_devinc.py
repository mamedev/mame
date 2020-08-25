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

def process_entry(srcfile, name, params):
    global last_src
    if (last_src != srcfile):
        last_src = srcfile
        print("// ---------------------------------------------------------------------")
        print("// Source: {}".format(srcfile))
        print("// ---------------------------------------------------------------------")

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
    print("    NET_REGISTER_DEVEXT({}, __VA_ARGS__)".format(name))


def process_file(srcfile):
    src = open(srcfile,'r')
    lines = src.readlines()
    for line in lines:
        ls = re.sub("\s+","",line.strip())
        ls = re.sub("^\s*//.*","",ls)
        ls = re.sub("\"","",ls)
        m = re.match(r"NETLIB_DEVICE_IMPL\((\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
        #print("Line{}: {}".format(count, line.strip()))
        if m != None:
            process_entry(srcfile, m.group(2), m.group(3))
        else:
            m = re.match(r"NETLIB_DEVICE_IMPL_ALIAS\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
            if m != None:
                process_entry(srcfile, m.group(3), m.group(4))
            else:
                m = re.match(r"NETLIB_DEVICE_IMPL_NS\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
                if m != None:
                    process_entry(srcfile, m.group(3), m.group(4))
                else:
                    m = re.match(r"LOCAL_LIB_ENTRY\((\w+)\)", ls)
                    if m != None:
                        process_entry(srcfile, m.group(1), "")
                    else:
                        m = re.match(r"TRUTHTABLE_START\((\w+),(\w+),(\w+),([a-zA-Z0-9_+@,]*)", ls)
                        if m != None:
                            process_entry(srcfile, m.group(1), m.group(4))

    src.close()

if __name__ == '__main__':
    if (len(sys.argv) == 0):
        print('Usage:')
        print('  create_devinc files ...')
        sys.exit(0)

    for argno in range(1, len(sys.argv)):
        process_file(sys.argv[argno])

