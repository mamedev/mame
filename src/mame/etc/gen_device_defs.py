# license: BSD-3-Clause
"""Simple Python script to generate a new definition from the template_* files
"""
import argparse
import os

def get_args():
	parser = argparse.ArgumentParser(description="""
		Create a new device .cpp/.h definition, using template_device.* as a base.
		All arguments are sanitized to match the given patterns
		"""
	)

	parser.add_argument(
		"-license_opt",
		default="BSD-3-Clause",
		type=str,
		help="License option"
	)
	parser.add_argument(
		"-device_longname",
		required=True,
		type=str,
		help="Friendly name for a device \"Acorn VIDC20\""
	)
	parser.add_argument(
		"-device_classname",
		required=True,
		type=str,
		help="Class declaration \"acorn_vidc20\", this will also be the filename output"
	)
	parser.add_argument(
		"-device_typename",
		required=True,
		type=str,
		help='Type handle declaration \"ACORN_VIDC20\"'
	)
	parser.add_argument(
		"-author_name",
		required=True,
		type=str,
		help="Your author handle name in copyright header"
	)
	return vars(parser.parse_args())

if __name__ == '__main__':
	args = get_args()
	for file_type in [".cpp", ".h"]:
		with open(".{0}template_device{1}".format(os.path.sep, file_type), "r", encoding="utf-8", newline="\n") as template_fh:
			buf_string = template_fh.read()
		for k, v in args.items():
			buf_string = buf_string.replace("<" + k + ">", v)
		with open(".{0}{2}{1}".format(os.path.sep, file_type, args["device_classname"]), "w", encoding="utf-8", newline="\n") as result_fh:
			result_fh.write(buf_string)
