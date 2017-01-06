#!/usr/bin/python

# Fix discrepancies in arcade ROM dump names, by Zoe Blade
# For Python 2

import xml.etree.ElementTree

print('Loading XML file...')
root = xml.etree.ElementTree.parse('arcade.xml').getroot()
print('Done.')

for childMachine in root.iter('machine'):
	if not childMachine.get('cloneof'):
		continue

	for parentMachine in root.iter('machine'):
		if not parentMachine.get('name') == childMachine.get('cloneof'):
			continue

		# Machine pair found

		for childRom in childMachine.iter('rom'):
			for parentRom in parentMachine.iter('rom'):
				if not parentRom.get('sha1') == childRom.get('sha1'):
					continue

				# ROM pair found

				if parentRom.get('name') == childRom.get('name'):
					break

				# The names don't match

				sourceFilename = childMachine.get('sourcefile')

				input = open(sourceFilename, 'r')
				source = input.read()
				input.close()

				oldRomFilename = '"' + childRom.get('name') + '"'
				newRomFilename = '"' + parentRom.get('name') + '"'

				oldRomFilenamePadded = oldRomFilename.ljust(14, ' ')
				newRomFilenamePadded = newRomFilename.ljust(14, ' ')

				source = source.replace(oldRomFilenamePadded, newRomFilenamePadded) # Try to preserve fancy spacing where possible
				source = source.replace(oldRomFilename, newRomFilename) # Fallback on just replacing the filename

				output = open(sourceFilename, 'w')
				output.write(source)
				output.close()

				print(sourceFilename + ': ' + oldRomFilename + ' -> ' + newRomFilename)
