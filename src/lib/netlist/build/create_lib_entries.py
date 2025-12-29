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
devices = []

def process_file(srcfile):
        src = open(srcfile,'r')
        lines = src.readlines()
        for line in lines:
            ls = re.sub("\s+","",line.strip())
            ls = re.sub("^\s*//.*","",ls)
            m = re.match(r"NETLIB_DEVICE_IMPL\((\w+),", ls)
            #print("Line{}: {}".format(count, line.strip()))
            if m != None:
                n = m.group(1)
                devices.append(n)
            else:
                m = re.match(r"NETLIB_DEVICE_IMPL_ALIAS\((\w+),", ls)
                if m != None:
                    n = m.group(1)
                    devices.append(n)
                else:
                    m = re.match(r"NETLIB_DEVICE_IMPL_NS\((\w+),(\w+),", ls)
                    if m != None:
                        n = m.group(2)
                        devices.append(n)
        src.close()

if __name__ == '__main__':
    if (len(sys.argv) == 0):
        print('Usage:')
        print('  create_lib_entries.py files...')
        sys.exit(0)

    for argno in range(1, len(sys.argv)):
        process_file(sys.argv[argno])

    devices.sort()
    for d in devices:
        print("LIB_ENTRY({})".format(d))
