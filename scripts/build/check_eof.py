#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
"""Checks that all files in source code ends with a newline at end of file
"""

import os
import sys

if __name__ == '__main__':
	CHAR_DEPTH = 2 if sys.platform in ["win32", "cygwin"] else 1
	for r, _, f in os.walk("src"):
		FILEPATHS = [os.path.join(r, file) for file in f]
		for item in FILEPATHS:
			if not item.endswith((".cpp", ".h", ".mm", ".lua", ".py")):
				continue
			with open(item, mode="r", encoding="utf-8") as fh:
				try:
					fh.seek(0, os.SEEK_END)
					fh.seek(fh.tell() - CHAR_DEPTH, os.SEEK_SET)
					value = fh.read(CHAR_DEPTH)
					if value != os.linesep:
						raise ValueError(value, item)
				except UnicodeDecodeError:
					print(item, file=sys.stderr)
					raise
