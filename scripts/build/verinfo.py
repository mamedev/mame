#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner

import io
import re
import sys


def parse_args():
    def usage():
        sys.stderr.write('Usage: verinfo.py [-b mame|mess|ume] [-r|-p] [-o <outfile>] <srcfile>\n')
        sys.exit(1)

    flags = True
    target = 'mame'
    format = 'rc'
    input = None
    output = None
    i = 1
    while i < len(sys.argv):
        if flags and (sys.argv[i] == '-r'):
            format = 'rc'
        elif flags and (sys.argv[i] == '-p'):
            format = 'plist'
        elif flags and (sys.argv[i] == '-b'):
            i += 1
            if i >= len(sys.argv):
                usage()
            else:
                target = sys.argv[i]
        elif flags and (sys.argv[i] == '-o'):
            i += 1
            if (i >= len(sys.argv)) or (output is not None):
                usage()
            else:
                output = sys.argv[i]
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
    return target, format, input, output


def extract_version(input):
    pattern = re.compile('\s+BARE_BUILD_VERSION\s+"(([^."]+)\.([^."]+))"')
    for line in input.readlines():
        match = pattern.search(line)
        if match:
            return match.group(1), match.group(2), match.group(3)
    return None, None, None


build, outfmt, srcfile, dstfile = parse_args()

try:
    fp = io.open(srcfile, 'r')
except IOError:
    sys.stderr.write("Unable to open source file '%s'\n" % srcfile)
    sys.exit(1)

version_string, version_major, version_minor = extract_version(fp)
version_build = "0"
version_subbuild = "0"
if not version_string:
    sys.stderr.write("Unable to extract version from source file '%s'\n" % srcfile)
    sys.exit(1)
fp.close()

if dstfile is not None:
    try:
        fp = open(dstfile, 'w')
    except IOError:
        sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
        sys.exit(1)
else:
    fp = sys.stdout

if build == "mess":
    # MESS
    author = "MESS Team"
    comments = "Multi Emulation Super System"
    company_name = "MESS Team"
    file_description = "MESS"
    internal_name = "MESS"
    original_filename = "MESS"
    product_name = "MESS"
    bundle_identifier = "org.mamedev.mess"
else:
    # MAME
    author = "Nicola Salmoria and the MAME Team"
    comments = "Multi-purpose emulation framework"
    company_name = "MAME Team"
    file_description = "MAME"
    internal_name = "MAME" if build == "mame" else build
    original_filename = "MAME" if build == "mame" else build
    product_name = "MAME" if build == "mame" else build
    bundle_identifier = "org.mamedev." + build

legal_copyright = "Copyright Nicola Salmoria and the MAME team"

if outfmt == 'rc':
    fp.write('VS_VERSION_INFO VERSIONINFO\n')
    fp.write('\tFILEVERSION %s,%s,%s,%s\n' % (version_major, version_minor, version_build, version_subbuild))
    fp.write('\tPRODUCTVERSION %s,%s,%s,%s\n' % (version_major, version_minor, version_build, version_subbuild))
    fp.write('\tFILEFLAGSMASK 0x3fL\n')
    if version_build == 0:
        fp.write('\tFILEFLAGS 0x0L\n')
    else:
        fp.write('\tFILEFLAGS VS_FF_PRERELEASE\n')
    fp.write('\tFILEOS VOS_NT_WINDOWS32\n')
    fp.write('\tFILETYPE VFT_APP\n')
    fp.write('\tFILESUBTYPE VFT2_UNKNOWN\n')
    fp.write('BEGIN\n')
    fp.write('\tBLOCK "StringFileInfo"\n')
    fp.write('\tBEGIN\n')
    fp.write('#ifdef UNICODE\n')
    fp.write('\t\tBLOCK "040904b0"\n')
    fp.write('#else\n')
    fp.write('\t\tBLOCK "040904E4"\n')
    fp.write('#endif\n')
    fp.write('\t\tBEGIN\n')
    fp.write('\t\t\tVALUE "Author", "%s\\0"\n' % author)
    fp.write('\t\t\tVALUE "Comments", "%s\\0"\n' % comments)
    fp.write('\t\t\tVALUE "CompanyName", "%s\\0"\n' % company_name)
    fp.write('\t\t\tVALUE "FileDescription", "%s\\0"\n' % file_description)
    fp.write('\t\t\tVALUE "FileVersion", "%s, %s, %s, %s\\0"\n' % (version_major, version_minor, version_build, version_subbuild))
    fp.write('\t\t\tVALUE "InternalName", "%s\\0"\n' % internal_name)
    fp.write('\t\t\tVALUE "LegalCopyright", "%s\\0"\n' % legal_copyright)
    fp.write('\t\t\tVALUE "OriginalFilename", "%s\\0"\n' % original_filename)
    fp.write('\t\t\tVALUE "ProductName", "%s\\0"\n' % product_name)
    fp.write('\t\t\tVALUE "ProductVersion", "%s\\0"\n' % version_string)
    fp.write('\t\tEND\n')
    fp.write('\tEND\n')
    fp.write('\tBLOCK "VarFileInfo"\n')
    fp.write('\tBEGIN\n')
    fp.write('#ifdef UNICODE\n')
    fp.write('\t\tVALUE "Translation", 0x409, 1200\n')
    fp.write('#else\n')
    fp.write('\t\tVALUE "Translation", 0x409, 1252\n')
    fp.write('#endif\n')
    fp.write('\tEND\n')
    fp.write('END\n')
elif outfmt == 'plist':
    fp.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    fp.write('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
    fp.write('<plist version="1.0">\n')
    fp.write('<dict>\n')
    fp.write('\t<key>CFBundleDisplayName</key>\n')
    fp.write('\t<string>%s</string>\n' % product_name)
    fp.write('\t<key>CFBundleIdentifier</key>\n')
    fp.write('\t<string>%s</string>\n' % bundle_identifier)
    fp.write('\t<key>CFBundleInfoDictionaryVersion</key>\n')
    fp.write('\t<string>6.0</string>\n')
    fp.write('\t<key>CFBundleName</key>\n')
    fp.write('\t<string>%s</string>\n' % product_name)
    fp.write('\t<key>CFBundleShortVersionString</key>\n')
    fp.write('\t<string>%s.%s.%s</string>\n' % (version_major, version_minor, version_build))
    fp.write('\t<key>NSPrincipalClass</key>\n')
    fp.write('\t<string>NSApplication</string>\n')
    fp.write('</dict>\n')
    fp.write('</plist>\n')
fp.flush()
