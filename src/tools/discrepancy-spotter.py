#!/usr/bin/python

# Find discrepancies in arcade ROM dump names, by Zoe Blade
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

				print(childMachine.get('sourcefile') + ' ' + childMachine.get('name') + ': ' + childRom.get('name') + ' -> ' + parentRom.get('name'))
