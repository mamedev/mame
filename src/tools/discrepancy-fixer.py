#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Zoe Blade

# Fix discrepancies in arcade ROM dump names, by Zoe Blade
# For Python 2 and 3

import sys
import xml.etree.ElementTree


def fixPair(parentMachine, childMachine):
    changes = { }
    for childRom in childMachine.iter('rom'):
        for parentRom in parentMachine.iter('rom'):
            if parentRom.get('sha1') == childRom.get('sha1'):
                # ROM pair found
                if parentRom.get('name') != childRom.get('name'):
                    # The names don't match
                    changes[childRom.get('name')] = parentRom.get('name')

    if changes:
        sourceFilename = childMachine.get('sourcefile')

        try:
            input = open(sourceFilename, 'r')
            source = input.read()
            input.close()
        except Exception as e:
            sys.stderr.write('%s: error reading %s: %s\n' % (sys.argv[0], sourceFilename, e))
            return False

        for oldRomFilename in changes:
            newRomFilename = '"%s"' % (changes[oldRomFilename])
            oldRomFilename = '"%s"' % (oldRomFilename)

            paddedLen = max(len(oldRomFilename), len(newRomFilename))
            oldRomFilenamePadded = oldRomFilename.ljust(paddedLen, ' ')
            newRomFilenamePadded = newRomFilename.ljust(paddedLen, ' ')

            source = source.replace(oldRomFilenamePadded, newRomFilenamePadded) # Try to preserve fancy spacing where possible
            source = source.replace(oldRomFilename, newRomFilename) # Fallback on just replacing the filename

            sys.stdout.write('%s: %s -> %s\n' % (sourceFilename, oldRomFilename, newRomFilename))

        output = open(sourceFilename, 'w')
        output.write(source)
        output.close()

    return True


if __name__ == '__main__':
    if len(sys.argv) > 2:
        sys.stderr.write('Usage:\n%s [arcade.xml]\n' % sys.argv[0])
        sys.exit(1)

    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        filename = 'arcade.xml'

    sys.stderr.write('Loading XML file...')
    sys.stderr.flush()
    try:
        root = xml.etree.ElementTree.parse(filename).getroot()
    except Exception as e:
        sys.stderr.write('\n%s: error parsing %s: %s\n' % (sys.argv[0], filename, e))
        sys.exit(2)
    sys.stderr.write('done.\n')

    errors = 0
    for childMachine in root.iter('machine'):
        if childMachine.get('cloneof'):
            for parentMachine in root.iter('machine'):
                if parentMachine.get('name') == childMachine.get('cloneof'):
                    # Machine pair found
                    if not fixPair(parentMachine, childMachine):
                        errors += 1

    sys.exit(0 if errors == 0 else 3)
