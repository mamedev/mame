#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert

import sys
import re

# Initial state
state = 1
text = ""
dispatch_to_states = { "MAIN" : 0 }
states_to_dispatch = { 0 : "MAIN" }

def load_file(fname, lines):
	path = fname.rpartition('/')[0]
	if path != "":
		path += '/'
	try:
		f = open(fname, "rU")
	except Exception:
		err = sys.exc_info()[1]
		sys.stderr.write("Cannot read opcodes file %s [%s]\n" % (fname, err))
		sys.exit(1)
	
	rawlines = re.split('(\n|; *\n?)', f.read())
	count = 0
	while count < len(rawlines)-1:
		line = rawlines[count+0] + rawlines[count+1]
		if line.startswith("#include"):
			load_file(path + line.split('"')[1], lines)
		else:
			lines.append(line)
		count += 2
	
	f.close()

# Get lines
lines = []
load_file(sys.argv[1], lines)

count = 0
while count < len(lines):
	# Retrieve this line
	line = lines[count]

	# Retrieve the whitespace
	whitespace = line[:len(line) - len(line.lstrip())]

	# Check to see if the next line is a return
	next_line_is_return = (count + 1 == len(lines)) or lines[count+1].strip() == "return;"

	# Check to see if the next line is a dispatch followed by return
	next_line_is_dispatch_and_return = (count + 1 < len(lines)) and re.match('([A-Za-z0-9\t ]+\:)*\s*\%', lines[count+1]) and lines[count+2].strip() == "return;"

	if re.match('([A-Za-z0-9\t ]+\:)*\s*\%', line):
		# This is a dispatch - find the '%'
		percent_pos = line.find("%")
		dispatch = line[percent_pos+1:].strip("\t\n; ")

		# Do we have a label?
		label = line[:percent_pos].strip()
		if label != "":
			text += whitespace + label + "\n"
			whitespace += "\t"
		
		# Create the goto command
		if dispatch[-1:] == "*":
			goto_command = "if (is_register_register_op_16_bit()) goto %s16; else goto %s8;\n" %(dispatch[:-1], dispatch[:-1])
		else:
			goto_command = "goto %s;\n" % dispatch

		# Are we right before a 'return'?
		if next_line_is_return:
			text += whitespace + goto_command
			count += 1  # Skip the return
		elif next_line_is_dispatch_and_return:
			# We are followed by a dispatch/return combo; identify the next dispatch
			percent_pos = lines[count+1].find("%")
			next_dispatch = lines[count+1][percent_pos+1:].strip("\t\n; ")

			# If there is no state number associated with the next dispatch, make one
			if next_dispatch not in dispatch_to_states:
				dispatch_to_states[next_dispatch] = state
				states_to_dispatch[state] = next_dispatch
				state += 1

			text += whitespace + "push_state(%s);\t// %s\n" % (dispatch_to_states[next_dispatch], next_dispatch)
			text += whitespace + goto_command
			count += 2  # Skip the dispatch/return

		else:
			# Normal dispatch
			text += whitespace + "push_state(%s);\n" % state
			text += whitespace + goto_command
			text += "state_%s:\n" % state
			state += 1
	else:
		# "Normal" code
		# Is there an '@' here?
		check_icount = line.lstrip().startswith("@")
		if check_icount:
			line = line.replace("@", "", 1)

		# Output the line
		text += line

		# If we have to decrement the icount, output more info
		if check_icount and not next_line_is_return:
			text += whitespace + "if (UNEXPECTED(m_icount <= 0)) { push_state(%s); return; }\n" % state
			text += "state_%s:\n" % state
			state += 1

	# Advance to next line
	count += 1

# Output the case labels
for i in range(0, state):
	print("\tcase %d:	goto %s;" % (i, states_to_dispatch.get(i, "state_%d" % i)))

# Output a default case
print("\tdefault:")
print("\t\tfatalerror(\"Unexpected state\");")
print("\t\tbreak;")
print("")

# Finally output the text
print(text)
