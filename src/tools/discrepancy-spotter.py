#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Zoe Blade

# Find discrepancies in arcade ROM dump names, by Zoe Blade
# For Python 2 and 3

import sys
import xml.etree.ElementTree


def checkPair(parentMachine, childMachine):
    for childRom in childMachine.iter('rom'):
        for parentRom in parentMachine.iter('rom'):
            if parentRom.get('sha1') == childRom.get('sha1'):
                # ROM pair found
                if parentRom.get('name') != childRom.get('name'):
                    # The names don't match
                    sys.stdout.write('%s %s: %s -> %s\n' % (childMachine.get('sourcefile'), childMachine.get('name'), childRom.get('name'), parentRom.get('name')))
                else:
                    break


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

    for childMachine in root.iter('machine'):
        if childMachine.get('cloneof'):
            for parentMachine in root.iter('machine'):
                if parentMachine.get('name') == childMachine.get('cloneof'):
                    # Machine pair found
                    checkPair(parentMachine, childMachine)

    sys.exit(0)
