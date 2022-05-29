#!/usr/bin/env python3
# disclaimer: this file is mostly copied from Catch2

import os
import re
import datetime
import sys

root_path = os.path.dirname(os.path.realpath( os.path.dirname(sys.argv[0])))
version_string = "0.0.1"

starting_header = os.path.join(root_path, 'rapidfuzz', 'rapidfuzz_all.hpp')
output_header = os.path.join(root_path, 'extras', 'rapidfuzz_amalgamated.hpp')
output_cpp = os.path.join(root_path, 'extras', 'rapidfuzz_amalgamated.cpp')

# These are the copyright comments in each file, we want to ignore them
def is_copyright_line(line):
    copyright_lines = [
        '/* SPDX-License-Identifier: MIT',
        '/* Copyright '
    ]

    for copyright_line in copyright_lines:
        if line.startswith(copyright_line):
            return True
    return False


# The header of the amalgamated file: copyright information + explanation
# what this file is.
file_header = '''\
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  RapidFuzz v{version_string}
//  Generated: {generation_time}
//  ----------------------------------------------------------
//  This file is an amalgamation of multiple different files.
//  You probably shouldn't edit it directly.
//  ----------------------------------------------------------
'''

# Returns file header with proper version string and generation time
def formatted_file_header():
    return file_header.format(version_string=version_string,
                              generation_time=datetime.datetime.now())

# Which headers were already concatenated (and thus should not be
# processed again)
concatenated_headers = set()

internal_include_parser = re.compile(r'\s*#include <(rapidfuzz/.*)>.*')

def concatenate_file(out, filename: str, expand_headers: bool) -> int:
    # Gathers statistics on how many headers were expanded
    concatenated = 1
    with open(filename, mode='r', encoding='utf-8') as input:
        for line in input:
            if is_copyright_line(line):
                continue

            if line.startswith('#pragma once'):
                continue

            m = internal_include_parser.match(line)
            # anything that isn't a Catch2 header can just be copied to
            # the resulting file
            if not m:
                out.write(line)
                continue

            # We do not want to expand headers for the cpp file
            # amalgamation but neither do we want to copy them to output
            if not expand_headers:
                continue

            next_header = m.group(1)
            # We have to avoid re-expanding the same header over and
            # over again
            if next_header in concatenated_headers:
                continue
            concatenated_headers.add(next_header)
            concatenated += concatenate_file(out, os.path.join(root_path, next_header), expand_headers)

    return concatenated


def generate_header():
    with open(output_header, mode='w', encoding='utf-8') as header:
        header.write(formatted_file_header())
        header.write('#ifndef RAPIDFUZZ_AMALGAMATED_HPP_INCLUDED\n')
        header.write('#define RAPIDFUZZ_AMALGAMATED_HPP_INCLUDED\n')
        print('Concatenated {} headers'.format(concatenate_file(header, starting_header, True)))
        header.write('#endif // RAPIDFUZZ_AMALGAMATED_HPP_INCLUDED\n')

def generate_cpp():
    from glob import glob
    cpp_files = sorted(glob(os.path.join(root_path, 'rapidfuzz', '**/*.cpp'), recursive=True))
    with open(output_cpp, mode='w', encoding='utf-8') as cpp:
        cpp.write(formatted_file_header())
        cpp.write('\n#include "rapidfuzz_amalgamated.hpp"\n')
        for file in cpp_files:
            concatenate_file(cpp, file, False)
    print('Concatenated {} cpp files'.format(len(cpp_files)))


generate_header()
#generate_cpp()


# Notes:
# * For .cpp files, internal includes have to be stripped and rewritten
# * for .hpp files, internal includes have to be resolved and included
# * The .cpp file needs to start with `#include "catch_amalgamated.hpp"
# * include guards can be left/stripped, doesn't matter
# * *.cpp files should be included sorted, to minimize diffs between versions
# * *.hpp files should also be somehow sorted -> use catch_all.hpp as the
# *       entrypoint
# * allow disabling main in the .cpp amalgamation