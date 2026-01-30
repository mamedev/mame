#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import argparse
import io
import re
import string
import sys


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', '-t', metavar='<target>', default='mame', help='target name')
    parser.add_argument('--subtarget', '-s', metavar='<subtarget>', default='mame', help='subtarget name')
    parser.add_argument('--executable', '-e', metavar='<executable>', default='mame', help='base executable name')
    parser.add_argument('--format', '-f', choices=('rc', 'plist'), metavar='<format>', default='rc', help='output format')
    parser.add_argument('--resources', '-r', metavar='<resfile>', help='resource file to include')
    parser.add_argument('-o', metavar='<outfile>', help='output file name')
    parser.add_argument('input', metavar='<srcfile>', help='version info source file')
    return parser.parse_args()


def extract_version(verinfo):
    pattern = re.compile(r'\s+BARE_BUILD_VERSION\s+"(([^."]+)\.([^."]+))"')
    for line in verinfo:
        match = pattern.search(line)
        if match:
            return match.group(1), match.group(2), match.group(3)
    return None, None, None


if __name__ == '__main__':
    options = parse_args()

    try:
        with io.open(options.input, 'r') as verinfo:
            verfull, vermajor, verminor = extract_version(verinfo)
            verbuild = '0'
    except IOError as e:
        sys.stderr.write("Error reading source file '%s': %s\n" % (options.input, e))
        sys.exit(1)

    if verfull is None:
        sys.stderr.write("Unable to extract version from source file '%s'\n" % (options.input, ))
        sys.exit(1)

    if options.format == 'plist':
        template = string.Template(
                '<?xml version="1.0" encoding="UTF-8"?>\n' \
                '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n' \
                '<plist version="1.0">\n' \
                '<dict>\n' \
                '\t<key>CFBundleDisplayName</key>\n' \
                '\t<string>${product}</string>\n' \
                '\t<key>CFBundleIdentifier</key>\n' \
                '\t<string>${rdns}</string>\n' \
                '\t<key>CFBundleInfoDictionaryVersion</key>\n' \
                '\t<string>6.0</string>\n' \
                '\t<key>CFBundleName</key>\n' \
                '\t<string>${product}</string>\n' \
                '\t<key>CFBundleShortVersionString</key>\n' \
                '\t<string>${major}.${minor}.${build}</string>\n' \
                '\t<key>NSPrincipalClass</key>\n' \
                '\t<string>NSApplication</string>\n' \
                '</dict>\n' \
                '</plist>\n')
    else:
        template = string.Template(
                '#include <windows.h>\n' \
                '#pragma code_page(65001)\n' \
                'VS_VERSION_INFO VERSIONINFO\n' \
                '\tFILEVERSION ${major},${minor},${build},${subbuild}\n' \
                '\tPRODUCTVERSION ${major},${minor},${build},${subbuild}\n' \
                '\tFILEFLAGSMASK 0x3fL\n' \
                '\tFILEFLAGS ${winfileflags}\n' \
                '\tFILEOS VOS_NT_WINDOWS32\n' \
                '\tFILETYPE VFT_APP\n' \
                '\tFILESUBTYPE VFT2_UNKNOWN\n' \
                'BEGIN\n' \
                '\tBLOCK "StringFileInfo"\n' \
                '\tBEGIN\n' \
                '#ifdef UNICODE\n' \
                '\t\tBLOCK "040904b0"\n' \
                '#else\n' \
                '\t\tBLOCK "040904E4"\n' \
                '#endif\n' \
                '\t\tBEGIN\n' \
                '\t\t\tVALUE "Author", "${author}\\0"\n' \
                '\t\t\tVALUE "Comments", "${comments}\\0"\n' \
                '\t\t\tVALUE "CompanyName", "${company}\\0"\n' \
                '\t\t\tVALUE "FileDescription", "${filedesc}\\0"\n' \
                '\t\t\tVALUE "FileVersion", "${major}, ${minor}, ${build}, ${subbuild}\\0"\n' \
                '\t\t\tVALUE "InternalName", "${internal}\\0"\n' \
                '\t\t\tVALUE "LegalCopyright", "${copyright}\\0"\n' \
                '\t\t\tVALUE "OriginalFilename", "${original}\\0"\n' \
                '\t\t\tVALUE "ProductName", "${product}\\0"\n' \
                '\t\t\tVALUE "ProductVersion", "${version}\\0"\n' \
                '\t\tEND\n' \
                '\tEND\n' \
                '\tBLOCK "VarFileInfo"\n' \
                '\tBEGIN\n' \
                '#ifdef UNICODE\n' \
                '\t\tVALUE "Translation", 0x409, 1200\n' \
                '#else\n' \
                '\t\tVALUE "Translation", 0x409, 1252\n' \
                '#endif\n' \
                '\tEND\n' \
                'END\n' \
                '#include "${resources}"\n')

    internal = options.target + '_' + options.subtarget if options.target != options.subtarget else options.target
    text = template.substitute(
            version=verfull,
            major=vermajor, minor=verminor, build='0', subbuild='0',
            author='MAMEdev and contributors',
            comments='Multi-purpose emulation framework',
            company='MAMEdev',
            filedesc='MAME',
            internal=internal,
            original=options.executable,
            product=('MAME' if options.target == 'mame' else options.target),
            rdns=('org.mamedev.' + internal),
            copyright='\u00a9 1997-2026 MAMEdev and contributors',
            winfileflags=('0x0L' if verbuild == '0' else 'VS_FF_PRERELEASE'),
            resources=(options.resources or 'mame.rc'))

    if options.o is not None:
        try:
            with io.open(options.o, 'w', encoding='utf-8') as out:
                out.write(text)
        except IOError as e:
            sys.stderr.write("Error writing output file '%s': %s\n" % (options.o, e))
            sys.exit(1)
    else:
        sys.stdout.write(text)
