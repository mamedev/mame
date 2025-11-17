#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import re
import subprocess
import sys


# identify .text section
sec_pattern = re.compile(' *(0|[1-9][0-9]*) +\\.text +[0-9a-f]+ +[0-9a-f]+ +[^ ].+')
sections = []
with subprocess.Popen(('objdump', '--section-headers', sys.argv[1]), stdout=subprocess.PIPE) as proc:
    for line in proc.stdout:
        text = line.decode('utf-8')
        match = sec_pattern.match(text)
        if match:
            sections.append(int(match.group(1)) + 1)
    proc.wait()
    if proc.returncode != 0:
        sys.exit(proc.returncode)
    sections = frozenset(sections)

# filter symbols from other sections
sym_pattern = re.compile('^\\[ *(0|[1-9][0-9]*)\\]\\(sec +(-?[1-9][0-9]*)\\).+')
aux_pattern = re.compile('^[A-Z]+ +.+')
with subprocess.Popen(('objdump', '--syms', '--demangle', sys.argv[1]), stdout=subprocess.PIPE) as proc:
    ignored_section = False
    for line in proc.stdout:
        text = line.decode('utf-8')
        match = sym_pattern.match(text)
        if match:
            if int(match.group(2)) in sections:
                ignored_section = False
                sys.stdout.write(text)
            else:
                ignored_section = True
        else:
            match = aux_pattern.match(text)
            if (not match) or (not ignored_section):
                sys.stdout.write(text)
    if proc.returncode != 0:
        sys.exit(proc.returncode)
