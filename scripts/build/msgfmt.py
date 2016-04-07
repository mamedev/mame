#! /usr/bin/env python
# -*- coding: utf-8 -*-
# Written by Martin v. Löwis <loewis@informatik.hu-berlin.de>

"""Generate binary message catalog from textual translation description.

This program converts a textual Uniforum-style message catalog (.po file) into
a binary GNU catalog (.mo file).  This is essentially the same function as the
GNU msgfmt program, however, it is a simpler implementation.

Usage: msgfmt.py [OPTIONS] filename.po

Options:
    -o file
    --output-file=file
        Specify the output file to write to.  If omitted, output will go to a
        file named filename.mo (based off the input file name).

    -h
    --help
        Print this message and exit.

    -V
    --version
        Display version information and exit.
"""

from __future__ import print_function
import os
import sys
import getopt
import struct
import array
import re
import codecs
from email.parser import HeaderParser

__version__ = "1.2"

MESSAGES = {}



def usage(code, msg=''):
    print(__doc__, file=sys.stderr)
    if msg:
        print(msg, file=sys.stderr)
    sys.exit(code)



def add(id, str, fuzzy):
    "Add a non-fuzzy translation to the dictionary."
    global MESSAGES
    if not fuzzy and str:
        MESSAGES[id] = str

def dequote(s):
    if (s[0] == s[-1]) and s.startswith(("'", '"')):
        return s[1:-1]
    return s

# decode_escapes from http://stackoverflow.com/a/24519338
ESCAPE_SEQUENCE_RE = re.compile(r'''
    ( \\U........      # 8-digit hex escapes
    | \\u....          # 4-digit hex escapes
    | \\x..            # 2-digit hex escapes
    | \\[0-7]{1,3}     # Octal escapes
    | \\N\{[^}]+\}     # Unicode characters by name
    | \\[\\'"abfnrtv]  # Single-character escapes
    )''', re.UNICODE | re.VERBOSE)

def decode_escapes(s):
    def decode_match(match):
        return codecs.decode(match.group(0), 'unicode-escape')

    return ESCAPE_SEQUENCE_RE.sub(decode_match, s)


def generate():
    "Return the generated output."
    global MESSAGES
    # the keys are sorted in the .mo file
    keys = sorted(MESSAGES.keys())
    offsets = []
    ids = strs = b''
    for id in keys:
        # For each string, we need size and file offset.  Each string is NUL
        # terminated; the NUL does not count into the size.
        offsets.append((len(ids), len(id), len(strs), len(MESSAGES[id])))
        ids += id + b'\0'
        strs += MESSAGES[id] + b'\0'
    output = ''
    # The header is 7 32-bit unsigned integers.  We don't use hash tables, so
    # the keys start right after the index tables.
    # translated string.
    keystart = 7*4+16*len(keys)
    # and the values start after the keys
    valuestart = keystart + len(ids)
    koffsets = []
    voffsets = []
    # The string table first has the list of keys, then the list of values.
    # Each entry has first the size of the string, then the file offset.
    for o1, l1, o2, l2 in offsets:
        koffsets += [l1, o1+keystart]
        voffsets += [l2, o2+valuestart]
    offsets = koffsets + voffsets
    output = struct.pack("Iiiiiii",
                         0x950412de,       # Magic
                         0,                 # Version
                         len(keys),         # # of entries
                         7*4,               # start of key index
                         7*4+len(keys)*8,   # start of value index
                         0, 0)              # size and offset of hash table
    output += array.array("i", offsets).tostring()
    output += ids
    output += strs
    return output



