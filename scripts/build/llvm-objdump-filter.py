#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import re
import sys


sections = frozenset([int(x) + 1 for x in sys.argv[1:]])
sym_pattern = re.compile('^\\[ *(0|[1-9][0-9]*)\\]\\(sec +(-?[1-9][0-9]*)\\).+')
aux_pattern = re.compile('^[A-Z]+ +.+')

ignored_section = False
for line in sys.stdin:
    sym_match = sym_pattern.match(line)
    if sym_match:
        if int(sym_match.group(2)) in sections:
            ignored_section = False
            sys.stdout.write(line)
        else:
            ignored_section = True
    else:
        aux_match = aux_pattern.match(line)
        if (not aux_match) or (not ignored_section):
            sys.stdout.write(line)
