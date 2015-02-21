#!/usr/bin/python

from __future__ import with_statement

import os
import re
import sys


def parse_args():
    def usage():
        sys.stderr.write('Usage: verinfo.py [-b mame|mess|ume] [-r|-p] <filename>\n')
        sys.exit(1)

    flags = True
    target = 'mame'
    input = None
    output = 'rc'
    i = 1
    while i < len(sys.argv):
        if flags and (sys.argv[i] == '-r'):
            output = 'rc'
        elif flags and (sys.argv[i] == '-p'):
            output = 'plist'
        elif flags and (sys.argv[i] == '-b'):
            i += 1;
            if (i >= len(sys.argv)) or (sys.argv[i] not in ('mame', 'mess', 'ume')):
                usage()
            else:
                target = sys.argv[i]
        elif flags and (sys.argv[i] == '--'):
            flags = False
        elif flags and sys.argv[i].startswith('-'):
            usage()
        elif input is not None:
            usage()
        else:
            input = sys.argv[i]
        i += 1
    if input is None:
        usage()
    return target, input, output


def extract_version(input):
    pattern = re.compile('\s+BARE_BUILD_VERSION\s+"(([^."]+)\.([^."]+))"')
    for line in input.readlines():
        match = pattern.search(line)
        if match:
            return match.group(1), match.group(2), match.group(3)
    return None, None, None


build, srcfile, outfmt = parse_args()

try:
    fp = open(srcfile, 'rb')
except IOError:
    sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
    sys.exit(1)

version_string, version_major, version_minor = extract_version(fp)
version_build = "0"
version_subbuild = "0"
if not version_string:
    sys.stderr.write("Unable to extract version from source file '%s'\n" % srcfile)
    sys.exit(1)

if build == "mess":
    # MESS
    author = "MESS Team";
    comments = "Multi Emulation Super System";
    company_name = "MESS Team";
    file_description = "Multi Emulation Super System";
    internal_name = "MESS";
    original_filename = "MESS";
    product_name = "MESS";
    bundle_identifier = "org.mamedev.mess"
elif build == "ume":
    # UME
    author = "MAME and MESS Team"
    comments = "Universal Machine Emulator"
    company_name = "MAME and MESS Team"
    file_description = "Universal Machine Emulator"
    internal_name = "UME"
    original_filename = "UME"
    product_name = "UME"
    bundle_identifier = "org.mamedev.ume"
else:
    # MAME
    author = "Nicola Salmoria and the MAME Team"
    comments = "Multiple Arcade Machine Emulator"
    company_name = "MAME Team"
    file_description = "Multiple Arcade Machine Emulator"
    internal_name = "MAME"
    original_filename = "MAME"
    product_name = "MAME"
    bundle_identifier = "org.mamedev.mame"

legal_copyright = "Copyright Nicola Salmoria and the MAME team"

if outfmt == 'rc':
    print("VS_VERSION_INFO VERSIONINFO")
    print("\tFILEVERSION %s,%s,%s,%s" % (version_major, version_minor, version_build, version_subbuild))
    print("\tPRODUCTVERSION %s,%s,%s,%s" % (version_major, version_minor, version_build, version_subbuild))
    print("\tFILEFLAGSMASK 0x3fL")
    if (version_build == 0) :
        print("\tFILEFLAGS 0x0L")
    else :
        print("\tFILEFLAGS VS_FF_PRERELEASE")
    print("\tFILEOS VOS_NT_WINDOWS32")
    print("\tFILETYPE VFT_APP")
    print("\tFILESUBTYPE VFT2_UNKNOWN")
    print("BEGIN")
    print("\tBLOCK \"StringFileInfo\"")
    print("\tBEGIN")
    print("#ifdef UNICODE")
    print("\t\tBLOCK \"040904b0\"")
    print("#else")
    print("\t\tBLOCK \"040904E4\"")
    print("#endif")
    print("\t\tBEGIN")
    print("\t\t\tVALUE \"Author\", \"%s\\0\"" % author)
    print("\t\t\tVALUE \"Comments\", \"%s\\0\"" % comments)
    print("\t\t\tVALUE \"CompanyName\", \"%s\\0\"" % company_name)
    print("\t\t\tVALUE \"FileDescription\", \"%s\\0\"" % file_description)
    print("\t\t\tVALUE \"FileVersion\", \"%s, %s, %s, %s\\0\"" % (version_major, version_minor, version_build, version_subbuild))
    print("\t\t\tVALUE \"InternalName\", \"%s\\0\"" % internal_name)
    print("\t\t\tVALUE \"LegalCopyright\", \"%s\\0\"" % legal_copyright)
    print("\t\t\tVALUE \"OriginalFilename\", \"%s\\0\"" % original_filename)
    print("\t\t\tVALUE \"ProductName\", \"%s\\0\"" % product_name)
    print("\t\t\tVALUE \"ProductVersion\", \"%s\\0\"" % version_string)
    print("\t\tEND")
    print("\tEND")
    print("\tBLOCK \"VarFileInfo\"")
    print("\tBEGIN")
    print("#ifdef UNICODE")
    print("\t\tVALUE \"Translation\", 0x409, 1200")
    print("#else")
    print("\t\tVALUE \"Translation\", 0x409, 1252")
    print("#endif")
    print("\tEND")
    print("END")
elif outfmt == 'plist':
    print('<?xml version="1.0" encoding="UTF-8"?>')
    print('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">')
    print('<plist version="1.0">')
    print('<dict>')
    print('\t<key>CFBundleDisplayName</key>')
    print('\t<string>%s</string>' % product_name)
    print('\t<key>CFBundleIdentifier</key>')
    print('\t<string>%s</string>' % bundle_identifier)
    print('\t<key>CFBundleInfoDictionaryVersion</key>')
    print('\t<string>6.0</string>')
    print('\t<key>CFBundleName</key>')
    print('\t<string>%s</string>' % product_name)
    print('\t<key>CFBundleShortVersionString</key>')
    print('\t<string>%s.%s.%s</string>' % (version_major, version_minor, version_build))
    print('</dict>')
    print('</plist>')