def make(filename, outfile):
    ID = 1
    STR = 2

    # Compute .mo name from .po name and arguments
    if filename.endswith('.po'):
        infile = filename
    else:
        infile = filename + '.po'
    if outfile is None:
        outfile = os.path.splitext(infile)[0] + '.mo'

    try:
        lines = open(infile, 'rb').readlines()
    except IOError as msg:
        print(msg, file=sys.stderr)
        sys.exit(1)

    section = None
    fuzzy = 0
    empty = 0
    header_attempted = False

    # Start off assuming Latin-1, so everything decodes without failure,
    # until we know the exact encoding
    encoding = 'latin-1'

    # Start off assuming Latin-1, so everything decodes without failure,
    # until we know the exact encoding
    encoding = 'latin-1'

    # Parse the catalog
    for lno, l in enumerate(lines):
        l = l.decode(encoding)
        # If we get a comment line after a msgstr, this is a new entry
        if l[0] == '#' and section == STR:
            add(msgid, msgstr, fuzzy)
            section = None
            fuzzy = 0
        # Record a fuzzy mark
        if l[:2] == '#,' and 'fuzzy' in l:
            fuzzy = 1
        # Skip comments
        if l[0] == '#':
            continue
        # Now we are in a msgid section, output previous section
        if l.startswith('msgid') and not l.startswith('msgid_plural'):
            if section == STR:
                add(msgid, msgstr, fuzzy)
                if not msgid:
                    # See whether there is an encoding declaration
                    p = HeaderParser()
                    charset = p.parsestr(msgstr.decode(encoding)).get_content_charset()
                    if charset:
                        encoding = charset
            section = ID
            l = l[5:]
            msgid = msgstr = b''
            is_plural = False
            if l.strip() == '""':
                # Check if next line is msgstr. If so, this is a multiline msgid.
                if lines[lno+1].decode(encoding).startswith('msgstr'):
                    # If this is the first empty msgid and is followed by msgstr, this is the header, which may contain the encoding declaration.
                    # Otherwise this file is not valid
                    if empty > 1:
                        print("Found multiple empty msgids on line " + str(lno) + ", not valid!")
                    empty += 1
        # This is a message with plural forms
        elif l.startswith('msgid_plural'):
            if section != ID:
                print('msgid_plural not preceded by msgid on %s:%d' % (infile, lno),
                      file=sys.stderr)
                sys.exit(1)
            l = l[12:]
            msgid += b'\0' # separator of singular and plural
            is_plural = True
        # Now we are in a msgstr section
        elif l.startswith('msgstr'):
            section = STR
            if l.startswith('msgstr['):
                if not is_plural:
                    print('plural without msgid_plural on %s:%d' % (infile, lno),
                          file=sys.stderr)
                    sys.exit(1)
                l = l.split(']', 1)[1]
                if msgstr:
                    msgstr += b'\0' # Separator of the various plural forms
            else:
                if (l[6:].strip() == '""') and (empty == 1) and (not header_attempted):
                    header = ""
                    # parse up until next empty line = end of header
                    hdrno = lno
                    while(hdrno < len(lines)-1):
                        # This is a roundabout way to strip non-ASCII unicode characters from the header.
                        # As we are only parsing out the encoding, we don't need any unicode chars in it.
                        l = lines[hdrno+1].decode('unicode_escape').encode('ascii','ignore').decode(encoding)
                        if l.strip():
                            header += decode_escapes(dequote(l.strip()))
                        else:
                            break
                        hdrno += 1
                    # See whether there is an encoding declaration
                    if(hdrno > lno):
                        p = HeaderParser()
                        charset = p.parsestr(str(header)).get_content_charset()
                        header_attempted = True
                        if charset:
                            encoding = charset
                if is_plural:
                    print('indexed msgstr required for plural on  %s:%d' % (infile, lno),
                          file=sys.stderr)
                    sys.exit(1)
                l = l[6:]
        # Skip empty lines
        l = l.strip()
        if not l:
            continue
        l = decode_escapes(dequote(l)) # strip quotes and replace newlines if present
        if section == ID:
            msgid += l.encode(encoding)
        elif section == STR:
            msgstr += l.encode(encoding)
        else:
            print('Syntax error on %s:%d' % (infile, lno), \
                  'before:', file=sys.stderr)
            print(l, file=sys.stderr)
            sys.exit(1)
    # Add last entry
    if section == STR:
        add(msgid, msgstr, fuzzy)

    # Compute output
    output = generate()

    try:
        open(outfile,"wb").write(output)
    except IOError as msg:
        print(msg, file=sys.stderr)



def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hVo:',
                                   ['help', 'version', 'output-file='])
    except getopt.error as msg:
        usage(1, msg)

    outfile = None
    # parse options
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage(0)
        elif opt in ('-V', '--version'):
            print("msgfmt.py", __version__)
            sys.exit(0)
        elif opt in ('-o', '--output-file'):
            outfile = arg
    # do it
    if not args:
        print('No input file given', file=sys.stderr)
        print("Try `msgfmt --help' for more information.", file=sys.stderr)
        return

    for filename in args:
        make(filename, outfile)


if __name__ == '__main__':
    main()
