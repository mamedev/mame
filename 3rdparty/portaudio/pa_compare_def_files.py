# PortAudio Repository .def file checker
#
# Run this script from the root of the repository using the command:
#   python pa_compare_def_files.py
#
# The PortAudio repository contains two (semi-redundant) .def
# files that specify exported symbols for Windows dynamic link libraries.
#
# This script checks that the two .def files export the same symbols
# using the same ordinals.
#
# The .def files are:
#
#   1. msvc/portaudio.def
#   2. cmake/portaudio.def.in
#
# The CMake .def.in file is an input that generates a .def file
# with host-api-specific symbols commented out when a particular host API
# is not built.

import sys

msvc_portaudio_def_path = "msvc/portaudio.def"
cmake_portaudio_def_in_path = "cmake/portaudio.def.in"

def parse_def_file(file):
    result = {}
    for line in file:
        line = line.strip()
        if line:
            if "EXPORTS" in line or line[0] == ";":
                continue
            columns = line.split()
            #print(columns)
            symbol, ordinal = columns
            #print(symbol, ordinal)
            if ordinal in result:
                print(f"error: ordinal {ordinal} referenced multiple times")
            result[ordinal] = symbol
    return result

with open(msvc_portaudio_def_path, mode="rt", encoding="utf-8") as msvc_portaudio_def:
    msvc_portaudio_def_syms = parse_def_file(msvc_portaudio_def)

with open(cmake_portaudio_def_in_path, mode="rt", encoding="utf-8") as cmake_portaudio_def_in:
    cmake_portaudio_def_in_syms = parse_def_file(cmake_portaudio_def_in)

def clear_cmake_exclude_condition_prefix(sym):
    if "@" in sym:
        return sym.split("@")[-1]
    else:
        return sym

# dictionary keys are the ordinals
ordinals = list(set(msvc_portaudio_def_syms.keys()).union(cmake_portaudio_def_in_syms.keys()))
ordinals.sort(key=lambda s: int(s.replace("@", "")))

msvcMissingCount = 0
cmakeMissingCount = 0
differenceCount = 0

print("ordinal, msvc, cmake, remark")
for ordinal in ordinals:
    msvc_sym = msvc_portaudio_def_syms[ordinal] if ordinal in msvc_portaudio_def_syms else ""
    cmake_sym = cmake_portaudio_def_in_syms[ordinal] if ordinal in cmake_portaudio_def_in_syms else ""

    cmake_sym_no_cond = clear_cmake_exclude_condition_prefix(cmake_sym)

    remark = ""
    if not msvc_sym:
        remark = "missing in msvc/portaudio.def"
        msvcMissingCount += msvcMissingCount
    elif not cmake_sym:
        remark = "missing in cmake/portaudio.def.in"
        cmakeMissingCount += cmakeMissingCount
    elif msvc_sym != cmake_sym_no_cond:
        remark = "differs"
        differenceCount += 1
    else:
        remark = "ok"

    print(f"{ordinal}, {msvc_sym}, {cmake_sym}, {remark}")

print("SUMMARY")
print("=======")
issuesFound = msvcMissingCount > 0 or cmakeMissingCount > 0 or differenceCount > 0
if msvcMissingCount > 0:
    print(f"error: {msvc_portaudio_def_path} ({msvcMissingCount} missing symbols)")
if cmakeMissingCount > 0:
    print(f"error: {cmake_portaudio_def_in_path} ({cmakeMissingCount} missing symbols)")
if differenceCount > 0:
    print(f"error: there are {differenceCount} ordinals with non-matching symbols")

if issuesFound:
    sys.exit(1)
else:
    print("No issues found. All good.")
    sys.exit(0)
